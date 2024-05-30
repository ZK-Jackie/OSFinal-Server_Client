#ifndef SERVER_THREAD_H
#define SERVER_THREAD_H

#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include "param.h"

typedef struct task{
    void *arg;
    void (*func)(void *);
}Task;

typedef struct taskQueueBufferNode{
    int spot;
    bool isDone;
    Task *data;
    struct taskQueueBufferNode *next;
}TaskNode;

typedef struct taskQueueBuffer {
    TaskNode *head;
    struct taskQueueBufferNode *front;
    struct taskQueueBufferNode *rear;
    int size;
    sem_t done, undo, mutex;
}TaskBufferQueue;

typedef struct pool{
    /** 参数 **/
    int bufferSize;
    int threadNum;
    /** 实例 **/
    TaskBufferQueue *buffer;
    pthread_t *threadList;
    /** 锁 **/
    sem_t idle, work, mutex;
}ThreadPool;


void initPools(ServerParams args);
bool addTask(Task *task);

// 池
extern ThreadPool *pool;


#endif //SERVER_THREAD_H
