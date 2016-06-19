//
// Created by Tom Wang on 16/6/15.
//

#ifndef SIMPLETFTP_CLIENTSOCKET_H
#define SIMPLETFTP_CLIENTSOCKET_H
#include "Socket.h"

void SendC (char *pFilename, struct sockaddr_in client, char *pMode, int sock);
void GetC (char *pFilename, struct sockaddr_in client, char *pMode, int sock);


#endif //SIMPLETFTP_CLIENTSOCKET_H
