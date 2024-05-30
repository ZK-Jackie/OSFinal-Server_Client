#include "global.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>


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
void enQueue(char *msg);
LogNode *deQueue();
bool isQueueEmpty();

void log_info(const char *msg, int level, ...);


Logger initLogger(){
    // 1. 初始化日志链队
    logQueue = (LogQueue *)malloc(sizeof(LogQueue));
    logQueue->front = NULL;
    logQueue->rear = NULL;
    logQueue->size = 0;
    // 2. 初始化信号量
    sem_init(&logQueue->mutex, 0, 1);
    sem_init(&logQueue->undo, 0, 0);
    // 3. 初始化logger对象
    pthread_t *loggerThread = (pthread_t *)malloc(sizeof(pthread_t));
    logger.thread = loggerThread;
    logger.info = log_info;
    // 4. 创建日志线程
    pthread_create(loggerThread, NULL, loggerThreadFunc, NULL);
    // 5. 成功信息
    logger.info("Logger Module is successfully initialized.", LOG_INFO);
    return logger;
}



void *loggerThreadFunc(void *args){
    while (1){
        sem_wait(&logQueue->undo);
        sem_wait(&logQueue->mutex);
        LogNode *node = deQueue();
        printf("%s", node->message);
        free(node->message);
        free(node);
        sem_post(&logQueue->mutex);
    }
    pthread_exit(NULL);
}

void enQueue(char *msg){
    // 1. 创建新节点
    LogNode *new = (LogNode *) malloc(sizeof(LogNode));
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


void log_info(const char *msg, int level, ...){
    // 0. 等待日志模块初始化完成
    while (logger.info == NULL);
    // 1. 计算长度
    int chunk1_len = 30,
    chunk2_len = 0,
    chunk_info_len = (int)sizeof(LOG_LEVEL_STR[level]) + 1;
    // 1. 获取当前时间
    struct timeval tv;
    char buffer[30];
    struct tm* tm_info;
    gettimeofday(&tv, NULL);
    tm_info = localtime(&tv.tv_sec);
    strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);
    char chunk1[chunk1_len];
    sprintf(chunk1, "%s.%03ld", buffer, tv.tv_usec % 1000);
    // 2. 组装日志内容
    va_list args;
    va_start(args, level);
    // 复制，计算信息长度
    va_list argsCopy;
    va_copy(argsCopy, args);
    chunk2_len = vsnprintf(NULL, 0, msg, argsCopy);
    va_end(argsCopy);
    // 获取格式化信息
    char chunk2[chunk2_len + 1];
    vsnprintf(chunk2, chunk2_len + 1, msg, args);
    va_end(args);
    // 3. 最后组装
    int str_len = chunk1_len + chunk_info_len + chunk2_len + 3;
    char *str = (char *)malloc(str_len + 1);
    memset(str, 0, str_len + 1);
    sprintf(str, "%s\t%s\t%s\n", chunk1, LOG_LEVEL_STR[level], chunk2);
    // 4. 入队
    sem_wait(&logQueue->mutex);
    enQueue(str);
    sem_post(&logQueue->mutex);
    sem_post(&logQueue->undo);
}





