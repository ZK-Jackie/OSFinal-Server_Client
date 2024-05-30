#ifndef GLOBAL_H
#define GLOBAL_H
#include "logger.h"
#define UID_LEN 5

extern char *DEVICE_UID;
extern Logger logger;
extern volatile int running;
void initDevice(const char *customUid);
extern void exit_error(const char *msg);

#endif