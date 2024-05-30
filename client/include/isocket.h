#ifndef SERVER_ISOCKET_H
#define SERVER_ISOCKET_H

#define SERVER_PORT 8080
#define GET_MAX_SIZE 1024
#define POST_MAX_SIZE 2048

#include <netdb.h>

extern const char GET_REQ[GET_MAX_SIZE];
extern const char POST_REQ[POST_MAX_SIZE];

typedef struct request{
    char *(*get)(struct hostent *server,  const char *param_str);
    char *(*post)(struct hostent *server, const char *msg);
}Req;

extern Req req;


#endif //SERVER_ISOCKET_H
