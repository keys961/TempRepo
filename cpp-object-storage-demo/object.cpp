#include <sys/socket.h>
#include <unistd.h>
#include <algorithm>
#include <netinet/in.h>
#include <cstring>
#include <iostream>
#include "object.h"

int readn(int fd, char* buf, int size) {
    char* pt = buf;
    int count = size;
    while (count > 0) {
        int c = count;
        size_t len = recv(fd, pt, c, 0);
        if (len == -1) {
            if(errno == EAGAIN) {
                continue;
            }
            return -1;
        } else if (len == 0) {
            return size - count;
        }
        pt += len;
        count -= (int)len;
    }
    return size;
}

int Request::read(char *data) {
    int ptr = 0;
    // length
    memcpy((char *) &len, data + ptr, sizeof(int));
    len = ntohl(len);
    ptr += sizeof(int);
    // method
    memcpy((char *) &method, data + ptr, sizeof(int));
    method = ntohl(method);
    ptr += sizeof(int);

    int keyLen = 0;
    memcpy((char *) &keyLen, data + ptr, sizeof(int));
    keyLen = ntohl(keyLen);
    ptr += sizeof(int);

    char str[keyLen + 1];
    memcpy(str, data + ptr, keyLen);
    str[keyLen] = 0;
    this->key = std::string(str);
    ptr += keyLen;

    if (len > 0) {
        buf = (char *) malloc(len);
        memcpy(buf, data + ptr, len);
    }
    return 0;
}

int writen(int fd, const char* msg, int size) {
    const char* buf = msg;
    int count = size;
    while (count > 0) {
        int c = count;
        size_t len = send(fd, buf, c, 0);
        if (len == -1) {
            if(errno == EAGAIN) {
                continue;
            }
            close(fd);
            return -1;
        } else if (len == 0) {
            continue;
        }
        buf += len;
        count -= (int)len;
    }
    return size - count;
}

int Request::write(int fd) {
    char *data = (char *) malloc(len + 4 * sizeof(int));
    // len
    int bigTotalLen = htonl(len + 3 * sizeof(int) + key.size());
    // buf
    int bigLen = htonl(len);
    int bigMethod = htonl(method);
    int bigStrLen = htonl(key.size());
    memcpy(data, &bigTotalLen, sizeof(int));
    memcpy(data + sizeof(int), &bigLen, sizeof(int));
    memcpy(data + 2 * sizeof(int), &bigMethod, sizeof(int));
    memcpy(data + 3 * sizeof(int), &bigStrLen, sizeof(int));
    memcpy(data + 4 * sizeof(int), key.data(), key.size());
    if (buf != nullptr && len > 0) {
        memcpy(data + 4 * sizeof(int) + key.size(), buf, len);
    }
    int ret = writen(fd, data, len + 4 * sizeof(int) + key.size());
    free(data);
    return ret;
}

int Request::getLen() const {
    return len;
}

void Request::setLen(int l) {
    Request::len = l;
}

int Request::getMethod() const {
    return method;
}

void Request::setMethod(int m) {
    Request::method = m;
}

char *Request::getBuf() const {
    return buf;
}

void Request::setBuf(char *data) {
    Request::buf = data;
}

const std::string &Request::getKey() const {
    return key;
}

void Request::setKey(const std::string &k) {
    Request::key = k;
}

int Response::read(int fd) {
    // total length
    int totalLength = 0;
    if (readn(fd, (char *) &totalLength, sizeof(int)) == -1) {
        close(fd);
        return -1;
    }
    // buf size
    if (readn(fd, (char *) &len, sizeof(int)) == -1) {
        close(fd);
        return -1;
    }
    len = ntohl(len);

    // status
    if (readn(fd, (char *) &status, sizeof(int)) == -1) {
        close(fd);
        return -1;
    }
    status = ntohl(status);

    // buffer data
    if(len > 0) {
        buf = (char*)malloc(len);
        int ret = readn(fd, buf, len);
        if(ret != len)
        {
            close(fd);
            free(buf);
            return -1;
        }
        buf[len] = 0;
    }

    return 2 * sizeof(int) + len;
}


int Response::write(char *data) {
    int bigTotalLen = htonl(2 * sizeof(int) + len);
    int bigLen = htonl(len);
    int bigStatus = htonl(status);
    memcpy(data, &bigTotalLen, sizeof(int));
    memcpy(data + sizeof(int), &bigLen, sizeof(int));
    memcpy(data + 2 * sizeof(int), &bigStatus, sizeof(int));
    if (len > 0) {
        memcpy(data + sizeof(int) * 3, buf, len);
    }
    return 0;
}


int Response::getLen() const {
    return len;
}

void Response::setLen(int len) {
    Response::len = len;
}

int Response::getStatus() const {
    return status;
}

void Response::setStatus(int status) {
    Response::status = status;
}

char *Response::getBuf() const {
    return buf;
}

void Response::setBuf(char *buf) {
    Response::buf = buf;
}
