#ifndef SERVER_ISOCKET_H
#define SERVER_ISOCKET_H

#define SERVER_PORT 8080

void iListen(void (*requestHandler)(void *arg));

#endif //SERVER_ISOCKET_H
