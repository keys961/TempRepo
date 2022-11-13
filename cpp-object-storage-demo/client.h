#ifndef ENTRY_TASK_CLIENT_H
#define ENTRY_TASK_CLIENT_H


#include <string>
#include <utility>
#include <unistd.h>
#include "object.h"

class Client {
private:
    std::string addr;
    int port;
    int connfd;

public:
    Client(std::string &addr, int port) {
        this->addr = addr;
        this->port = port;
        this->connfd = 0;
    }

    ~Client() {
        if (connfd > 0) {
            close(connfd);
        }
    }

    int connect();

    void get(std::string &key, Response *resp) const;

    void put(std::string &key, char *buf, int len, Response *resp) const;

    void del(std::string &key, Response *resp) const;

    __attribute__((unused)) static int save(std::string &path, std::string &key, Response *resp);
};

#endif //ENTRY_TASK_CLIENT_H
