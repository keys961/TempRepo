#ifndef ENTRY_TASK_OBJECT_H
#define ENTRY_TASK_OBJECT_H

#include <string>

#define METHOD_PUT 0
#define METHOD_DELETE 1
#define METHOD_GET 2

/**
 * Format: buf len (4) + method (4) + key len(4) + key (var size) +  buf (var size)
 */
class Request {
public:

    int write(int fd); // for client end
    int read(char *buf); // for server end

    Request() {
        buf = nullptr;
        len = 0;
        method = -1;
    }

    ~Request() = default;

    // Getter & Setter
    int getLen() const;

    void setLen(int l);

    int getMethod() const;

    void setMethod(int method);

    const std::string &getKey() const;

    void setKey(const std::string &key);

    char *getBuf() const;

    void setBuf(char *buf);

private:
    int len; // buf len
    int method; // method
    std::string key; // key
    char *buf; // data with len
};

#define RESP_INVALID (-2)
#define RESP_EXCEPTION (-1)
#define RESP_OK 0
#define RESP_NOT_FOUND 1

/**
 * Format: len (4) + status (4) + buf (var size)
 */
class Response {
public:
    int read(int fd); // for client end
    int write(char *buf); // for server end
    Response() {
        buf = nullptr;
        status = RESP_OK;
        len = 0;
    }

    ~Response() {
        if (buf != nullptr) {
            free(buf);
        }
        buf = nullptr;
        len = 0;
    }

    static std::string statusToString(int status) {
        switch (status) {
            case 0: return "OK";
            case 1: return "NOT_FOUND";
            case -2: return "METHOD_INVALID";
            default:
                return "EXCEPTION";
        }
    }

    // Getter & Setter
    int getLen() const;
    void setLen(int len);
    int getStatus() const;
    void setStatus(int status);
    char *getBuf() const;
    void setBuf(char *buf);
private:
    int len; // buf len
    int status; // return status
    char *buf; // only used in GET

};

#endif //ENTRY_TASK_OBJECT_H
