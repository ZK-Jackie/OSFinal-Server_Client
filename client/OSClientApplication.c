#include "utils/param.h"
#include "socket/isocket.h"
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct{
    int threadNum;
    int sleepTime;
    struct hostent *server;
    pthread_barrier_t concurBarrier;
    pthread_mutex_t fifoMutex;
}RequestSettings;

void concur(RequestSettings settings);
void *concurRequest(void *args);

void fifo(RequestSettings settings);
void *fifoRequest(void *args);


void run(ClientParams params) {
    struct hostent *server;
    server = gethostbyname("localhost");

    RequestSettings requestSettings = {params.threadNum, params.sleepTime, server};
    if (!params.workload) {
        concur(requestSettings);
    } else {
        fifo(requestSettings);
    }
}

void concur(RequestSettings settings) {
    pthread_t *thread = malloc(settings.threadNum * sizeof(pthread_t));
    pthread_barrier_init(&settings.concurBarrier, NULL, settings.threadNum); // 初始化屏障
    for (int i = 0; i < settings.threadNum; i++) {
        pthread_create(&thread[i], NULL, concurRequest, &settings);
    }
    for (int i = 0; i < settings.threadNum; i++) {
        pthread_join(thread[i], NULL);
    }
    free(thread);
    thread = NULL;
}

void *concurRequest(void *args) {
    RequestSettings *settings = (RequestSettings *) args;
    while(1){
        iRequest2(settings->server);
        pthread_barrier_wait(&settings->concurBarrier); // 屏障可以重复利用
        sleep(settings->sleepTime);
    }
}

void fifo(RequestSettings settings) {
    pthread_t *fifoThread = malloc(settings.threadNum * sizeof(pthread_t));
    pthread_mutex_init(&settings.fifoMutex, NULL); // 初始化互斥锁
    for (int i = 0; i < settings.threadNum; i++) {
        pthread_create(&fifoThread[i], NULL, fifoRequest, &settings);
    }
    for (int i = 0; i < settings.threadNum; i++) {
        pthread_join(fifoThread[i], NULL);
    }
    free(fifoThread);
    fifoThread = NULL;
}

void *fifoRequest(void *args) {
    RequestSettings *settings = (RequestSettings *) args;
    while(1){
        pthread_mutex_lock(&settings->fifoMutex);
        iRequest2(settings->server);
        sleep(settings->sleepTime);
        pthread_mutex_unlock(&settings->fifoMutex);
    }
}