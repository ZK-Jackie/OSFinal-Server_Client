#include "global.h"
#include "isocket.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

char *get(struct hostent *server, const char *param_str);
char *post(struct hostent *server, const char *msg);

const char POST_REQ[POST_MAX_SIZE] = "POST %s HTTP/1.1\r\n"
                                     "Content-Type: application/json;charset=utf-8\r\n"
                                     "Content-Length: %d\r\n"
                                     "Accept: */*\r\n"
                                     "\r\n"
                                     "%s";
const char GET_REQ[GET_MAX_SIZE] = "GET %s HTTP/1.1\r\n"
                                    "Accept: */*\r\n"
                                    "\r\n";

Req req = {
    .get = get,
    .post = post
};


/**
 * 请求服务器
 * @note 采用getaddrinfo获取服务器地址，始终是线程安全的，但占用资源较多
 * */
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

/**
 * 请求服务器
 * @note 采用主调函数提供的gethostbyname的服务器信息进行链接，占用资源较少
 * */
char * iRequest2(struct hostent *server, const char *msg) {
    // 0. 检查参数
    if (server == NULL || msg == NULL) {
        exit_error("ERROR, invalid parameter");
    }
    // 1. 创建socket描述符
    int sockfd;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        exit_error("ERROR opening socket");
    }
    // 2. 连接服务器
    if (server == NULL || server->h_addr == NULL) {
        exit_error("ERROR, no such host");
    }
    // 3. 设置服务器地址
    struct sockaddr_in serv_addr;
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVER_PORT);
    bcopy((char *) server->h_addr, (char *) &serv_addr.sin_addr.s_addr, server->h_length);
    // 4. 连接服务器
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        exit_error("ERROR connecting");
    }
    // 5. 发送请求
    char buffer[256];
    bzero(buffer, 256);
    strcpy(buffer, msg);
    int n = (int)write(sockfd, buffer, strlen(buffer));
    if (n < 0) {
        exit_error("ERROR writing to socket");
    }
    // 6. 接收响应
    bzero(buffer, 256);
    n = (int)read(sockfd, buffer, 255);
    if (n < 0) {
        exit_error("ERROR reading from socket");
    }
    // 7. 关闭socket
    close(sockfd);
    return strdup(buffer);
}

char *get(struct hostent *server, const char *param_str) {
    // 组装GET请求报文
    char host_buffer[GET_MAX_SIZE];
    strcpy(host_buffer, server->h_name);
    strcat(host_buffer, "?");
    strcat(host_buffer, param_str);
    char buffer[GET_MAX_SIZE];
    snprintf(buffer, GET_MAX_SIZE, GET_REQ, host_buffer);
    return iRequest2(server, buffer);
}


char *post(struct hostent *server, const char *msg) {
    // 组装POST请求报文
    char buffer[POST_MAX_SIZE];
    snprintf(buffer, POST_MAX_SIZE, POST_REQ, server->h_name, (int)strlen(msg), msg);
    return iRequest2(server, buffer);
}