#include "thread.h"
#include "isocket.h"
#include "param.h"
#include "logger.h"
#include "global.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define SERVER_APP_VERSION "1.0.0"

const char RESULT_SUCCESS[] = "{\"code\": 200, \"msg\": \"success\", \"data\": %s}";
const char RESULT_BAD_REQUEST[] = "{\"code\": 400, \"msg\": \"bad request\"}";
const char RESULT_NOT_FOUND[] = "{\"code\": 404, \"msg\": \"not found\"}";
const char RESULT_SERVER_ERROR[] = "{\"code\": 500, \"msg\": \"server error\"}";


/**
 * 请求处理函数
 * @param args 参数
 * @return void* 返回值
 */
void *handleRequest(void *args){
    int newsockfd = *(int *)args;
    // 1. 读数据
    char buffer[ACCEPT_MAX_SIZE];
    getRequest(newsockfd, buffer);
    // 2. 解析数据
    char *deviceToken = getHeader(buffer, "Token");
    char *request = requestResolver(buffer);
    // 3. 业务处理
    logger.info("From: %s, request: %s", LOG_INFO, deviceToken, request);
    // 4. 响应数据，把请求数据放入data中
    char *resp = (char *)malloc(strlen(request) + strlen(RESULT_SUCCESS) + 1);
    memset(resp, 0, strlen(request) + strlen(RESULT_SUCCESS) + 1);
    sprintf(resp, RESULT_SUCCESS, request);
    response(newsockfd, resp);
    // 5. 释放资源
    free(resp);
    free(deviceToken);
    free(request);
    close(newsockfd);
    return NULL;
}


/**
 * 程序运行入口，解析参数，初始化设备、线程池、监听端口
 * @param argc 用户输入参数个数
 * @param argv 用户输入参数列表
 */
void run(int argc, char *argv[]){
    // 1.初始化设备：随机生成当前设备ID、初始化日志线程
    ServerParams params = parseArgs(argc, argv);
    initDevice(NULL);
    // 2. 线程池 & 缓冲池初始化
    initPools(params.THREAD_NUM, params.BUFFER_SIZE, NULL);
    // 3. 监听端口，设定请求处理函数
    iListen(&handleRequest);
    // 4. 程序结束
    destroyDevice();
}