#include "global.h"
#include "thread.h"
#include "isocket.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

const char POST_REQ[] = "POST / HTTP/1.1\r\n"
                         "Host: %s\r\n"
                         "Token: %s\r\n"
                         "Content-Type: application/json;charset=utf-8\r\n"
                         "Content-Length: %d\r\n"
                         "Accept: */*\r\n"
                         "\r\n"
                         "%s";
const int POST_REQ_LEN = strlen(POST_REQ);
const int POST_CONTENT_LEN = POST_MAX_SIZE - POST_REQ_LEN;

const char GET_REQ[] = "GET / HTTP/1.1\r\n"
                       "Host: %s\r\n"
                       "Token: %s\r\n"
                       "Accept: */*\r\n"
                       "\r\n";
const int GET_REQ_LEN = strlen(POST_REQ);
const int GET_CONTENT_LEN = GET_MAX_SIZE - GET_REQ_LEN;

const char RESPONSE[] = "HTTP/1.1 200 OK\r\n"
                       "Content-Type: application/json;charset=utf-8\r\n"
                       "Content-Length: %d\r\n"
                       "Connection: close\r\n"
                       "Server: SWS/1.0 (Unix) (Ubuntu/Linux)\r\n"
                       "\r\n"
                       "%s";
const int RESPONSE_LEN = strlen(RESPONSE);
const int RESPONSE_CONTENT_LEN = RESPONSE_MAX_SIZE - RESPONSE_LEN;


char *get(struct hostent *server, const char *getReqFormat);
char *post(struct hostent *server, const char *msg);
Req req = {
        .get = get,
        .post = post
};

/**
 * 服务器监听
 * @param reqHandler 请求处理函数
 * */
void iListen(void *(*reqHandler)(void *arg)){
    // 1. 创建socket描述符，domain -- 协议（当前为IPv4） type -- 类型（当前为TCP） protocol -- 协议（0表示默认）
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {   // 创建失败会返回 -1
        exit_error("ERROR opening socket");
    }
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0) {
        exit_error("setsockopt(SO_REUSEADDR) failed");
    }
    // 2. 确定服务器信息
    struct sockaddr_in serv_addr;
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET; // 协议
    serv_addr.sin_addr.s_addr = INADDR_ANY; // 本地地址
    serv_addr.sin_port = htons(SERVER_PORT); // 端口号
    // 3. 绑定socket描述符和服务器信息
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        exit_error("ERROR on binding");
    }
    // 4. 监听socket描述符
    listen(sockfd, 5);
    // 5. 接收客户端请求
    struct sockaddr_in cli_addr;
    socklen_t client = sizeof(cli_addr);
    logger.info("Server is listening on port %d", LOG_INFO, SERVER_PORT);
    while(running){
        // 获取接收到的新的socket描述符
        int newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &client);
        if (newsockfd < 0) {
            exit_error("ERROR on accept");
        }
        // 6. 打包成Task交由线程池处理
        Task *task = malloc(sizeof(Task));
        task->func = reqHandler;
        task->arg = malloc(sizeof(int));
        *(int *)(task->arg) = newsockfd;
        addTask(task);
    }
    close(sockfd);
}


/**
 * 请求服务器
 * @note 采用getaddrinfo获取服务器地址，始终是线程安全的，但占用资源较多
 * */
void iRequest() {
    // 1. 创建socket描述符
    int sockfd;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        exit_error("ERROR opening socket");
    }
    // 2. 连接服务器
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo("localhost", NULL, &hints, &res) != 0) {
        exit_error("ERROR, no such host");
    }
    // 3. 设置服务器地址
    struct sockaddr_in *serv_addr = (struct sockaddr_in *) res->ai_addr;
    serv_addr->sin_port = htons(SERVER_PORT);
    // 4. 连接服务器
    if (connect(sockfd, (struct sockaddr *) serv_addr, sizeof(*serv_addr)) < 0) {
        exit_error("ERROR connecting");
    }
    freeaddrinfo(res);
    // 5. 发送请求
    char buffer[POST_MAX_SIZE];
    bzero(buffer, POST_MAX_SIZE);
    strcpy(buffer, "GET / HTTP/1.1\nHost: www.baidu.com");
    int n = (int) write(sockfd, buffer, strlen(buffer));
    if (n < 0) {
        exit_error("ERROR writing to socket");
    }
    // 6. 接收响应
    bzero(buffer, RESPONSE_MAX_SIZE);
    n = (int) read(sockfd, buffer, RESPONSE_MAX_SIZE);
    if (n < 0) {
        exit_error("ERROR reading from socket");
    }
    printf("%s\n", buffer);
    // 7. 关闭socket
    close(sockfd);
}


/**
 * 请求服务器
 * @note 采用主调函数提供的gethostbyname的服务器信息进行链接，占用资源较少
 * @param msg 请求数据
 * @param server 服务器信息
 * */
char * iRequest2(struct hostent *server, const char *msg) {
    // 0. 检查参数
    if (server == NULL || msg == NULL) {
        exit_error("ERROR, invalid parameter");
    }
    // 1. 创建socket描述符
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
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
    int status = (int)write(sockfd, msg, strlen(msg));
    if (status < 0) {
        exit_error("ERROR writing to socket");
    }
    // 6. 接收响应
    char res[RESPONSE_MAX_SIZE];
    bzero(res, RESPONSE_MAX_SIZE);
    status = (int)read(sockfd, res, RESPONSE_MAX_SIZE);
    if (status < 0) {
        exit_error("ERROR reading from socket");
    }
    // 7. 关闭socket
    close(sockfd);
    return strdup(res);
}


/**
 * GET请求
 * @param server 服务器信息
 * @param getReqFormat 请求参数
 * @return 响应数据
 * */
char *get(struct hostent *server, const char *getReqFormat) {
    if (server == NULL || getReqFormat == NULL) {
        logger.info("ERROR, invalid GET request parameter", LOG_ERROR);
        return NULL;
    }else if (strlen(getReqFormat) > GET_CONTENT_LEN) {
        logger.info("ERROR, GET request too long", LOG_ERROR);
        return NULL;
    }
    // 组装GET请求报文
    char host_buffer[GET_CONTENT_LEN];
    strcpy(host_buffer, server->h_name);
    strcat(host_buffer, "?");
    strcat(host_buffer, getReqFormat);
    char buffer[GET_MAX_SIZE];
    snprintf(buffer, GET_MAX_SIZE, GET_REQ, host_buffer, DEVICE_UID);
    return iRequest2(server, buffer);
}


/**
 * POST请求
 * @param server 服务器信息
 * @param msg 请求数据
 * @return 响应数据
 * */
char *post(struct hostent *server, const char *msg) {
    if (server == NULL || msg == NULL) {
        logger.info("ERROR, invalid POST request parameter", LOG_ERROR);
        return NULL;
    }else if (strlen(msg) > POST_CONTENT_LEN) {
        logger.info("ERROR, POST request too long", LOG_ERROR);
        return NULL;
    }
    // 组装POST请求报文
    char buffer[POST_MAX_SIZE];
    snprintf(buffer, POST_MAX_SIZE, POST_REQ, server->h_name, DEVICE_UID, (int)strlen(msg), msg);
    return iRequest2(server, buffer);
}


/**
 * 从完整请求报文中获取请求头的值
 * @param msg 请求报文
 * @param header 请求头名，当前版本需提供的请求头名与目标请求头名完全一致
 * @return 返回值
 * */
char *getHeader(const char *msg, const char *header) {
    char *msgBak = strdup(msg);
    // 1.定义header头部，在msg中查找子串起始位置
    char *valStart = strstr(msgBak, header);
    if (valStart == NULL) {
        logger.info("Header '%s' not found", LOG_ERROR, header);
        return NULL;
    }
    // 2.找到header值的起始位置，即冒号、空格后
    valStart += strlen(header) + 2;
    // 3.找到header值的结束位置
    char* valEnd = strstr(valStart, "\r\n");
    if (valEnd == NULL) {
        logger.info("Malformed msg, no end of line found when reading header '%s'", LOG_ERROR, header);
        return NULL;
    }
    // 4.计算Token Value的长度
    size_t tokenLength = valEnd - valStart;
    // 5.复制Token Value到新的字符串
    char* token = malloc(tokenLength + 1);
    strncpy(token, valStart, tokenLength);
    token[tokenLength] = '\0'; // Null-terminate the string
    free(msgBak);
    return token;
}


/**
 * 从POST请求中获取载荷
 * @param msg 完整请求报文
 * @return 载荷
 * */
char *getLoad(const char *msg) {
    char *msgBak = strdup(msg);
    // 1.找到空行
    char *loadStart = strstr(msgBak, "\r\n\r\n");
    if (loadStart == NULL) {
        logger.info("Malformed msg, no end of header found", LOG_ERROR);
        return NULL;
    }
    // 2.计算载荷长度
    size_t loadLength = strlen(loadStart + 4);
    // 3.复制载荷到新的字符串
    char *load = malloc(loadLength + 1);
    strcpy(load, loadStart + 4);
    free(msgBak);
    return load;
}


/**
 * 将GET请求参数转换为JSON
 * @param request GET请求参数字符串
 * @return JSON String，GET请求参数字符串转换后的JSONStr
 * */
char *getReq2JSON(const char *getParams){
    char *getParamsBak = strdup(getParams);
    char *jsonBuffer = (char *) malloc(GET_MAX_SIZE);
    bzero(jsonBuffer, GET_MAX_SIZE);
    strcat(jsonBuffer, "{");

    char *tmp = strtok(getParamsBak, "&");
    while (tmp != NULL) {
        // 1. 找到等号位置
        int eqPos = (int)(strchr(tmp, '=') - tmp);
        // 2. 确定key和value长度
        char *key = (char *) malloc(eqPos + 1);
        char *value = (char *) malloc(strlen(tmp) - eqPos);
        bzero(key, eqPos + 1);
        bzero(value, strlen(tmp) - eqPos);
        // 3. 复制key和value
        strncpy(key, tmp, eqPos);
        strncpy(value, tmp + eqPos + 1, strlen(tmp) - eqPos);
        // 4. 拼接json
        strcat(jsonBuffer, "\"");
        strcat(jsonBuffer, key);
        strcat(jsonBuffer, "\":\"");
        strcat(jsonBuffer, value);
        strcat(jsonBuffer, "\",");
        // 5. 释放内存
        free(key);
        free(value);
        // 6. 下一个参数
        tmp = strtok(NULL, "&");
    }
    jsonBuffer[strlen(jsonBuffer) - 1] = '}';
    free(getParamsBak);
    return jsonBuffer;
}


/**
 * 从完整请求报文中获取负载
 * @param request 完整请求报文
 * @return JSON String，负载字符串
 * */
char *requestResolver(const char *request) {
    char *reqBak = strdup(request);
    // 1. 查看请求类型
    char *method = strtok(reqBak, " ");
    if (method == NULL) {
        free(reqBak);
        logger.info("ERROR, invalid request, reading: %s", LOG_ERROR, request);
        return NULL;
    }
    // 2. 解析请求
    if (strcmp(method, "GET") == 0) {
        char *host = getHeader(request, "Host");
        if (host == NULL) {
            free(reqBak);
            logger.info("ERROR, invalid request, reading: %s", LOG_ERROR, request);
            return NULL;
        }
        // 找问号
        strtok(host, "?");
        char *ret = getReq2JSON(strtok(NULL, "?"));
        free(reqBak);
        return ret;
    } else if (strcmp(method, "POST") == 0) {
        char *content = getLoad(request);
        if (content == NULL) {
            free(reqBak);
            logger.info("ERROR, invalid request, reading: %s", LOG_ERROR, request);
            return NULL;
        }
        free(reqBak);
        return content;
    }else{
        free(reqBak);
        logger.info("Unrecognized request", LOG_ERROR);
        return NULL;
    }
}


/**
 * 从请求中获取请求数据
 * @param request 请求报文
 * @param buffer 从客户端的请求中获取报文完整
 * */
void getRequest(int sockfd, char *buffer){
    bzero(buffer, ACCEPT_MAX_SIZE);
    int status = (int)read(sockfd, buffer, ACCEPT_MAX_SIZE - 1);
    if (status < 0) {
        exit_error("ERROR reading from socket");
    }
}


/**
 * 响应请求
 * @param sockfd socket描述符
 * @param buffer 服务端要发送回客户端的完整报文
 * */
void response(int sockfd, const char *buffer){
    char temp[RESPONSE_MAX_SIZE];
    snprintf(temp, RESPONSE_MAX_SIZE, RESPONSE, (int)strlen(buffer), buffer);
    int status = (int)write(sockfd, temp, strlen(temp));
    if (status < 0) {
        exit_error("ERROR writing to socket");
    }
}