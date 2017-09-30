#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

#define MSGKEY 1024
#define MAXNUMOFCONNECT 100

extern int errno;

int currentStatus = 0; //0: unconnect, 1: connect
struct ReceiveArgs arg; //需要变为全局变量，如果定义在connectToServer函数中，函数返回时变量就被销毁了，而线程执行的函数中有指向这个变量的指针，
                        //如果connect函数返回后线程才开始执行，那么指针所指向的内容就是不确定的。

struct ClientInfo 
{
    int connectfd; //与客户端连接的服务端socket描述符
    int id; //客户端编号
    char ip[20]; //客户端ip
    int port; //客户端端口号
};

struct Request 
{
    int type; //表示请求的类型。0表示请求时间，1表示请求服务器名字，2表示请求客户端列表，3表示请求转发消息，4表示服务端向客户端转发消息
    int id; //接受转发消息的目标客户端编号
    char message[200]; //消息
};

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



struct ReceiveArgs
{
    int sockfd;
    struct sockaddr_in serverAddr;
};

void printMenu();
//void* receiveMessage(int sockfd, struct sockaddr_in serverAddr);//Sub-thread (TODO)
void* receiveMessage(void* arg);
//Menu operations
void connectToServer(int sockfd, struct sockaddr_in* serverAddr);
void disconnectFromServer(int sockfd, struct sockaddr_in serverAddr);
void getTimeFromServer(int sockfd, struct sockaddr_in serverAddr); //CMD: "TIME" (TODO)
void getNameFromServer(int sockfd, struct sockaddr_in serverAddr); //CMD: "NAME" (TODO)
void getClientsFromServer(int sockfd, struct sockaddr_in serverAddr); //CMD: "LIST" (TODO)
void sendMsgToAnotherClient(int sockfd, struct sockaddr_in serverAddr); //CMD: "$ID $MSG"

int main(int argc, char *argv[]) //1, Server IP;
{
    int sockfd;
    struct sockaddr_in serverAddr;

    if(argc != 2)//TODO: need modify;
    {
        printf("Need args: Server_IP");
        return -1;
    }

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("Error: Fail to create socket. ERRNO: %d\n", errno);
        return -1;
    }

    //Set server ip & port
    //bzero(&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(argv[1]);
    serverAddr.sin_port = htons(2100);
    bzero(&(serverAddr.sin_zero), 8);

    //msgq
    int msgId = msgget(MSGKEY, IPC_CREAT | 0666);
    if(msgId < 0)
    {
        printf("ERROR: Cannot create the message queue. ERRNO: %d.\n", errno);
        return -1;
    }

    int selectOption = 0;
    printMenu();
    scanf("%d", &selectOption);
    while(selectOption != 7)
    {
        switch(selectOption)
        {
            case 1:
                connectToServer(sockfd, &serverAddr);
                break;
            case 2:
                disconnectFromServer(sockfd, serverAddr);
                break;
            case 3:
                getTimeFromServer(sockfd, serverAddr);
                //printMsg(msgId);
                break;
            case 4:
                getNameFromServer(sockfd, serverAddr);
                //printMsg(msgId);
                break;
            case 5:
                getClientsFromServer(sockfd, serverAddr);
                //printMsg(msgId);
                break;
            case 6:
                sendMsgToAnotherClient(sockfd, serverAddr);
                //printMsg(msgId);
                break;
        }
        scanf("%d", &selectOption);
    }
    if(currentStatus)
        disconnectFromServer(sockfd, serverAddr);
    printf("Bye!\n");

    return 0;
}

void printMenu()
{
    printf("Please select one of the options below:\n");
    printf("1. Connect to the server.\n");
    printf("2. Disconnect from the server.\n");
    printf("3. Get current time from the server.\n");
    printf("4. Get the name of the server.\n");
    printf("5. Get the info of all the clients connected to the server.\n");
    printf("6. Send messages to a specified client connected to the server.\n");
    printf("7. Exit.\n");
}

void* receiveMessage(void* arg)
{
    int sockfd = ((struct ReceiveArgs*)arg)->sockfd;
    struct sockaddr_in serverAddr = ((struct ReceiveArgs*)arg)->serverAddr;

    struct Response resp; 
    while(currentStatus > 0)//connected
    {
        int totalLen = sizeof(resp);
        int len;
        bzero((void*)&resp, sizeof(resp));
        void* ptr = &resp;
        while((len = recv(sockfd, ptr, totalLen, MSG_DONTWAIT)) && totalLen)
        {
            //printf("Receiving...\n");
            if(len <= 0)
            {
                if(errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR)
                {
                    printf("ERROR: Receive messages failed. ERRNO: %d.\n", errno);
                    exit(-1);
                }
            }
            else
            {
                totalLen -= len;
                ptr = (char*)ptr + len;
            }
        }

        switch(resp.type)
        {
            case 0: 
                printf("Current time: %s\n", ctime(&resp.timeInSecond));
                break;
            case 1:
                printf("Server name: %s\n", resp.serverName);
                break;
            case 2:
                for(int i = 0; i < resp.numOfConnect; i++)
                    printf("Client ID: %d, IP: %s, Port: %d.\n", resp.clientList[i].id, resp.clientList[i].ip, resp.clientList[i].port);
                break;
            case 3:
                if(!resp.success)
                    printf("Send message success!\n");
                else
                    printf("Send message error!, MSG: %s\n", resp.error);
                break;
            case 4:
                printf("From client %d, ip = %s, port = %d: %s\n", resp.client.id, resp.client.ip, resp.client.port, resp.message);
                break;
        }
    }
}

void connectToServer(int sockfd, struct sockaddr_in* serverAddr)
{
    if(currentStatus == 1)
    {
        printf("Message: The client has already been connected to the server!\n");
        return;
    }

    if(connect(sockfd, (struct sockaddr*)serverAddr, sizeof(struct sockaddr)) == -1)
    {
        //if(errno == EINPROGRESS)
        {
            printf("ERROR: Cannot connect to the server! ERRNO: %d\n", errno);
            close(sockfd);
            exit(-1);
        }
    }
    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
    currentStatus = 1;
    printf("Connect success!\n");
    //invoke a thread
    pthread_t subThread;
    arg.serverAddr = *serverAddr;
    arg.sockfd = sockfd;
    if(pthread_create(&subThread, NULL, (void*)receiveMessage, (void*)&arg))
    {
        printf("ERROR: Create sub-thread failed!\n");
        close(sockfd);
        exit(-1);
    }
}

void disconnectFromServer(int sockfd, struct sockaddr_in serverAddr)
{
    if(currentStatus == 0)
    {
        printf("Message: The client is not connected to the server.\n");
       // connectToServer(sockfd, serverAddr);
        return;
    }
    char msg[50] = "EXIT";
    send(sockfd, msg, sizeof(msg), MSG_DONTWAIT);
    currentStatus = 0;//Automatically kill the sub-thread
    close(sockfd);
    printf("The client has been disconnected from the server.\n");
}

void getTimeFromServer(int sockfd, struct sockaddr_in serverAddr)
{
    if(currentStatus == 0)
    {
        printf("ERROR: The client has not been connected to the server!\n");
       // connectToServer(sockfd, serverAddr);
       return;
    }
    struct Request reqTime;
    reqTime.type = 0;
    if(send(sockfd, (void*)&reqTime, sizeof(reqTime), MSG_DONTWAIT) < 0)
    {
        printf("ERROR: Request error! ERRNO: %d\n",errno);
        return;
    }
}

void getNameFromServer(int sockfd, struct sockaddr_in serverAddr)
{
    if(currentStatus == 0)
    {
        printf("ERROR: The client has not been connected to the server!\n");
        return;
    }
    struct Request reqName; //Command: NAME
    reqName.type = 1;
    if(send(sockfd, (void*)&reqName, sizeof(reqName), MSG_DONTWAIT) < 0)
    {
        printf("ERROR: Request error! ERRNO: %d\n",errno);
        return;
    }
}

void getClientsFromServer(int sockfd, struct sockaddr_in serverAddr)
{
    if(currentStatus == 0)
    {
        printf("ERROR: The client has not been connected to the server!\n");
        return;
    }
    struct Request reqList; //Command: LIST
    reqList.type = 2;
    if(send(sockfd, (void*)&reqList, sizeof(reqList), MSG_DONTWAIT) < 0)
    {
        printf("ERROR: Request error! ERRNO: %d\n",errno);
        return;
    }
}

void sendMsgToAnotherClient(int sockfd, struct sockaddr_in serverAddr)
{
    if(currentStatus == 0)
    {
        printf("ERROR: The client has not been connected to the server!\n");
        return;
    }
    printf("Please input your destination client's id and message ended with ENTER:\n");
    printf("Format: $id $msg.\n");
    struct Request req;
    req.type = 3;
    scanf("%d", &req.id);
    scanf("%s", req.message);
    int n = 0;
    int len = sizeof(struct Request);
    void* ptr = &req;
    while((n = send(sockfd, ptr, len - n, MSG_DONTWAIT)))
    {
        if(n <= 0)
        {
            printf("ERROR: Send message error! ERNNO: %d\n", errno);
            return;
        }
        ptr = (char*)ptr + n;
    }
}