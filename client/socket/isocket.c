#include "isocket.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>



void iRequest() {
    // 1. 创建socket描述符
    int sockfd;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(EXIT_FAILURE);
    }
    // 2. 连接服务器
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo("localhost", NULL, &hints, &res) != 0) {
        perror("getaddrinfo");
        exit(EXIT_FAILURE);
    }
    // 3. 设置服务器地址
    struct sockaddr_in *serv_addr = (struct sockaddr_in *) res->ai_addr;
    serv_addr->sin_port = htons(SERVER_PORT);
    // 4. 连接服务器
    if (connect(sockfd, (struct sockaddr *) serv_addr, sizeof(*serv_addr)) < 0) {
        perror("ERROR connecting");
        exit(EXIT_FAILURE);
    }
    freeaddrinfo(res);
    // 5. 发送请求
    char buffer[256];
    bzero(buffer, 256);
    strcpy(buffer, "GET / HTTP/1.1\nHost: www.baidu.com");
    int n = (int) write(sockfd, buffer, strlen(buffer));
    if (n < 0) {
        perror("ERROR writing to socket");
        exit(EXIT_FAILURE);
    }
    // 6. 接收响应
    bzero(buffer, 256);
    n = (int) read(sockfd, buffer, 255);
    if (n < 0) {
        perror("ERROR reading from socket");
        exit(EXIT_FAILURE);
    }
    printf("%s\n", buffer);
    // 7. 关闭socket
    close(sockfd);
}

void iRequest2(struct hostent *server){
    // 1. 创建socket描述符
    int sockfd;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }
    // 2. 连接服务器
    if (server == NULL) {
        perror("ERROR, no such host\n");
        exit(0);
    }
    // 3. 设置服务器地址
    struct sockaddr_in serv_addr;
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVER_PORT);
    bcopy((char *) server->h_addr, (char *) &serv_addr.sin_addr.s_addr, server->h_length);
    // 4. 连接服务器
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR connecting");
        exit(1);
    }
    // 5. 发送请求
    char buffer[256];
    bzero(buffer, 256);
    strcpy(buffer, "GET / HTTP/1.1\nHost: www.baidu.com");
    int n = (int)write(sockfd, buffer, strlen(buffer));
    if (n < 0) {
        perror("ERROR writing to socket");
        exit(1);
    }
    // 6. 接收响应
    bzero(buffer, 256);
    n = (int)read(sockfd, buffer, 255);
    if (n < 0) {
        perror("ERROR reading from socket");
        exit(1);
    }
    printf("%s\n", buffer);
    // 7. 关闭socket
    close(sockfd);
}