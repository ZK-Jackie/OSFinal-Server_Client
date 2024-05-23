#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "thread/thread.h"
#include "socket/isocket.h"
#include "utils/param.h"
#include "utils/log.h"

void handleRequest(void *args);

void run(ServerParams params){
    // 1. 线程池 & 缓冲池初始化
    pool_init(params);
    pthread_t poolThread;
    pthread_create(&poolThread, NULL, pool_maintain, NULL);
    // 2. 监听socket初始化
    iListen(&handleRequest);
}

void handleRequest(void *args){
    int newsockfd = *(int *)args;
    // 读写数据
    char buffer[256];
    bzero(buffer, 256);
    int n;
    n = (int)read(newsockfd, buffer, 255);
    if (n < 0) {
        perror("ERROR reading from socket");
        exit(EXIT_FAILURE);
    }
    printf("Here is the message: %s\n", buffer);
    n = (int)write(newsockfd, "I got your message", 18);
    if (n < 0) {
        perror("ERROR writing to socket");
        exit(EXIT_FAILURE);
    }
    // 7. 关闭socket描述符
    close(newsockfd);
}