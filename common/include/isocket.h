#ifndef SERVER_ISOCKET_H
#define SERVER_ISOCKET_H

#define SERVER_PORT 8080
#define GET_MAX_SIZE 1024
#define POST_MAX_SIZE 2048
#define ACCEPT_MAX_SIZE 2048
#define RESPONSE_MAX_SIZE 2048

#include <netdb.h>

extern const char GET_REQ[];
extern const char POST_REQ[];
extern const int GET_CONTENT_LEN;
extern const int POST_CONTENT_LEN;
extern const int RESPONSE_CONTENT_LEN;

typedef struct request{
    char *(*get)(struct hostent *server,  const char *getReqFormat);
    char *(*post)(struct hostent *server, const char *msg);
}Req;

extern Req req;
void iListen(void *(*reqHandler)(void *arg));
void getRequest(int sockfd, char *buffer);
char *getHeader(const char *msg, const char *header);
char *getLoad(const char *msg);
char *getReq2JSON(const char *getParams);
char *requestResolver(const char *request);
void response(int sockfd, const char *buffer);

#endif //SERVER_ISOCKET_H
