#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <pthread.h>
#include <netinet/in.h>   
#include <arpa/inet.h>
#include <unistd.h>

#define MAXNUMOFCONNECT 100

//存储客户端信息
struct ClientInfo {
    int connectfd; //与客户端连接的服务端socket描述符
    int id; //客户端编号
    char ip[20]; //客户端ip
    int port; //客户端端口号
};

//请求包
struct Request {
    int type; //表示请求的类型。0表示请求时间，1表示请求服务器名字，2表示请求客户端列表，3表示请求转发消息
    int id; //接受转发消息的目标客户端编号
    char message[200]; //消息
};

//响应包
struct Response {
    int type; //同请求包，增加4表示服务端向客户端转发消息
    time_t timeInSecond; //返回时间。表示从公元 1970 年1 月1 日的UTC 时间从0 时0 分0 秒算起到现在所经过的秒数。响应的转换由客户端完成
    char serverName[20]; //服务器名字
    struct ClientInfo clientList[MAXNUMOFCONNECT]; //客户端列表
    int numOfConnect; //服务器的连接的客户端的数量。
    int success; //0表示消息转发成功，1表示消息转发失败
    char error[50]; //保存转发消息时的出错信息
    struct ClientInfo client; //保存源客户端信息
    char message[200]; //消息
};

struct ClientInfo clientList[MAXNUMOFCONNECT]; //维护一个客户端列表
pthread_t threadPool[MAXNUMOFCONNECT]; //维护一个线程池。每个线程负责响应一个客户端
int flag[MAXNUMOFCONNECT]; //用于pthread_create函数的参数传递，使得threadPool[i]对应clientList[i]
int count = -1; //用于计数

void *threadFunc(void *vptr_args);
int recvOrSendAll(int socketfd, void *buffer, size_t length, int flag);
void response(int socketfd, struct Request *request, int clientId);
void timeResponse(struct Response *response);
void nameResponse(struct Response *response);
void clientListResponse(struct Response *response);
void messageResponse(struct Response *response, struct Request *request, int clientId);

int main() {
    //初始化一个socket
    int socketfd = socket(AF_INET, SOCK_STREAM, 0);

    //初始化失败
    if (socketfd == -1) {
        printf("create socket failed!");
        exit(0);
    }

    // 绑定端口
    struct sockaddr_in sockAddr;

    memset(&sockAddr, 0, sizeof(struct sockaddr_in));

    sockAddr.sin_family = AF_INET;
    sockAddr.sin_port = htons(2100);
    sockAddr.sin_addr.s_addr = INADDR_ANY;

    int res = bind(socketfd, (const struct sockaddr *)&sockAddr, sizeof(struct sockaddr_in));

    //绑定端口失败
    if (res == -1) {
        printf("bind failed!");
        exit(0);
    }

    //监听端口
    res = listen(socketfd, 20);

    //监听是比
    if (res == -1) {
        printf("listen failed!");
        exit(0);
    }

    //初始化客户端列表和flag
    for (int i = 0;i < MAXNUMOFCONNECT;i++) {
        clientList[i].connectfd = -1;
        flag[i] = i;
    }
    
    //服务器响应连接请求
    while (1) {
        //接受连接请求
        int connectfd = accept(socketfd, NULL, NULL);

        //连接失败
        if (connectfd < 0) {
            printf("accept failed!");
            close(socketfd);
            exit(0);
        }

        //计数器加一
        ++count;
        count = count % MAXNUMOFCONNECT;

        //由于服务器设有最大连接数，如果现在服务器已经满负荷，并且有新的连接请求，则最早的连接将被关闭
        if (clientList[count].connectfd != -1) {
            close(clientList[count].connectfd);
        }

        //填充客户端信息
        memset(&clientList[count], 0, sizeof(struct ClientInfo));

        clientList[count].connectfd = connectfd;
        clientList[count].id = count;

        struct sockaddr_in clientAddr;
        unsigned int len = sizeof(struct sockaddr_in);

        getpeername(connectfd, (struct sockaddr*)&clientAddr, &len);

        strcpy(clientList[count].ip, inet_ntoa(clientAddr.sin_addr));
        clientList[count].port = ntohs(clientAddr.sin_port);

        printf("one user come in!\nid=%d ip=%s port=%d\n", clientList[count].id, clientList[count].ip, clientList[count].port);
        
        //创建新的线程
        if (pthread_create(&threadPool[count], NULL, threadFunc, &flag[count]) != 0) {
            printf("create thread failed!");
            exit(0);
        }
    }
}

//线程所要执行的函数
void *threadFunc(void *vptr_args) {
    //获取对应的客户端编号
    int id = *((int *)vptr_args);
    //获取套接字描述符
    int fd = clientList[id].connectfd;

    while (1) {
        //接受请求包
        struct Request *request = (struct Request *)malloc(sizeof(struct Request));

        if (!recvOrSendAll(fd, request, sizeof(struct Request), 0)) {
            printf("can not receive all data!");
            clientList[id].connectfd = -1;
            return NULL;
        }

        //响应
        response(fd, request, id);
    }

    return NULL;
}


//接受或发送数据包，falg为0表示接受数据包，1表示发送数据包。之所以需要这个函数，是因为单纯的调用recv或send函数可能并不能完整的接受或发送数据包。
int recvOrSendAll(int socketfd, void *buffer, size_t length, int flag) {
    char *ptr = (char *)buffer;

    while (length > 0) {
        int i = flag == 0 ? recv(socketfd, ptr, length, 0) : send(socketfd, ptr, length, 0);

        if (i < 1)
            return 0;

        ptr += i;
        length -= i;
    }

    return 1;
}

//响应函数
void response(int socketfd, struct Request *request, int clientId) {
    //初始化响应数据包
    struct Response *response = (struct Response *)malloc(sizeof(struct Response));

    memset(response, 0, sizeof(struct Response));

    //根据请求的类型装配响应包
    switch (request->type) {
        case 0: timeResponse(response); break;
        case 1: nameResponse(response); break;
        case 2: clientListResponse(response); break;
        case 3: messageResponse(response, request, clientId); break;
    }

    //发送响应包
    recvOrSendAll(socketfd, response, sizeof(struct Response), 1);    
}

//响应时间请求
void timeResponse(struct Response *response) {
    response->type = 0;
    response->timeInSecond = time(NULL);
}

//响应名字请求
void nameResponse(struct Response *response) {
    response->type = 1;
    strcpy(response->serverName, "Yusen");
}

//响应客户端列表请求
void clientListResponse(struct Response *response) {
    response->type = 2;

    int index = 0;

    for (int i = 0;i < MAXNUMOFCONNECT;i++) {
        if (clientList[i].connectfd != -1) {
            response->clientList[index++] = clientList[i];
        }
    }

    response->numOfConnect = index;
}

//响应消息转发请求
void messageResponse(struct Response *response, struct Request *request, int clientId) {
    response->type = 3;

    if (request->id >= MAXNUMOFCONNECT) {
        response->success = 1;
        strcpy(response->error, "invalid id!");
    } else if (clientList[request->id].connectfd == -1 ) {
        response->success = 1;
        strcpy(response->error, "no connect with target host!");
    } else {
        struct Response *response1 = (struct Response *)malloc(sizeof(struct Response));

        response1->type = 4;
        response1->client = clientList[clientId];
        response1->client.connectfd = -1;
        strcpy(response1->message, request->message);

        if (recvOrSendAll(clientList[request->id].connectfd, response1, sizeof(struct Response), 1)) {
            response->success = 0;
        } else {
            response->success = 1;
            strcpy(response->error, "send message failed!");
        }
    }
}