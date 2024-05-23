#include "../thread/thread.h"
#include "isocket.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

void iListen(void (*requestHandler)(void *arg)){
    // 1. 创建socket描述符，domain -- 协议（当前为IPv4） type -- 类型（当前为TCP）
    int sockfd;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {   // 创建失败会返回 -1
        perror("ERROR opening socket");
        exit(EXIT_FAILURE);
    }
    // 2. 确定服务器信息
    struct sockaddr_in serv_addr;
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET; // 协议
    serv_addr.sin_addr.s_addr = INADDR_ANY; // 本地地址
    serv_addr.sin_port = htons(SERVER_PORT); // 端口号
    // 3. 绑定socket描述符和服务器信息
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR on binding");
        exit(EXIT_FAILURE);
    }
    // 4. 监听socket描述符
    listen(sockfd, 5);
    // 5. 接收客户端请求
    struct sockaddr_in cli_addr;
    socklen_t clilen = sizeof(cli_addr);
    while(1){
        int newsockfd; // 获取接收到的新的socket描述符
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) {
            perror("ERROR on accept");
            exit(EXIT_FAILURE);
        }
        // 6. 打包成Task交由线程池处理
        Task *task = malloc(sizeof(Task));
        task->func = requestHandler;
        task->arg = malloc(sizeof(int));
        *(int *)(task->arg) = newsockfd;
        add_task(task);
    }
    close(sockfd);
}
