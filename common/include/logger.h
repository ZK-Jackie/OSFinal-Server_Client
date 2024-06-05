#ifndef SERVER_LOG_H
#define SERVER_LOG_H

#include <pthread.h>
#include <semaphore.h>

#define LOG_INFO 0
#define LOG_WARNING 1
#define LOG_ERROR 2
#define LOG_DEBUG 3
#define LOG_CRITICAL 4
#define LOG_DEPRECATED 5

extern const char *LOG_LEVEL_STR[];


typedef struct{
    pthread_t *thread;
    void (*info)(const char *msg, int level, ...);
}Logger;


extern Logger initLogger();
void destroyLogger();


#endif //SERVER_LOG_H
