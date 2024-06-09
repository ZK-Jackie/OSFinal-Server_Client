#include "global.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>


// 日志等级
const char *LOG_LEVEL_STR[] = {
        "[\033[34mINFO\033[0m]",
        "[\033[33mWARN\033[0m]",
        "[\033[31mERROR\033[0m]",
        "[\033[36mDEBUG\033[0m]",
        "[\033[35mCRITICAL\033[0m]",
        "[\033[33mDEPRECATED\033[0m]"
};

typedef struct logQueueNode{
    char *message;
    struct logQueueNode *next;
}LogNode;

typedef struct logQueue{
    LogNode *front;
    LogNode *rear;
    int size;
    sem_t mutex, undo;
}LogQueue;


LogQueue *logQueue = NULL;


void *loggerThreadFunc(void *args);
LogQueue *createQueue();
void enQueue(char *msg);
LogNode *deQueue();
bool isQueueEmpty();
void destroyQueue(LogQueue *queue);

void log_info(const char *msg, int level, ...);


/**
 * 初始化日志模块
 * @return logger对象
 */
Logger initLogger(){
    // 1. 初始化日志链队——头结点指向链表首结点和链表尾结点，元素从链表尾填入，从链表首结点取出
    logQueue = createQueue();
    // 2. 初始化信号量
    sem_init(&logQueue->mutex, 0, 1);
    sem_init(&logQueue->undo, 0, 0);
    // 3. 初始化logger对象
    pthread_t *loggerThread = (pthread_t *)malloc(sizeof(pthread_t));
    logger.thread = loggerThread;
    logger.info = log_info;
    logger.isLoggerThreadRunning = 1;
    // 4. 创建日志线程
    pthread_create(loggerThread, NULL, loggerThreadFunc, NULL);
    // 5. 成功信息
    logger.info("Logger Module is successfully initialized.", LOG_INFO);
    return logger;
}


/**
 * 销毁日志模块
 */
void destroyLogger(){
    // 1. 等待日志线程结束
    logger.isLoggerThreadRunning = 0;
    pthread_join(*logger.thread, NULL);
    // 2. 释放日志队列
    destroyQueue(logQueue);
    // 3. 释放信号量
    sem_destroy(&logQueue->mutex);
    sem_destroy(&logQueue->undo);
    // 4. 释放线程资源
    pthread_cancel(*logger.thread);
    pthread_join(*logger.thread, NULL);
    free(logger.thread);
    // 5. 释放logger对象
    logger.info = NULL;
    // 6. 打印销毁信息
    printf("Logger Module is destroyed.\n");
}


/**
 * 日志线程工作函数
 * @param args 线程参数
 */
void *loggerThreadFunc(void *args){
    int undoVal = 0;
    while (logger.isLoggerThreadRunning || undoVal){
        sem_wait(&logQueue->undo);
        sem_wait(&logQueue->mutex);
        LogNode *node = deQueue();
        printf("%s", node->message);
        free(node->message);
        free(node);
        sem_getvalue(&logQueue->undo, &undoVal);
        sem_post(&logQueue->mutex);
    }
    pthread_exit(NULL);
}


/**
 * 日志信息
 * @param msg 日志信息
 * @param level 日志等级
 * @param ... 日志信息中的可变参数，遵循printf的格式
 */
void log_info(const char *msg, int level, ...){
    if(msg == NULL){
        return;
    }
    // 保护时间准时性
    sem_wait(&logQueue->mutex);
    // 1. 计算长度
    int chunk_time_len = 25 + 1,
            chunk_msg_len = 0,
            chunk_threadId_len = 15 + 1,
            chunk_level_len = (int) strlen(LOG_LEVEL_STR[level]) + 1;
    // 2. 获取当前时间
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    char timeTemp[chunk_time_len];
    struct tm tm_info;
    // 线程安全版localtime，避免多线程在静态内存中的竞争
    localtime_r(&ts.tv_sec, &tm_info);
    strftime(timeTemp, chunk_time_len, "%Y-%m-%d %H:%M:%S", &tm_info);
    char chunk_time[chunk_time_len];
    snprintf(chunk_time, chunk_time_len, "%s.%03ld", timeTemp, ts.tv_nsec / 1000000);
    // 3. 获取当前线程ID
    char chunk_threadId[chunk_threadId_len];
    snprintf(chunk_threadId, chunk_threadId_len, "[thread-%ld]", pthread_self() % 10000);
    // 4. 组装日志内容
    va_list args;
    va_start(args, level);
    // 复制，计算信息长度
    va_list argsCopy;
    va_copy(argsCopy, args);
    chunk_msg_len = vsnprintf(NULL, 0, msg, argsCopy);
    va_end(argsCopy);
    // 获取格式化信息
    char chunk_msg[chunk_msg_len + 1];
    vsnprintf(chunk_msg, chunk_msg_len + 1, msg, args);
    va_end(args);
    // 5. 最后组装：时间 + 等级 + 当前线程ID +信息
    int str_len = chunk_time_len + chunk_level_len + chunk_threadId_len + chunk_msg_len + 1;
    char *str = (char *)malloc(str_len + 1);
    memset(str, 0, str_len + 1);
    sprintf(str, "%s\t%s\t%s\t%s\n", chunk_time, LOG_LEVEL_STR[level], chunk_threadId, chunk_msg);
    // 6. 入队
    enQueue(str);
    sem_post(&logQueue->mutex);
    sem_post(&logQueue->undo);
}


/********************************* 队列操作 *********************************/
LogQueue *createQueue(){
    LogQueue *queue = (LogQueue *)malloc(sizeof(LogQueue));
    queue->front = NULL;
    queue->rear = NULL;
    queue->size = 0;
    return queue;
}


void destroyQueue(LogQueue *queue){
    LogNode *node = queue->front;
    while (node != NULL){
        LogNode *temp = node;
        node = node->next;
        free(temp->message);
        free(temp);
    }
    free(queue);
    queue = NULL;
}


void enQueue(char *msg){
    // 1. 创建新节点
    LogNode *new = (LogNode *) malloc(sizeof(LogNode));
    // 检查malloc的返回值
    if (new == NULL) {
        fprintf(stderr, "Failed to allocate memory for new LogNode.\n");
        exit(EXIT_FAILURE);
    }
    new->message = msg;
    new->next = NULL;
    // 2. 入队
    if (logQueue->size == 0) {
        logQueue->front = new;
        logQueue->rear = new;
    } else {
        logQueue->rear->next = new;
        logQueue->rear = new;
    }
    logQueue->size++;
}


LogNode *deQueue(){
    if(isQueueEmpty()){
        exit(EXIT_FAILURE);
    }
    // 1. 出队
    LogNode *ret = logQueue->front;
    logQueue->front = logQueue->front->next;
    logQueue->size--;
    // 2. 返回
    return ret;
}


bool isQueueEmpty(){
    return logQueue->size == 0;
}