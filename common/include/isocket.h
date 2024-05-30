#ifndef SERVER_ISOCKET_H
#define SERVER_ISOCKET_H

#define SERVER_PORT 8080
#define GET_MAX_SIZE 1024
#define POST_MAX_SIZE 2048
#define ACCEPT_MAX_SIZE 2048
#define RESPONSE_MAX_SIZE 2048
#include <netdb.h>

extern const char GET_REQ[GET_MAX_SIZE];
extern const char POST_REQ[POST_MAX_SIZE];

typedef struct request{
    char *(*get)(struct hostent *server,  const char *getReqFormat);
    char *(*post)(struct hostent *server, const char *msg);
}Req;

extern Req req;
void iListen(void (*requestHandler)(void *arg));
void getRequest(int sockfd, char *buffer);
char *getReqHeaderToken(const char *request);
char *getReq2JSON(const char *getParams);
char *requestResolver(char *request);
void response(int sockfd, const char *buffer);

#endif //SERVER_ISOCKET_H
