#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <unistd.h>

extern int errno;

int currentStatus = 0; //0: unconnect, 1: connect

void printMenu();
void receiveMessage();//Sub-thread (TODO)
//Menu operations
void connectToServer(int sockfd, struct sockaddr_in serverAddr);
void disconnectFromServer(int sockfd, struct sockaddr_in serverAddr);
void getTimeFromServer(int sockfd, struct sockaddr_in serverAddr); //CMD: "TIME" (TODO)
void getNameFromServer(int sockfd, struct sockaddr_in serverAddr); //CMD: "NAME" (TODO)
void getClientsFromServer(int sockfd, struct sockaddr_in serverAddr); //CMD: "LIST" (TODO)
void sentMsgToAnotherClient(int sockfd, struct sockaddr_in serverAddr); //CMD: "$ID $MSG"

int main(int argc, char *argv[]) //1, Server IP; 2, Server Port
{
    int sockfd;
    struct sockaddr_in serverAddr;

    if(argc != 3)//TODO: need modify;
    {
        printf("Need args: Server_IP, Server_Port");
        return -1;
    }

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0) == -1)
    {
        printf("Error: Fail to create socket. ERRNO: %d\n", errno);
        return -1;
    }

    //Set server ip & port
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(atoi(argv[2]));
    serverAddr.sin_addr = inet_addr(argv[1]);
    bzero(&(serverAddr.sin_zero), 8);

    int selectOption = 0;
    printMenu();
    scanf("%d", &selectOption);
    while(selectOption != 7)
    {
        system("clear");
        switch(selectOption)
        {
            case 1:
                connectToServer(sockfd, serverAddr);
                break;
            case 2:
                disconnectFromServer(sockfd, serverAddr);
                break;
            case 3:
                getTimeFromServer(sockfd, serverAddr);
                break;
            case 4:
                getNameFromServer(sockfd, serverAddr);
                break;
            case 5:
                getClientsFromServer(sockfd, serverAddr);
                break;
            case 6:
                sentMsgToAnotherClient(sockfd, serverAddr);
                break;
            case 7:
                break;
            default:
                printf("Error: Input command error!\n");
        }
    }

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

void receiveMessage()//TODO
{
    while(currentStatus)//connected
    {

    }
}

void connectToServer(int sockfd, struct sockaddr_in serverAddr)
{
    if(currentStatus == 1)
    {
        printf("Message: The client has already been connected to the server!\n");
        return;
    }

    if(connect(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1)
    {
        printf("Error: Cannot connect to the server! ERRNO: %d\n", errno);
        return;
    }

    currentStatus = 1;
    //invoke a thread
}

void disconnectFromServer(int sockfd, struct sockaddr_in serverAddr)
{
    if(currentStatus == 0)
    {
        printf("Message: The client is not connected to the server.\n");
        return;
    }
    currentStatus = 0;
    close(sockfd);
    printf("The client has been disconnected from the server.\n");
    //Automatically kill the sub-thread
}

void getTimeFromServer(int sockfd, struct sockaddr_in serverAddr)
{
    if(currentStatus == 0)
    {
        printf("ERROR: The client has not been connected to the server!\n");
        return;
    }
    char msg[10] = "TIME"; //Command: TIME
    if(send(sockfd, (void*)msg, strlen(msg), 0) < 0)
    {
        printf("ERROR: Request error! ERRNO: %d\n",errno);
        return;
    }
    //Wait for sub-thread
}

void getNameFromServer(int sockfd, struct sockaddr_in serverAddr)
{
    if(currentStatus == 0)
    {
        printf("ERROR: The client has not been connected to the server!\n");
        return;
    }
    char msg[10] = "NAME"; //Command: NAME
    if(send(sockfd, (void*)msg, strlen(msg), 0) < 0)
    {
        printf("ERROR: Request error! ERRNO: %d\n",errno);
        return;
    }
    //Wait for sub-thread
}

void getClientsFromServer(int sockfd, struct sockaddr_in serverAddr)
{
    if(currentStatus == 0)
    {
        printf("ERROR: The client has not been connected to the server!\n");
        return;
    }
    char msg[10] = "LIST"; //Command: LIST
    if(send(sockfd, (void*)msg, strlen(msg), 0) < 0)
    {
        printf("ERROR: Request error! ERRNO: %d\n",errno);
        return;
    }
    //Wait for sub-thread
}

void sentMsgToAnotherClient(int sockfd, struct sockaddr_in serverAddr)
{
    if(currentStatus == 0)
    {
        printf("ERROR: The client has not been connected to the server!\n");
        return;
    }
    printf("Please input your destination client's id and message ended with ENTER:\n");
    printf("Format: $id $msg.\n");
    char msg[1030];
    gets(msg);
    if(send(sockfd, (void*)msg, strlen(msg), 0) < 0)
    {
        printf("ERROR: Request error! ERRNO: %d\n",errno);
        return;
    }
}
