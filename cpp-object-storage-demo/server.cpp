#include <netinet/in.h>
#include <iostream>
#include <fcntl.h>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include "server.h"
#include "object.h"
#include "file.h"

static int setNonBlocking(int sockfd) {
    if (fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD, 0)|O_NONBLOCK) == -1) {
        return -1;
    }
    return 0;
}

int Server::start() {
    if (createDirIfNotExists() < 0) {
        // creating data path provided by the program
        return -1;
    }

    struct sockaddr_in servaddr{};
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    inet_pton(AF_INET, this->conf.addr.c_str(), &(servaddr.sin_addr));
    servaddr.sin_port = htons(this->conf.port);

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) {
        perror("can't create socket fd");
        return -1;
    }
    int opt = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (setNonBlocking(listenfd) < 0) {
        perror("setnonblock error");
    }

    if (bind(listenfd, (struct sockaddr *) &servaddr, sizeof(struct sockaddr)) == -1) {
        perror("bind error");
        return -1;
    }

    if (listen(listenfd, MAX_BACKLOG) == -1) {
        perror("listen error");
        return -1;
    }

    epollfd = epoll_create(MAX_EPOLL_SIZE);
    struct epoll_event ev{};
    ev.events = EPOLLIN;
    ev.data.fd = listenfd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &ev) < 0) {
        // monitor ctl_add
        fprintf(stderr, "epoll set insertion error: fd=%d\n", listenfd);
        return -1;
    }

    int threadCount = pool->threadCount();
    printf("epoll server startup, %s:%d, max connection is %d, backlog is %d\n",
           this->conf.addr.c_str(), this->conf.port, MAX_EPOLL_SIZE * threadCount, MAX_BACKLOG);

    std::vector<std::future<void>> rets;
    rets.reserve(threadCount);
    for(int i = 0; i < threadCount; i++) {
        rets.emplace_back(pool->submit([this] { loop(); }));
    }
    for(auto& f : rets) {
        f.get();
    }
    return 0;
}

void Server::loop() {
    int curfds = 1;
    struct epoll_event events[MAX_EPOLL_SIZE];
    struct sockaddr_in cliaddr{};
    socklen_t socklen = sizeof(struct sockaddr_in);
    for(;;) {
        int nfds = epoll_wait(epollfd, events, MAX_EPOLL_SIZE, -1);

        if(nfds == -1) {
            perror("epoll_wait error");
            break;
        }

        for(int i = 0; i < nfds; i++) {
            if (events[i].events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
                // handle connection errors, e.g. connection closed by client, it will also trigger EPOLLIN
                struct epoll_event ev{};
                ev.events = EPOLLIN | EPOLLOUT | EPOLLONESHOT;
                auto *connection = (Connection *) events[i].data.ptr;
                int connfd = connection->connfd;
                close(connection->connfd);
                delete connection;
                curfds--;
                epoll_ctl(epollfd, EPOLL_CTL_DEL, connfd, &ev);
            } else if (events[i].data.fd == listenfd) {
                // handle new connection
                int connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &socklen);
                if (connfd > 0) {
                    if (curfds >= MAX_EPOLL_SIZE) {
                        fprintf(stderr, "too many connection, more than %d\n", MAX_EPOLL_SIZE);
                        close(connfd);
                        continue;
                    }
                    setNonBlocking(connfd);
                    struct epoll_event ev{};
                    ev.events = EPOLLIN | EPOLLET | EPOLLONESHOT; // using one shot in concurrent env

                    auto *connection = new Connection();
                    connection->connfd = connfd;

                    ev.data.ptr = connection; // set connection instance to record the r/w state

                    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, connfd, &ev) == -1) {
                        perror("epoll_ctl: add");
                        close(connfd);
                    }
                    printf("connection established from %s:%d.\n", inet_ntoa(cliaddr.sin_addr), cliaddr.sin_port);
                    curfds++;
                }
                if (connfd == -1) {
                    if (errno != EAGAIN && errno != ECONNABORTED
                        && errno != EPROTO && errno != EINTR) {
                        perror("accept failed");
                    }
                }
            } else if (events[i].events & EPOLLIN) {
                auto *conn = (Connection *) events[i].data.ptr;
                int connfd = conn->connfd;
                struct epoll_event ev;
                ev.events = EPOLLIN | EPOLLOUT | EPOLLONESHOT;
                ev.data.ptr = conn;

                // 1. Read data
                if (handleRead(conn) < 0) {
                    // error, remove fd from epoll
                    epoll_ctl(epollfd, EPOLL_CTL_DEL, connfd, &ev);
                    close(connfd);
                    delete conn;
                    curfds--;
                    continue;
                }
                if (conn->state < STATE_READ_DONE) {
                    // 2.1. not finish read, reset read events for the next round
                    ev.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
                    epoll_ctl(epollfd, EPOLL_CTL_MOD, connfd, &ev);
                } else if (conn->state == STATE_READ_DONE) {
                    // 2.2 finish read, parse & handle request, generate resp data, and set write events
                    if (handle(conn) < 0) {
                        auto *connection = (Connection *) ev.data.ptr;
                        delete connection;
                        curfds--;
                        ev.events = EPOLLIN | EPOLLOUT | EPOLLONESHOT;
                        epoll_ctl(epollfd, EPOLL_CTL_DEL, connfd, &ev);
                    } else {
                        // set write mode
                        ev.events = EPOLLOUT | EPOLLET | EPOLLONESHOT;
                        epoll_ctl(epollfd, EPOLL_CTL_MOD, connfd, &ev);
                    }
                }
            } else if (events[i].events & EPOLLOUT) {
                auto *conn = (Connection *) events[i].data.ptr;
                int connfd = conn->connfd;
                struct epoll_event ev{};
                ev.data.ptr = conn;
                // 1. Write data
                if (handleWrite(conn) < 0) {
                    // error, remove fd from epoll
                    close(connfd);
                    delete conn;
                    curfds--;
                    ev.events = EPOLLIN | EPOLLOUT | EPOLLONESHOT;
                    epoll_ctl(epollfd, EPOLL_CTL_DEL, connfd, &ev);
                    continue;
                }
                if (conn->state < STATE_WRITE_DONE) {
                    // 2.1. not finish write, reset write events for the next round
                    ev.events = EPOLLOUT | EPOLLET | EPOLLONESHOT;
                    epoll_ctl(epollfd, EPOLL_CTL_MOD, connfd, &ev);
                } else if (conn->state == STATE_WRITE_DONE) {
                    // 2.2. finish write, reset connection state
                    free(conn->outBuf);
                    conn->outBuf = nullptr;
                    conn->outLen = 0;
                    conn->outPtr = 0;
                    conn->state = STATE_INIT;
                    // 2.3. set read event for next requests
                    ev.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
                    epoll_ctl(epollfd, EPOLL_CTL_MOD, connfd, &ev);
                }
            }
        }
    }
}

int Server::handleRead(Connection *connection) {
    int fd = connection->connfd;
    int state = connection->state;
    if (state == STATE_INIT) {
        // read length
        int size = (int) read(fd, (char *) &(connection->len), sizeof(int));
        if (size < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // would try again
                return 0;
            }
            perror("read length failed");
            close(fd);
            return -1;
        }
        if (size == 0) {
            // closed by peers
            perror("connection closed by peers");
            close(fd);
            return -1;
        }
        connection->lenPrt += size;
        if (connection->lenPrt < sizeof(int)) {
            connection->state = STATE_READ_LEN;
            return size;
        }
        if (connection->lenPrt == sizeof(int)) {
            connection->len = ntohl(connection->len);
            connection->inLen = connection->len;
            connection->inBuf = (char *) malloc(sizeof(char) * connection->len);
            connection->state = STATE_READ_DATA;
            return size;
        }
    } else if (state == STATE_READ_LEN) {
        int size = (int) read(fd, ((char *) &connection->len) + connection->lenPrt,
                              sizeof(int) - connection->lenPrt);

        if (size < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return 0;
            }
            perror("read length failed");
            close(fd);
            return -1;
        }
        if (size == 0) {
            // closed by peers
            perror("connection closed by peers");
            close(fd);
            return -1;
        }

        connection->lenPrt += size;
        if (connection->lenPrt < sizeof(int)) {
            return size;
        }
        if (connection->lenPrt == sizeof(int)) {
            connection->len = ntohl(connection->len);
            connection->inLen = connection->len;
            connection->inBuf = (char *) malloc(sizeof(char) * connection->len);
            connection->state = STATE_READ_DATA;
            return size;
        }
    } else if (state == STATE_READ_DATA) {
        int size = (int) read(fd, connection->inBuf + connection->inPtr,
                              connection->inLen - connection->inPtr);
        if (size < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return 0;
            }
            perror("read payload failed");
            close(fd);
            return -1;
        }
        if (size == 0) {
            // closed by peers
            perror("connection closed by peers: ");
            close(fd);
            return -1;
        }
        connection->inPtr += size;
        if (connection->inPtr < connection->inLen) {
            return size;
        }
        if (connection->inPtr == connection->inLen) {
            // need to pop up and handle request
            connection->state = STATE_READ_DONE;
            return size;
        }
    }
    return 0;
}

int Server::handle(Connection *connection) {
    Request request;
    if (request.read(connection->inBuf) < 0) {
        perror("serialize request error");
        return -1;
    }
    // reset in-state
    free(connection->inBuf);
    connection->len = 0;
    connection->lenPrt = 0;
    connection->inBuf = nullptr;
    connection->inPtr = 0;
    connection->inLen = 0;
    // handle it
    Response resp;
    switch (request.getMethod()) {
        case METHOD_PUT:
            handlePut(&request, &resp);
            break;
        case METHOD_GET:
            handleGet(&request, &resp);
            break;
        case METHOD_DELETE:
            handleDelete(&request, &resp);
            break;
        default:
            resp.setStatus(RESP_INVALID);
            resp.setLen(0);
    }
    if (request.getBuf() != nullptr) {
        // free allocated buf
        free(request.getBuf());
    }
    connection->outLen = sizeof(char) * (3 * sizeof(int) + resp.getLen());
    connection->outBuf = (char *) malloc(connection->outLen);
    connection->state = STATE_WRITE_DATA;

    if (resp.write(connection->outBuf) < 0) {
        perror("deserialize response error");
        return -1;
    }
    // resp buf will be released
    return 0;
}

int Server::handleWrite(Connection *connection) {
    int fd = connection->connfd;
    int state = connection->state;
    if (state == STATE_WRITE_DATA) {
        int size = (int) write(fd, connection->outBuf + connection->outPtr,
                               connection->outLen - connection->outPtr);
        if (size < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // if temporary unavailable, try again
                return 0;
            }
            perror("write payload failed");
            close(fd);
            return -1;
        }
        if (size == 0) {
            // closed by peers
            perror("connection closed by peers: ");
            close(fd);
            return -1;
        }
        connection->outPtr += size;
        if (connection->outPtr < connection->outLen) {
            return size;
        }
        if (connection->outPtr == connection->outLen) {
            connection->state = STATE_WRITE_DONE;
            return size;
        }
    }
    return 0;
}

void Server::handlePut(Request *request, Response *response) {
    std::string key = request->getKey();
    std::shared_mutex &m = mutexes[hash(key) % mutexes.size()];
    m.lock(); // write lock
    int len = request->getLen();
    char *buf = request->getBuf();
    response->setLen(0);
    if (buf == nullptr) {
        response->setStatus(RESP_EXCEPTION);
    } else {
        if (meta->get(key) < 0) {
            // new data
            // wb+ will clean the existed data, so previous half write is ok
            auto *target = new FileIO(this->conf.file_path, key, "wb+");
            int w = target->writeData(buf, len);
            if (w < 0 || w != len) {
                perror("error writing file: ");
                response->setStatus(RESP_EXCEPTION);
            } else {
                // ok, update the metadata
                if (meta->put(key) < 0) {
                    perror("error updating meta: ");
                    response->setStatus(RESP_EXCEPTION);
                } else {
                    response->setStatus(RESP_OK);
                }
            }
            delete target;
        } else {
            // existed
            std::string tmpKey = key + ".tmp";
            auto *target = new FileIO(this->conf.file_path, tmpKey, "wb+"); // prev half write will be ok
            std::string old = this->conf.file_path + "/" + key;
            int w = target->writeData(buf, len);
            if (w < 0 || w != len) {
                // write error
                response->setStatus(RESP_EXCEPTION);
            } else {
                // rename is atomic for POSIX
                if (target->renameFile(old) < 0) {
                    // rename failed, just throw an exception
                    // since it's atomic, the old will be kept
                    perror("error rename new temp data file");
                    response->setStatus(RESP_EXCEPTION);
                } else {
                    // rename will remove the old one according to the doc
                    response->setStatus(RESP_OK);
                }
            }
            delete target;
        }
    }

    m.unlock();
}

void Server::handleDelete(Request *request, Response *response) {
    std::string key = request->getKey();
    response->setLen(0);
    std::shared_mutex &m = mutexes[hash(key) % mutexes.size()];
    m.lock(); // write lock

    if (meta->get(key) >= 0) {
        // existed
        auto *io = new FileIO(this->conf.file_path, key, "rb");
        std::string filename = io->getDir();
        if (io->getfp() == nullptr) {
            // if file not found
            // just update the meta
            response->setStatus(RESP_NOT_FOUND);
            // can ignore the result, since it will return the NOT_FOUND on GET when file not exists
            meta->del(key);
            delete io;
        } else {
            delete io;
            if (meta->del(key) < 0) {
                perror("error deleting meta");
                response->setStatus(RESP_EXCEPTION);
            } else {
                remove(filename.data());
                // can ignore the result
                // since wb+ will overwrite it
                response->setStatus(RESP_OK);
            }
        }
    } else {
        response->setStatus(RESP_NOT_FOUND);
        remove((this->conf.file_path + "/" + key).data());
    }
    m.unlock();
}

void Server::handleGet(Request *request, Response *response) {
    std::string key = request->getKey();
    std::shared_mutex &m = mutexes[hash(key) % mutexes.size()];
    m.lock_shared();  // read lock
    if (meta->get(key) >= 0) {
        auto *io = new FileIO(this->conf.file_path, key, "rb");
        int len = io->fileLength();
        if (io->getfp() == nullptr) {
            // file not found
            perror("read file not found: ");
            response->setStatus(RESP_NOT_FOUND);
            response->setLen(0);
        } else if (len < 0) {
            // read length error, io fail
            // just return the exception resp
            perror("read file length error: ");
            response->setStatus(RESP_EXCEPTION);
            response->setLen(0);
        } else {
            int size = len;
            char *buf = (char *) malloc(size);
            int r = io->readData(buf, size);
            if (size == r) {
                response->setStatus(RESP_OK);
                response->setLen(size);
                response->setBuf(buf);
            } else {
                perror("read file error: ");
                response->setStatus(RESP_EXCEPTION);
                response->setLen(0);
                free(buf);
            }
        }
        delete io;
    } else {
        response->setStatus(RESP_NOT_FOUND);
        response->setLen(0);
    }
    m.unlock();
}

int Server::createDirIfNotExists() const {
    namespace fs = std::filesystem;
    try {
        fs::create_directories(this->conf.file_path);
    }
    catch (std::exception &e) { // Not using fs::filesystem_error since std::bad_alloc can throw too.
        std::cout << e.what() << std::endl;
        return -1;
    }
    return 0;
}

