#include "thread.h"
#include "isocket.h"
#include "param.h"
#include "logger.h"
#include "global.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
char *DEVICE_UID = NULL;

void handleRequest(void *args);

void run(int argc, char *argv[]){
    // 0.随机生成当前设备ID
    srand(time(NULL));
    char *uid = (char *) malloc(UID_LEN);
    for (int i = 0; i < UID_LEN; ++i) {
        uid[i] = (char)('A' + rand() % ('Z' - 'A' + 1));
    }
    DEVICE_UID = uid;
    // 1. 初始化日志线程
    logger = initLogger();
    ServerParams params = parseArgs(argc, argv);
    // 2. 线程池 & 缓冲池初始化
    initPools(params);
    // 3. 监听socket初始化
    iListen(&handleRequest);
    // 4. 程序结束
    DEVICE_UID = NULL;
    free(uid);
}

void handleRequest(void *args){
    int newsockfd = *(int *)args;
    // 1. 读数据
    char buffer[ACCEPT_MAX_SIZE];
    getRequest(newsockfd, buffer);
    // 2. 解析数据
    char *deviceToken = getReqHeaderToken(buffer);
    char *request = requestResolver(buffer);
    // 3. 业务处理
    logger.info("From: %s, request: %s", LOG_INFO, deviceToken, request);
    // 4. 响应数据，把请求数据放入data中
    char *resp = (char *)malloc(strlen(request) + 100);
    sprintf(resp, "{\"code\": 200, \"msg\": \"success\", \"jsonObj\": %s}", request);
    response(newsockfd, resp);
    // 5. 释放资源
    free(resp);
    free(deviceToken);
    free(request);
    close(newsockfd);
}