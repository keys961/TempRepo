#include <netinet/in.h>
#include <cstring>
#include <arpa/inet.h>
#include "client.h"
#include "file.h"

int Client::connect() {
    struct sockaddr_in servaddr{};
    connfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    inet_pton(AF_INET, addr.c_str(), &servaddr.sin_addr);

    if (::connect(connfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        perror("connect error");
        return -1;
    }

    printf("connection established\n");
    return 0;
}

void Client::get(std::string &key, Response *resp) const {
    Request req;
    req.setKey(key);
    req.setMethod(METHOD_GET);
    req.setLen(0);

    if (req.write(connfd) < 0) {
        perror("send req error: ");
        resp->setStatus(RESP_EXCEPTION);
        return;
    }
    if (resp->read(connfd) < 0) {
        perror("recv resp error: ");
        resp->setStatus(RESP_EXCEPTION);
        return;
    }
}

void Client::put(std::string &key, char *buf, int len, Response *resp) const {
    Request req;
    req.setKey(key);
    req.setMethod(METHOD_PUT);
    req.setLen(len);
    req.setBuf(buf);

    if (req.write(connfd) < 0) {
        perror("send req error: ");
        resp->setStatus(RESP_EXCEPTION);
        return;
    }
    if (resp->read(connfd) < 0) {
        perror("recv resp error: ");
        resp->setStatus(RESP_EXCEPTION);
        return;
    }
}

void Client::del(std::string &key, Response *resp) const {
    Request req;
    req.setKey(key);
    req.setMethod(METHOD_DELETE);
    req.setLen(0);

    if (req.write(connfd) < 0) {
        perror("send req error: ");
        resp->setStatus(RESP_EXCEPTION);
        return;
    }

    if(resp->read(connfd) < 0) {
        perror("recv resp error: ");
        resp->setStatus(RESP_EXCEPTION);
        return;
    }
}

__attribute__((unused)) int Client::save(std::string &path, std::string &key, Response *resp) {
    int len = resp->getLen();
    char *buf = resp->getBuf();
    if (resp->getBuf() == nullptr) {
        fprintf(stderr, "no data, will not be saved.\n");
        return -1;
    }
    FileIO io(path, key, "wb+");
    if (io.writeData(buf, len) < len) {
        perror("write data to local storage failed: ");
        return -1;
    }
    return 0;
}
