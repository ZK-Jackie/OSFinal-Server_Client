#include "thread.h"
#include <stdio.h>
#include <stdlib.h>

ThreadPool *pool = NULL;

void *routine(void *arg);

TaskBufferQueue *buffer_init(int bufferSize);


/**
 * 池子自动管理机
 * */
void *pool_auto_manager(void *arg) {
    ServerParams args = *((ServerParams*) arg);
    // 1. 初始化线程池
    pool_init(args);
    // 2. 回调
    if (args.callback != NULL) {
        // 获取形参列表，列表有一个参数
        args.callback(NULL);
    }
    // 3. 维持线程池
    for (int i = 0; i < args.THREAD_NUM; i++) {
        pthread_join(pool->threadList[i], NULL);
    }
    return NULL;
}


/**
 * 线程池维持
 * */
void *pool_maintain(){
    for (int i = 0; i < pool->threadNum; i++) {
        pthread_join(pool->threadList[i], NULL);
    }
    return NULL;
}


/**
 * 初始化线程池
 *
 * @param threadNum 线程数量
 * @param bufferSize 缓冲区大小
 * @param isCallback 是否回调
 * @param ... 回调函数
 * */
void pool_init(ServerParams args) {
    pool = (ThreadPool *) malloc(sizeof(ThreadPool));
    // 1. 线程数组初始化
    bool isThreadCreateSuccess = false;
    pthread_t *initThreadList = (pthread_t *) malloc(sizeof(pthread_t) * args.THREAD_NUM);
    for (int i = 0; i < args.THREAD_NUM; i++) {
        int *arg = (int *) malloc(sizeof(int));
        if (arg == NULL) {
            fprintf(stderr, "Couldn't allocate memory for thread arg when index = %d .\n", i);
            exit(EXIT_FAILURE);
        }
        *arg = i;
        isThreadCreateSuccess = !pthread_create(&initThreadList[i], NULL, routine, arg);
        if (!isThreadCreateSuccess) {
            fprintf(stderr, "Couldn't create thread for unknown reason when index = %d .\n", i);
            exit(EXIT_FAILURE);
        }
    }
    // 2. 信号量初始化
    bool isSemInitSuccess = false;
    isSemInitSuccess = sem_init(&pool->idle, 0, args.THREAD_NUM) == 0;
    isSemInitSuccess |= sem_init(&pool->work, 0, 0) == 0;
    isSemInitSuccess |= sem_init(&pool->mutex, 0, 1) == 0;
    if (!isSemInitSuccess) {
        fprintf(stderr, "Couldn't init thread semaphore for unknown reason.\n");
        exit(EXIT_FAILURE);
    }
    // 3. 缓冲任务环形队列初始化，能到此处，缓冲池创建一定成功
    TaskBufferQueue *initBuffers = buffer_init(args.BUFFER_SIZE);
    // 4.装
    pool->buffer = initBuffers;
    pool->bufferSize = args.BUFFER_SIZE;
    pool->threadNum = args.THREAD_NUM;
    pool->threadList = initThreadList;
    // 5. 回调
    if (args.callback != NULL) {
        // 获取形参列表，列表有一个参数
        args.callback(NULL);
    }
}

TaskBufferQueue *buffer_init(int bufferSize) {
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
    TaskBufferQueue *buffer_queue = (TaskBufferQueue *) malloc(sizeof(TaskBufferQueue));
    if (buffer_queue == NULL) {
        fprintf(stderr, "Couldn't allocate memory for buffer queue.\n");
        exit(EXIT_FAILURE);
    }
    buffer_queue->head = head;
    buffer_queue->size = bufferSize;
    buffer_queue->front = head; // 初始时都指向头节点
    buffer_queue->rear = head;
    // 2.1 信号量初始化
    bool isSemInitSuccess = false;
    isSemInitSuccess = sem_init(&buffer_queue->mutex, 0, 1) == 0;
    isSemInitSuccess |= sem_init(&buffer_queue->undo, 0, 0) == 0;
    isSemInitSuccess |= sem_init(&buffer_queue->done, 0, bufferSize) == 0;
    if (!isSemInitSuccess) {
        fprintf(stderr, "Couldn't init buffer semaphore for unknown reason.\n");
        exit(EXIT_FAILURE);
    }
    // 3. 返回
    return buffer_queue;
}


/**
 * 添加任务到缓冲区
 *
 * @param task 要执行的任务
 * @return 是否添加成功
 * */
bool add_task(Task *task) {
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
 *
 * @return 任务
 * */
Task *push_task() {
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
 *
 * @return 是否已满
 * @note 由于采用的是环形队列，本环形队列中rear所指向的位置一直是空的，&#10;所以当rear下一个为front时，说明缓冲区已满
 * */
bool isBufferFull() {
    return pool->buffer->rear->next == pool->buffer->front;
}

/**
 * 判断缓冲区是否为空
 *
 * @return 是否为空
 * @note 由于采用的是环形队列，当front和rear指向同一位置时，说明缓冲区为空
 * */
bool isBufferEmpty() {
    return pool->buffer->front == pool->buffer->rear;
}


/**
 * 线程工作函数
 *
 * @param arg 线程编号
 * @return 无返回值
 * */
void *routine(void *arg) {
    int id = *((int *) arg);
    // TODO work
    while (1) {
        // 1. 等待有任务
        sem_wait(&pool->work);
        // 2. 获取并执行任务
        Task *task = push_task();
        task->func(task->arg);
        // 3. 释放空闲信号量
        sem_post(&pool->idle);
    }
    free(arg);
}