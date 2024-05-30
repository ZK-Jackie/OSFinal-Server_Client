#ifndef SERVER_PARAM_H
#define SERVER_PARAM_H
typedef struct {
    int threadNum;
    int workload;
    int sleepTime;
} ClientParams;

ClientParams parseArgs(int argc, char *argv[]);

#endif //SERVER_PARAM_H
