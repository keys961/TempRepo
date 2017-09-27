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

#define MSGKEY 1024

extern int errno;

int currentStatus = 0; //0: unconnect, 1: connect

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
    serverAddr.sin_port = htons(6666);
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
    // int msqId = msgget(MSGKEY, IPC_EXCL); //Create msg queue
    // if(msqId < 0)
    // {
    //     printf("ERROR: Failed to find the message queue! ERRNO: %d\n", errno);
    //     exit(-1);
    // }

    char buffer[2048] = {0,};
    while(currentStatus > 0)//connected
    {
       // printf("IN subthread\n");
        int totalLen = 0;
        int len;
        bzero((void*)buffer, sizeof(buffer));
        void* ptr = buffer;
        if((len = recv(sockfd, ptr, sizeof(buffer), MSG_DONTWAIT)))
        {
            //printf("Receiving...\n");
            if(len < 0)
            {
                if(errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR)
                {
                    printf("ERROR: Receive messages failed. ERRNO: %d.\n", errno);
                    exit(-1);
                }
            }
            else
                printf("Received: %s\n", buffer);
        }
        // if(totalLen > 0) //If msg exists
        // {
        //     //print to msg
        //     printf("%s\n", buffer);
        // }
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
    struct ReceiveArgs arg;
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
    char msg[128] = "TIME"; //Command: TIME
    if(send(sockfd, (void*)msg, sizeof(msg), MSG_DONTWAIT) < 0)
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
    char msg[128] = "NAME"; //Command: NAME
    if(send(sockfd, (void*)msg, sizeof(msg), MSG_DONTWAIT) < 0)
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
    char msg[128] = "LIST"; //Command: LIST
    if(send(sockfd, (void*)msg, sizeof(msg), MSG_DONTWAIT) < 0)
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
    char msg[1030];
    getchar();
    gets(msg);
    int n = 0;
    char* ptr = msg;
    while((n = send(sockfd, ptr, strlen(ptr), MSG_DONTWAIT)))
    {
        if(n <= 0)
        {
            printf("ERROR: Send message error! ERNNO: %d\n", errno);
            return;
        }
        ptr = ptr + n;
    }
}
