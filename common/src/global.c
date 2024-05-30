#include <unistd.h>
#include <stdlib.h>
#include "logger.h"
#include "global.h"


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