#include "utils/param.h"
#include "socket/isocket.h"
#include <stdlib.h>
#include <pthread.h>

void concur(int threadNum);
void *concurRequest();

void fifo(int threadNum);

pthread_barrier_t barrier;

int sockfd = 0;
struct hostent *server;


void run(ClientParams params) {
    server = gethostbyname("localhost");
    if (!params.WORKLOAD) {
        concur(params.THREAD_NUM);
    } else {
        fifo(params.THREAD_NUM);
    }
}

void concur(int threadNum) {
    pthread_t *thread = malloc(threadNum * sizeof(pthread_t));
    pthread_barrier_init(&barrier, NULL, threadNum); // 初始化屏障
    for (int i = 0; i < threadNum; i++) {
        pthread_create(&thread[i], NULL, concurRequest, NULL);
    }
    for (int i = 0; i < threadNum; i++) {
        pthread_join(thread[i], NULL);
    }
    free(thread);
    thread = NULL;
}

void *concurRequest() {
    // Request
    while(1){
//        iRequest();
        iRequest2(server);
        pthread_barrier_wait(&barrier);
    }
    pthread_exit(NULL);
}

void fifo(int threadNum) {

}