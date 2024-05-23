#ifndef SERVER_PARAM_H
#define SERVER_PARAM_H
typedef struct {
    int THREAD_NUM;
    int WORKLOAD;
    void (*callback)(void *);
} ClientParams;

ClientParams parse_args(int argc, char *argv[]);

#endif //SERVER_PARAM_H
