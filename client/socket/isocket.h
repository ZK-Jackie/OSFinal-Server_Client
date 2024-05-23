//
// Created by jackie on 5/22/24.
//

#ifndef SERVER_ISOCKET_H
#define SERVER_ISOCKET_H

#define SERVER_PORT 8080

#include <netdb.h>

void iRequest2(struct hostent *server);
void iRequest();


#endif //SERVER_ISOCKET_H
