#include "global.h"
#include "logger.h"
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

char *DEVICE_UID = NULL;
Logger logger = {NULL, NULL};
volatile int running = 1;
volatile int error_exit = 0;

/**
 * 初始化设备
 * @param customUid 可选自定义UID，为NULL则随机生成
 */
void initDevice(const char *customUid){
    logger = initLogger();
    char *uid = (char *) malloc(UID_LEN+1);
    // 1.若自定义UID
    if(customUid != NULL) {
        if(strlen(customUid) != UID_LEN) {
            logger.info("Invalid custom UID, an random one would be in use", LOG_WARNING);
        } else {
            DEVICE_UID = strdup(customUid);
            free(uid);
            return;
        }
    }
    // 2.随机生成当前设备ID
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
    running = 0;
    logger.info(msg, LOG_ERROR);
    destroyDevice();
    exit(EXIT_FAILURE);
}