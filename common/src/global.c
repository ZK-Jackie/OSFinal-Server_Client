#include "global.h"
#include "logger.h"
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

char *DEVICE_UID = NULL;
Logger logger = {NULL, NULL};
volatile int running = 1;
volatile int error_exit = 0;

void exit_error(const char *msg) {
    running = 0;
    logger.info(msg, LOG_ERROR);
    time_t exit_time = time(NULL) + 3;
    while (time(NULL) < exit_time) {
        sleep(1);
    }
    exit(EXIT_FAILURE);
}

void initDevice(const char *customUid){
    char *uid = (char *) malloc(UID_LEN+1);
    if(customUid != NULL) {
        if(strlen(customUid) != UID_LEN) {
            logger.info("Invalid custom UID, an random one would be in use", LOG_WARNING);
        } else {
            DEVICE_UID = strdup(customUid);
            free(uid);
            return;
        }
    }
    // 0.随机生成当前设备ID
    srand(time(NULL));
    memset(uid, 0, UID_LEN+1);
    for (int i = 0; i < UID_LEN; ++i) {
        uid[i] = (char)('A' + rand() % ('Z' - 'A' + 1));
    }
    DEVICE_UID = uid;
}