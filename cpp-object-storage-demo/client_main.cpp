#include <thread>
#include <sys/time.h>
#include <cstring>
#include <csignal>
#include "client.h"

// 1K 10K 100K 1MB 10MB 100MB
static char* requestBuf = nullptr;
static int len = 100;

struct TestArgs {
    std::string addr;
    int port;
    std::string method;
    std::string payload;
    int count;
    int threadId;
};

int getPayload(std::string& payload) {
    if (payload == "1k") {
        len = (1 << 10);
    } else if (payload == "10k") {
        len = (1 << 10) * 10;
    } else if (payload == "100k") {
        len = (1 << 10) * 100;
    } else if (payload == "1m") {
        len = (1 << 20);
    } else if (payload == "10m") {
        len = (1 << 20) * 10;
    } else if (payload == "100m") {
        len = (1 << 20) * 100;
    } else {
        len = 10;
    };
    return len;
}

double interval(struct timeval *start, struct timeval *end) {
    double d;
    time_t s;
    suseconds_t u;
    s = end->tv_sec - start->tv_sec;
    u = end->tv_usec - start->tv_usec;
    d = (double) s;
    d *= 1000000.0;
    d += (double) u;
    return d / 1000.0;
}

bool testPut(Client* client, std::string key, bool functional) {
    Response resp;
    client->put(key, requestBuf, len, &resp);
    if(functional) {
        printf("Response for PUT: status %s.\n", Response::statusToString(resp.getStatus()).data());
    }
    return resp.getStatus() == RESP_OK;
}

bool testGet(Client* client, std::string key, bool functional) {
    Response resp;
    client->get(key, &resp);
    if(functional) {
        printf("Response for GET: status %s, payload len: %d\n",
               Response::statusToString(resp.getStatus()).data(), resp.getLen());
        if(resp.getBuf() != nullptr) {
            if (memcmp(requestBuf, resp.getBuf(), len) == 0) {
                printf("Response compares to request: payload is same.\n");
            } else {
                printf("Response compares to request: not same.\n");
            }
        }
    }
    return resp.getStatus() == RESP_OK;
}

bool testDelete(Client* client, std::string key, bool functional) {
    Response resp;
    client->del(key, &resp);
    if(functional) {
        printf("Response for DELETE: status %s.\n", Response::statusToString(resp.getStatus()).data());
    }
    return resp.getStatus() == RESP_OK;
}

void testPerformanceTask(TestArgs args) {
    struct timeval start{}, end{};

    Client client(args.addr, args.port);
    client.connect();

    int errorCount = 0;
    gettimeofday(&start, nullptr);
    if(args.method == "GET") {
        for(int i = 0; i < args.count; i++) {
            if(!testGet(&client, std::to_string(i) + "-" + std::to_string(args.threadId), false)) {
                errorCount += 1;
            }
        }
    } else if(args.method == "PUT") {
        for(int i = 0; i < args.count; i++) {
            if(!testPut(&client, std::to_string(i) + "-" + std::to_string(args.threadId), false)) {
                errorCount += 1;
            }
        }
    } else {
        for(int i = 0; i < args.count; i++) {
            if(!testDelete(&client, std::to_string(i) + "-" + std::to_string(args.threadId), false)) {
                errorCount += 1;
            }
        }
    }
    gettimeofday(&end, nullptr);

    double diff = interval(&start, &end); // in ms
    printf("thread finished with %d req in %.2f sec. error cnt: %d\n", args.count, diff / 1000.0, errorCount);
}

int main(int argc, char **argv) {
    signal(SIGPIPE, SIG_IGN);
    if(argc == 5) {
        printf("For performance test. Format:./client addr port method count-per-thread threads.\n");
        std::string addr = std::string(argv[1]);
        int port = atoi(argv[2]);
        std::string method = std::string(argv[3]);
        std::string payload = std::string(argv[4]);
        len = getPayload(payload);
        if (method == "PUT" || method == "GET") {
            len = getPayload(payload);
            requestBuf = (char *) malloc(sizeof(char) * len);
            memset(requestBuf, 'a', len);
            requestBuf[len] = 0;
        }
        Client client(addr, port);
        client.connect();
        if(method == "PUT") {
            testPut(&client, "test", true);
        } else if(method == "GET") {
            testGet(&client, "test", true);
        } else {
            testDelete(&client, "test", true);
        }

        if (requestBuf != nullptr) {
            free(requestBuf);
        }
        return 0;
    } else if (argc == 7){
        printf("For functional test. Format:./client addr port method payload.\n");
        std::string addr = std::string(argv[1]);
        int port = atoi(argv[2]);
        std::string method = std::string(argv[3]);
        std::string payload = std::string(argv[4]);
        int countPerThread = atoi(argv[5]);
        int threadCount = atoi(argv[6]);

        TestArgs args{
                addr, port, method, payload, countPerThread
        };

        if (method == "PUT" || method == "GET") {
            len = getPayload(payload);
            requestBuf = (char *) malloc(sizeof(char) * len);
            memset(requestBuf, 'a', len);
            requestBuf[len] = 0;
        }

        std::thread threads[threadCount];
        struct timeval start{}, end{};
        gettimeofday(&start, nullptr);
        for (int i = 0; i < threadCount; i++) {
            args.threadId = i;
            threads[i] = std::thread(testPerformanceTask, args);
        }

        for (auto &t: threads) {
            t.join();
        }

        gettimeofday(&end, nullptr);
        printf("finally finished with in %.2f sec. \n", interval(&start, &end) / 1000.0);
        if (requestBuf != nullptr) {
            free(requestBuf);
        }
        return 0;
    } else {
        fprintf(stderr, "For performance test. Format:./client addr port method payload count-per-thread threads.\n");
        fprintf(stderr, "For functional test. Format:./client addr port method payload.\n");
        return 1;
    }
}
