#ifndef GLOBAL_H
#define GLOBAL_H
#include "logger.h"
#define UID_LEN 5

extern char *DEVICE_UID;
extern void initDevice(const char *customUid);

extern Logger logger;
extern volatile int running;

extern void exit_error(const char *msg);
extern void destroyDevice();

#endif