#ifndef GLOBAL_H
#define GLOBAL_H
#include "logger.h"

extern Logger logger;
extern volatile int running;
extern void exit_error(const char *msg);

#endif