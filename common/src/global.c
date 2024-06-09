#include "global.h"
#include "logger.h"
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>

char *DEVICE_UID = NULL;
Logger logger = {NULL, 0, NULL};
volatile int running = 1;

/**
 * 初始化设备
 * @param customUid 可选自定义UID，为NULL则随机生成
 */
void initDevice(const char *customUid){
    // 配置ctrl+c信号处理
    signal(SIGINT, exit_interrupt);
    // 1. 初始化日志
    logger = initLogger();
    // 2. 初始化设备UID
    char *uid = (char *) malloc(UID_LEN+1);
    // 若自定义UID
    if(customUid != NULL) {
        if(strlen(customUid) != UID_LEN) {
            logger.info("Invalid custom UID, an random one would be in use", LOG_WARNING);
        } else {
            DEVICE_UID = strdup(customUid);
            free(uid);
            return;
        }
    }
    // 随机生成当前设备ID
    srand(time(NULL));
    memset(uid, 0, UID_LEN+1);
    for (int i = 0; i < UID_LEN; ++i) {
        uid[i] = (char)('A' + rand() % ('Z' - 'A' + 1));
    }
    DEVICE_UID = uid;
    logger.info("Device UID generated, UID: %s", LOG_INFO, DEVICE_UID);
}

/**
 * 销毁设备，释放资源
 */
void destroyDevice() {
    if(DEVICE_UID != NULL) {
        free(DEVICE_UID);
        DEVICE_UID = NULL;
    }
    if(logger.info != NULL){
        destroyLogger();
    }
}

/**
 * 异常退出程序，释放资源
 * @param msg 退出信息
 */
void exit_error(const char *msg) {
    logger.info(msg, LOG_ERROR);
    running = 0;
    destroyDevice();
    exit(EXIT_FAILURE);
}

void exit_normal() {
    destroyDevice();
    running = 0;
    exit(0);
}

void exit_interrupt() {
    logger.info("Interrupt signal(CTRL+C) received, exiting...", LOG_WARNING);
    raise(SIGUSR1);
    running = 0;
    destroyDevice();
    exit(EXIT_SUCCESS);
}