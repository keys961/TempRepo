/**
 *   Queue: before get into area
 *   Cross: between the area
 *   后面possible不再死锁是因为死锁的那一次后，四个队列发车有时间差，
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define MAX_THREAD_NUM 1000

#define NORTH 1
#define EAST 2
#define SOUTH 3
#define WEST 4

typedef struct CarQueue //Data structure of a queue
{
    int carId[MAX_THREAD_NUM]; //Ids
    int front = 0;
    int rear = 0;
    int count = 0;
}CarQueue;

//Mutex for area a,b,c,d 
sem_t mutexA;
sem_t mutexB;
sem_t mutexC;
sem_t mutexD;
int remainA = 1; // Remain number of area of a-d
int remainB = 1;
int remainC = 1;
int remainD = 1;
//Mutex for deadlock thread
pthread_mutex_t mutexDeadlock;
//Mutex for queue of 4 directions
pthread_mutex_t mutexQueueNorth;
pthread_mutex_t mutexQueueEast;
pthread_mutex_t mutexQueueSouth;
pthread_mutex_t mutexQueueWest;
//Mutex for the cross of 4 directions
pthread_mutex_t mutexCrossNorth;
pthread_mutex_t mutexCrossEast;
pthread_mutex_t mutexCrossSouth;
pthread_mutex_t mutexCrossWest;

//Cond for the queue of 4 directions
pthread_cond_t condQueueNorth;
pthread_cond_t condQueueEast;
pthread_cond_t condQueueSouth;
pthread_cond_t condQueueWest;
CarQueue queueEast; int currentEastId = 0;//Queue for 4 directions , popped results
CarQueue queueWest; int currentWestId = 0;
CarQueue queueSouth; int currentSouthId = 0;
CarQueue queueNorth; int currentNorthId = 0;
//Cond for the cross of 4 directions
pthread_cond_t condCrossNorth;
pthread_cond_t condCrossEast;
pthread_cond_t condCrossSouth;
pthread_cond_t condCrossWest;
//Cond of deadlock thread
pthread_cond_t condDeadlock;
//Cond to wake up the car on the right (To wake up the waiting car)
pthread_cond_t condLock;
//All car threads
pthread_t carThreads[MAX_THREAD_NUM]; 
pthread_t deadlockThread; //Deadlock detector & resolver thread
int carIds[MAX_THREAD_NUM];//Store id

int flagEast = 0; // Whether the car has left the queue (ready bit)
int flagSouth = 0;
int flagWest = 0;
int flagNorth = 0;
int isDeadlock = 0; // Whether there is a deadlock
int currentDirection = 0;

void *threadDeadlock(void *arg)
{
    int val1, val2, val3, val4;
    sleep(1);
    //First wake up first car
    if(queueNorth.count > 0)
    {
        queueNorth.count--;
        currentNorthId = queueNorth.carId[queueNorth.front++];
        pthread_cond_signal(&condQueueNorth);
    }
    if(queueEast.count > 0)
    {
        queueEast.count--;
        currentEastId = queueEast.carId[queueEast.front++];
        pthread_cond_signal(&condQueueEast);
    }
    if(queueWest.count > 0)
    {
        queueWest.count--;
        currentWestId = queueWest.carId[queueWest.front++];
        pthread_cond_signal(&condQueueWest);
    }
    if(queueSouth.count > 0)
    {
        queueSouth.count--;
        currentSouthId = queueSouth.carId[queueSouth.front++];
        pthread_cond_signal(&condQueueSouth);
    }

    //Check deadlock
    sleep(3);
    while(1)
    {
        sem_getvalue(&mutexA, &val1);
        sem_getvalue(&mutexB, &val2);
        sem_getvalue(&mutexC, &val3);
        sem_getvalue(&mutexD, &val4);
       // printf("Checking: %d %d %d %d\n", val1, val2, val3, val4);
        if(remainA == 0 && remainD == 0 && remainB == 0 && remainC == 0) //Deadlock happend
        {
            printf("DEADLOCK: Car jam detected! Signalling north to go.\n");
            pthread_cond_signal(&condCrossNorth);
            if(val4 == 0)
                sem_post(&mutexD);
            //remainD = 1; //Because north car want D area
        }
        sleep(2);//sleep(1);
    }
}

void *threadNorth(void *arg)
{
    int val = 0;
    pthread_mutex_lock(&mutexQueueNorth); //lock queue north
    pthread_cond_wait(&condQueueNorth, &mutexQueueNorth); //wait for pop out of queue
    flagNorth = 1;
    pthread_mutex_unlock(&mutexQueueNorth);
    //Go to next, get area C
    sem_wait(&mutexC);
    sleep(1);
    remainC = 0;
    printf("Car %d from North arrives at crossing.\n", currentNorthId); //Arrive at crossing
    pthread_mutex_lock(&mutexCrossNorth);
    if(flagWest)
        pthread_cond_wait(&condCrossNorth, &mutexCrossNorth); //Wait for the right car signal to go
    pthread_mutex_unlock(&mutexCrossNorth);
    //Go to area D
    sem_wait(&mutexD); //Lock D
    //while(remainD == 0){} //No remainD, stucked
    remainC = 1; //Restore
    flagNorth = 0;
    sem_getvalue(&mutexC, &val);
    if(val == 0)
        sem_post(&mutexC); //Unlock C
    
     //Send signal to the left car
    printf("Car %d from North leaves at crossing.\n", currentNorthId);
    pthread_cond_signal(&condCrossEast);
    sleep(1);
    sem_getvalue(&mutexD, &val);
    if(val == 0)
        sem_post(&mutexD); //Unlock C
    if(queueNorth.count > 0)//Awake next car if there still cars left
    {
        currentNorthId = queueNorth.carId[queueNorth.front++];
        queueNorth.count--;
        sleep(5);
        pthread_cond_signal(&condQueueNorth); //wake up next
    }
}

void *threadEast(void *arg)
{
    int val = 0;
    pthread_mutex_lock(&mutexQueueEast); //lock queue East
    pthread_cond_wait(&condQueueEast, &mutexQueueEast); //wait for pop out of queue
    flagEast = 1;
    pthread_mutex_unlock(&mutexQueueEast);
    //Go to next, get area B
    sem_wait(&mutexB);
    sleep(1);
    remainB = 0;
    printf("Car %d from East arrives at crossing.\n", currentEastId); //Arrive at crossing
    pthread_mutex_lock(&mutexCrossEast);
    if(flagNorth)
        pthread_cond_wait(&condCrossEast, &mutexCrossEast); //Wait for the right car signal to go
    pthread_mutex_unlock(&mutexCrossEast);
    //Go to area C
    sem_wait(&mutexC); //Lock C
   // while(remainC == 0){} //No remainC, stucked
    
    remainB = 1; //Restore
    flagEast = 0;
    sem_getvalue(&mutexB, &val);
    if(val == 0)
        sem_post(&mutexB); //Unlock B
    printf("Car %d from East leaves at crossing.\n", currentEastId);
    pthread_cond_signal(&condCrossSouth); //Send signal to the left car that able to move
    sleep(1);
    
    sem_getvalue(&mutexC, &val);
    if(val == 0)
        sem_post(&mutexC); //Unlock C
    if(queueEast.count > 0)//Awake next car if there still cars left
    {
        currentEastId = queueEast.carId[queueEast.front++];
        queueEast.count--;
        sleep(5);
        pthread_cond_signal(&condQueueEast); //wake up next car
    }
}

void *threadSouth(void *arg)
{
    int val = 0;
    pthread_mutex_lock(&mutexQueueSouth); //lock queue south
    pthread_cond_wait(&condQueueSouth, &mutexQueueSouth); //wait for pop out of queue
    flagSouth = 1; //Set ready
    pthread_mutex_unlock(&mutexQueueSouth);
    //Go to next, get area A
    sem_wait(&mutexA);
    sleep(1);
    remainA = 0;
    printf("Car %d from South arrives at crossing.\n", currentSouthId); //Arrive at crossing
    pthread_mutex_lock(&mutexCrossSouth);
    if(flagEast) //If there is east car ready
        pthread_cond_wait(&condCrossSouth, &mutexCrossSouth); //Wait for the right car signal to go
    pthread_mutex_unlock(&mutexCrossSouth);
    //Go to area B
    sem_wait(&mutexB); //Lock B
    remainA = 1; //Restore
    flagSouth = 0;
    sem_getvalue(&mutexA, &val);
    if(val == 0)
        sem_post(&mutexA); //Unlock A
    //while(remainB == 0) {} //No remainB, stucked
    printf("Car %d from South leaves at crossing.\n", currentSouthId);
    pthread_cond_signal(&condCrossWest); //Send signal to the left car
    sleep(1);
    
    
    
    sem_getvalue(&mutexB, &val);
    if(val == 0)
        sem_post(&mutexB); //Unlock B
    if(queueSouth.count > 0)//Awake next car if there still cars left
    {
        currentSouthId = queueSouth.carId[queueSouth.front++];
        queueSouth.count--;
        sleep(5);
        pthread_cond_signal(&condQueueSouth); //wake up next
    }
}

void *threadWest(void *arg)
{
    int val = 0;
    pthread_mutex_lock(&mutexQueueWest); //lock queue West
    pthread_cond_wait(&condQueueWest, &mutexQueueWest); //wait for pop out of queue
    flagWest = 1;
    pthread_mutex_unlock(&mutexQueueWest);
    //Go to next, get area C
    sem_wait(&mutexD);
    sleep(1);
    remainD = 0;
    printf("Car %d from West arrives at crossing.\n", currentWestId); //Arrive at crossing
    pthread_mutex_lock(&mutexCrossWest);
    if(flagSouth)
        pthread_cond_wait(&condCrossWest, &mutexCrossWest); //Wait for the right car signal to go
    pthread_mutex_unlock(&mutexCrossWest);
    //Go to area D
    sem_wait(&mutexA); //Lock A
    //while(remainB == 0) {}
    
    remainD = 1; //Restore
    flagWest = 0;
    sem_getvalue(&mutexD, &val);
    if(val == 0)
        sem_post(&mutexD); //Unlock D
    printf("Car %d from West leaves at crossing.\n", currentWestId);
    pthread_cond_signal(&condCrossNorth); //Send signal to the left car
    sleep(1);
    
    
    sem_getvalue(&mutexA, &val);
    if(val == 0)
        sem_post(&mutexA); //Unlock A
    if(queueWest.count > 0)//Awake next car if there still cars left
    {
        currentWestId = queueWest.carId[queueWest.front++];
        queueWest.count--;
        sleep(5);
        pthread_cond_signal(&condQueueWest); //wake up next
    }
}

int main(int argc, char const *argv[])
{
    if(argc < 2)
        printf("No input arg!\n");

    // init mutex & cond vars
    {
        sem_init(&mutexA, 0, 1);
        sem_init(&mutexB, 0, 1);
        sem_init(&mutexC, 0, 1);
        sem_init(&mutexD, 0, 1);
        pthread_mutex_init(&mutexDeadlock, NULL);
        pthread_mutex_init(&mutexQueueNorth, NULL);
        pthread_mutex_init(&mutexQueueEast, NULL);
        pthread_mutex_init(&mutexQueueSouth, NULL);
        pthread_mutex_init(&mutexQueueWest, NULL);
        pthread_mutex_init(&mutexCrossNorth, NULL);
        pthread_mutex_init(&mutexCrossEast, NULL);
        pthread_mutex_init(&mutexCrossSouth, NULL);
        pthread_mutex_init(&mutexCrossWest, NULL);
        pthread_cond_init(&condQueueNorth, NULL);
        pthread_cond_init(&condQueueEast, NULL);
        pthread_cond_init(&condQueueSouth, NULL);
        pthread_cond_init(&condQueueWest, NULL);
        pthread_cond_init(&condCrossNorth, NULL);
        pthread_cond_init(&condCrossEast, NULL);
        pthread_cond_init(&condCrossSouth, NULL);
        pthread_cond_init(&condCrossWest, NULL);
        pthread_cond_init(&condDeadlock, NULL);
        pthread_cond_init(&condLock, NULL);
    }

    int len = strlen(argv[1]); //Get number of cars
    for(int i = 0; i < len; i++)
    {
        carIds[i] = i + 1;
        switch(argv[1][i]) //Create threads
        {
            case 'e':
                flagEast = 1; //There's a car ready to go
                queueEast.carId[queueEast.rear++] = i + 1; //Push the car into the queue
                queueEast.count++; //Update count
                pthread_create(&carThreads[i], NULL, threadEast, NULL); //Create thread
                break;
            case 'n':
                flagNorth = 1; //There's a car ready to go
                queueNorth.carId[queueNorth.rear++] = i + 1; //Push the car into the queue
                queueNorth.count++; //Update count
                pthread_create(&carThreads[i], NULL, threadNorth, NULL); //Create thread
                break;
            case 'w':
                flagWest = 1; //There's a car ready to go
                queueWest.carId[queueWest.rear++] = i + 1; //Push the car into the queue
                queueWest.count++; //Update count
                pthread_create(&carThreads[i], NULL, threadWest, NULL); //Create thread
                break;
            case 's':
                flagSouth = 1; //There's a car ready to go
                queueSouth.carId[queueSouth.rear++] = i + 1; //Push the car into the queue
                queueSouth.count++; //Update count
                pthread_create(&carThreads[i], NULL, threadSouth, NULL); //Create thread
                break;
        }
    }

    pthread_create(&deadlockThread, NULL, threadDeadlock, NULL); //Create deadlock thread

    for(int i = 0; i < len; i++)
        pthread_join(carThreads[i], NULL); //Join & Exit
    //pthread_cancel(deadlockThread); //Exit deadlock thread
    return 0;
}