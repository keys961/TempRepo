#ifndef ENTRY_TASK_SERVER_H
#define ENTRY_TASK_SERVER_H

#include <string>
#include <sys/epoll.h>
#include <unistd.h>
#include <array>
#include <shared_mutex>
#include <unordered_map>
#include <filesystem>
#include "threadpool.h"
#include "object.h"
#include "meta.h"

struct Configuration {
    std::string addr;
    int port;
    std::string file_path;
    std::string db_path;
    int threads;
};

/**
 * State: INIT -> READ_LEN--------v                              v-----|
 *         ^  |------------> READ_DATA ----> READ_DONE -----> WRITE_DATA -----> WRITE_DONE
 *         |                  |----^                                                |
 *         ------------------------------------------------------------------------ |
 */
#define STATE_INIT 0
#define STATE_READ_LEN 1
#define STATE_READ_DATA 2
#define STATE_READ_DONE 3
#define STATE_WRITE_DATA 4
#define STATE_WRITE_DONE 5

struct Connection {
    int connfd;

    int state;

    int len;
    int lenPrt;

    char *inBuf;
    int inLen;
    int inPtr;
    char *outBuf;
    int outLen;
    int outPtr;

    Connection() : connfd(0), state(STATE_INIT), len(0), lenPrt(0),
                   inBuf(nullptr), inLen(0), inPtr(0),
                   outBuf(nullptr), outLen(0), outPtr(0) {

    }

    ~Connection() {
        if (inBuf != nullptr) {
            free(inBuf);
            inBuf = nullptr;
        }
        if (outBuf != nullptr) {
            free(outBuf);
            outBuf = nullptr;
        }
    }
};

#define MAX_EPOLL_SIZE 10000
#define MAX_BACKLOG 1024
#define MUTEX_COUNT 16

class Server {
private:
    Configuration conf;
    int listenfd; // listen socket fd
    int epollfd; // epoll fd
    ThreadPool *pool;
    std::array<std::shared_mutex, MUTEX_COUNT> mutexes;
    std::hash<std::string> hasher;

    MetaStorage *meta;

    // handle read events for multiple rounds
    static int handleRead(Connection *connection);

    // handle write events for multiple rounds
    static int handleWrite(Connection *connection);

    // handle requests and generate resp data
    // after read finishes, before write starts
    int handle(Connection *connection);

    void handlePut(Request *request, Response *response);

    void handleGet(Request *request, Response *response);

    void handleDelete(Request *request, Response *response);

    int createDirIfNotExists() const;

    inline int hash(std::string &key) {
        return (int) (hasher(key) & 0x7fffffff);
    }

public:
    Server(Configuration &conf) {
        this->conf = conf;
        this->pool = new ThreadPool(conf.threads);
        this->meta = new MetaStorage(conf.db_path);
    }

    ~Server() {
        close(listenfd);
        close(epollfd);
        delete meta;
        delete pool;
    }

    int start();
    void loop();
};

#endif //ENTRY_TASK_SERVER_H
