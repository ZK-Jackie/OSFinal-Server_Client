#ifndef SERVER_PARAM_H
#define SERVER_PARAM_H
typedef struct {
    int threadNum;
    int workload;
    int sleepTime;
} ClientParams;

ClientParams parse_args(int argc, char *argv[]);

#endif //SERVER_PARAM_H
