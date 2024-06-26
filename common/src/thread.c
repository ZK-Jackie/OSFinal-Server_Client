#include "global.h"
#include "thread.h"
#include <stdio.h>
#include <stdlib.h>

ThreadPool *pool = NULL;

void *routine();

TaskBufferQueue *initBufferQ(int bufferSize);
void destroyBufferQ(TaskBufferQueue **bufferQueue);


/**
 * 初始化线程池
 * @param threadNum 线程数量
 * @param bufferSize 缓冲区大小
 * @param callback 回调函数
 * @param ... 回调函数所需参数，可选
 * */
void initPools(int threadNum, int bufferSize, void (*callback)(void *arg), ...) {
    logger.info("Thread pool & buffer pool init.", LOG_INFO);
    pool = (ThreadPool *) malloc(sizeof(ThreadPool));
    // 1. 缓冲任务环形队列初始化
    TaskBufferQueue *initBuffers = initBufferQ(bufferSize);
    // 2. 线程池信号量初始化
    bool isSemInitSuccess = false;
    isSemInitSuccess = sem_init(&pool->idle, 0, threadNum) == 0;
    isSemInitSuccess |= sem_init(&pool->work, 0, 0) == 0;
    isSemInitSuccess |= sem_init(&pool->mutex, 0, 1) == 0;
    if (!isSemInitSuccess) {
        exit_error("Couldn't init semaphore for unknown reason.");
    }
    // 3. 线程数组初始化，创建线程
    bool isThreadCreateSuccess = false;
    pthread_t *initThreadList = (pthread_t *) malloc(sizeof(pthread_t) * threadNum);
    for (int i = 0; i < threadNum; i++) {
        isThreadCreateSuccess = !pthread_create(&initThreadList[i], NULL, routine, NULL);
        if (!isThreadCreateSuccess) {
            exit_error("Couldn't create thread for unknown reason.");
        }
    }
    // 4.装
    pool->buffer = initBuffers;
    pool->bufferSize = bufferSize;
    pool->threadNum = threadNum;
    pool->threadList = initThreadList;
    // 5. 回调
    if (callback != NULL) {
        // 获取形参列表，列表有一个参数
        callback(NULL);
    }
    logger.info("Thread pool & buffer pool init success.", LOG_INFO);
}


/**
 * 销毁线程池
 * */
void destroyPools() {
    logger.info("Thread pool & buffer pool destroy.", LOG_INFO);
    running = false;
    // 1. 释放线程
    for (int i = 0; i < pool->threadNum; i++) {
        pthread_cancel(pool->threadList[i]);
    }
    free(pool->threadList);
    // 2. 释放缓冲区
    destroyBufferQ(&pool->buffer);
    // 3. 释放信号量
    sem_destroy(&pool->idle);
    sem_destroy(&pool->work);
    sem_destroy(&pool->mutex);
    // 4. 释放线程池
    free(pool);
    pool = NULL;
    logger.info("Thread pool & buffer pool destroy success.", LOG_INFO);
}

/**
 * 初始化缓冲区
 * @param bufferSize 缓冲区大小
 * @return 缓冲队列
 * */
TaskBufferQueue *initBufferQ(int bufferSize) {
    // 1. 构造&初始化缓冲环形队列
    TaskNode *head = (TaskNode *) malloc(sizeof(TaskNode));    // 头节点
    if (head == NULL) {
        fprintf(stderr, "Couldn't allocate memory for buffer head.\n");
        exit(EXIT_FAILURE);
    }
    head->spot = 0;
    head->isDone = true;
    head->data = NULL;
    head->next = NULL;
    TaskNode *temp = head;  // 指向尾节点
    for (int i = 0; i < bufferSize; ++i) {
        TaskNode *new = (TaskNode *) malloc(sizeof(TaskNode)); // 新节点
        if (new == NULL) {
            fprintf(stderr, "Couldn't allocate memory for buffer node.\n");
            exit(EXIT_FAILURE);
        }
        new->spot = i + 1;
        new->isDone = true;
        new->data = NULL;
        temp->next = new;
        temp = new;
    }
    temp->next = head;  // 尾节点指回头节点
    // 2. 装
    TaskBufferQueue *bufferQueue = (TaskBufferQueue *) malloc(sizeof(TaskBufferQueue));
    if (bufferQueue == NULL) {
        fprintf(stderr, "Couldn't allocate memory for buffer queue.\n");
        exit(EXIT_FAILURE);
    }
    bufferQueue->head = head;
    bufferQueue->size = bufferSize;
    bufferQueue->front = head; // 初始时都指向头节点
    bufferQueue->rear = head;
    // 3.队列信号量初始化
    bool isSemInitSuccess = false;
    isSemInitSuccess = sem_init(&bufferQueue->mutex, 0, 1) == 0;
    isSemInitSuccess |= sem_init(&bufferQueue->undo, 0, 0) == 0;
    isSemInitSuccess |= sem_init(&bufferQueue->done, 0, bufferSize) == 0;
    if (!isSemInitSuccess) {
        fprintf(stderr, "Couldn't init buffer semaphore for unknown reason.\n");
        exit(EXIT_FAILURE);
    }
    // 3. 返回
    return bufferQueue;
}

void destroyBufferQ(TaskBufferQueue **bufferQueue) {
    // 释放缓冲区
    TaskNode *temp = pool->buffer->head;
    TaskNode *next = NULL;
    for (int i = 0; i <= pool->bufferSize; i++) {
        next = temp->next;
        free(temp);
        temp = next;
    }
    // 释放信号量
    sem_destroy(&pool->buffer->mutex);
    sem_destroy(&pool->buffer->undo);
    sem_destroy(&pool->buffer->done);
    // 释放缓冲区
    free(pool->buffer);
    *bufferQueue = NULL;
}


/**
 * 添加任务到缓冲区
 * @param task 要执行的任务
 * @return 是否添加成功
 * */
bool addTask(Task *task) {
    // 1. 等待空闲位置和缓冲区锁
    sem_wait(&pool->buffer->done);
    sem_wait(&pool->buffer->mutex);
    // 2. 添加任务
    TaskNode *node = pool->buffer->rear;
    node->data = task;
    node->isDone = false;
    pool->buffer->rear = node->next;
    // 3. 释放锁
    sem_post(&pool->buffer->mutex);
    sem_post(&pool->buffer->undo);
    sem_post(&pool->work);
    return true;
}


/**
 * 发送执行任务给线程
 * @return 任务
 * */
Task *pushTask() {
    // 等待队列中有任务
    sem_wait(&pool->buffer->undo);
    // 获取队列的互斥锁
    sem_wait(&pool->buffer->mutex);

    // 从队列的队头取出一个任务，原位置清空，并将队头指针向后移动一位
    TaskNode *node = pool->buffer->front;
    Task *task = node->data;
    node->data = NULL;
    node->isDone = true;
    pool->buffer->front = node->next;

    // 释放队列的互斥锁
    sem_post(&pool->buffer->mutex);
    // 增加队列的空闲位置数量
    sem_post(&pool->buffer->done);

    // 返回取出的任务
    return task;
}


/**
 * 判断缓冲区是否已满
 * @return 是否已满
 * @note 由于采用的是环形队列，本环形队列中rear所指向的位置一直是空的，&#10;所以当rear下一个为front时，说明缓冲区已满
 * */
bool isBufferFull() {
    return pool->buffer->rear->next == pool->buffer->front;
}


/**
 * 判断缓冲区是否为空
 * @return 是否为空
 * @note 由于采用的是环形队列，当front和rear指向同一位置时，说明缓冲区为空
 * */
bool isBufferEmpty() {
    return pool->buffer->front == pool->buffer->rear;
}


/**
 * 线程工作函数
 * */
void *routine() {
    while (running) {
        // 1. 等待有任务
        if(sem_wait(&pool->work) == -1){
            break;
        }
        // 2. 获取并执行任务
        Task *task = pushTask();
        task->func(task->arg);
        // 3. 释放空闲信号量
        sem_post(&pool->idle);
        // 4. 释放任务
        free(task->arg);
        free(task);
    }
    pthread_exit(NULL);
}