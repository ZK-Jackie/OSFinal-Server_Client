#ifndef SERVER_PARAM_H
#define SERVER_PARAM_H
typedef struct {
    int THREAD_NUM;
    int BUFFER_SIZE;
    void (*callback)(void *);
} ServerParams;

ServerParams parseArgs(int argc, char *argv[]);

#endif //SERVER_PARAM_H
