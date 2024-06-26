#include "global.h"
#include "param.h"
#include "isocket.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#define CLIENT_APP_VERSION "1.0.0"

typedef struct {
    char uid[10];
    int threadNum;
    int sleepTime;
    struct hostent *server;
    pthread_barrier_t *concurBarrier;
    pthread_mutex_t *fifoMutex;
} RequestSettings;

void concur(RequestSettings settings);
void *concurRequest(void *args);

void fifo(RequestSettings settings);
void *fifoRequest(void *args);

void endHandler();


void run(int argc, char *argv[]) {
    signal(SIGUSR1, endHandler);
    // 1. 初始化日志线程
    initDevice(NULL);
    // 2. 解析参数
    ClientParams params = parseArgs(argc, argv);
    // 3. 初始化socket
    struct hostent *server = gethostbyname("localhost");
    if (server == NULL) {
        exit_error("No such host called 'localhost'");
    }
    // 4. 配置&运行
    RequestSettings requestSettings = {
            .threadNum = params.threadNum,
            .sleepTime = params.sleepTime,
            .server = server
    };
    if (!params.workload) {
        logger.info("Running in concurrent mode", LOG_INFO);
        concur(requestSettings);
    } else {
        logger.info("Running in FIFO mode", LOG_INFO);
        fifo(requestSettings);
    }
}

void endHandler() {

}

void concur(RequestSettings settings) {
    pthread_t *thread = malloc(settings.threadNum * sizeof(pthread_t));
    settings.concurBarrier = (pthread_barrier_t *) malloc(sizeof(pthread_barrier_t));
    pthread_barrier_init(settings.concurBarrier, NULL, settings.threadNum); // 初始化屏障
    int res;
    char concurUid[12];
    for (int i = 0; i < settings.threadNum; i++) {
        // 1. 准备生成uid
        memset(concurUid, 0, 12);
        snprintf(concurUid, 10, "concur-%d", i);
        // 2. 为每个线程创建独立的设置
        RequestSettings *threadSettings = (RequestSettings *) malloc(sizeof(RequestSettings));
        (*threadSettings) = settings;  // 复制settings属性
        memset(threadSettings->uid, 0, sizeof(threadSettings->uid));
        strcpy(threadSettings->uid, concurUid); // 复制uid
        // 3. 创建线程
        res = pthread_create(&thread[i], NULL, concurRequest, threadSettings);
        if (res != 0) {
            exit_error("Failed to create thread");
        }
    }
    // 等待线程结束
    for (int i = 0; i < settings.threadNum; i++) {
        pthread_join(thread[i], NULL);
    }
    free(thread);
    free(settings.concurBarrier);
    thread = NULL;
    settings.concurBarrier = NULL;
}

void *concurRequest(void *args) {
    RequestSettings *settings = (RequestSettings *) args;
    while (running) {
        // 1. 准备请求数据
        char sendBuffer[POST_CONTENT_LEN];
        memset(sendBuffer, 0, POST_CONTENT_LEN);
        snprintf(sendBuffer, POST_CONTENT_LEN, "{\"name\":\"%s\", \"mode\": \"concurrent\"}", settings->uid);
        logger.info("[%s]\tRequest: %s.", LOG_INFO, settings->uid, sendBuffer);
        // 2. 发送请求
        char *res = req.post(settings->server, sendBuffer);
        // 3. 处理响应
        logger.info("[%s]\tReceive from: %s, response: %s", LOG_INFO, settings->uid, getHeader(res, "Server"), getLoad(res));
        free(res);  // 释放请求strdup返回的内存
        pthread_barrier_wait(settings->concurBarrier); // 屏障可以重复利用
        // 4. 让权
        sleep(settings->sleepTime);
    }
    free(settings);
    pthread_exit(NULL);
}


void fifo(RequestSettings settings) {
    pthread_t *fifoThread = malloc(settings.threadNum * sizeof(pthread_t));
    settings.fifoMutex = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(settings.fifoMutex, NULL); // 初始化互斥锁
    int res;
    char fifoUid[12];
    for (int i = 0; i < settings.threadNum; i++) {
        // 1. 准备生成uid
        memset(fifoUid, 0, 12);
        snprintf(fifoUid, 10, "fifo-%d", i);
        // 2. 为每个线程创建独立的设置
        RequestSettings *threadSettings = (RequestSettings *) malloc(sizeof(RequestSettings));
        (*threadSettings) = settings;  // 复制settings属性
        memset(threadSettings->uid, 0, sizeof(threadSettings->uid));
        strcpy(threadSettings->uid, fifoUid); // 复制uid
        // 3. 创建线程
        strcpy(settings.uid, fifoUid);
        res = pthread_create(&fifoThread[i], NULL, fifoRequest, threadSettings);
        if (res != 0) {
            exit_error("Failed to create thread");
        }
    }
    // 等待线程结束
    for (int i = 0; i < settings.threadNum; i++) {
        pthread_join(fifoThread[i], NULL);
    }
    free(fifoThread);
    fifoThread = NULL;
}

void *fifoRequest(void *args) {
    RequestSettings *settings = (RequestSettings *) args;
    while (running) {
        pthread_mutex_lock(settings->fifoMutex);
        // 1. 准备请求数据
        char sendBuffer[GET_CONTENT_LEN];
        memset(sendBuffer, 0, GET_CONTENT_LEN);
        snprintf(sendBuffer, GET_CONTENT_LEN, "name=%s&mode=fifo", settings->uid);
        logger.info("[%s]\tRequest: %s.", LOG_INFO, settings->uid, sendBuffer);
        // 2. 发送请求，获得回应
        char *res = req.get(settings->server, sendBuffer);
        // 3. 处理响应
        logger.info("[%s]\tReceive from: %s, response: %s", LOG_INFO, settings->uid, getHeader(res, "Server"), getLoad(res));
        free(res);  // 释放请求strdup返回的内存
        pthread_mutex_unlock(settings->fifoMutex);
        sleep(settings->sleepTime);
    }
    free(settings);
    pthread_exit(NULL);
}