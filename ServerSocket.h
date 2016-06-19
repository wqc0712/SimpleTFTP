//
// Created by Tom Wang on 16/6/14.
//

#ifndef SIMPLETFTP_SERVERSOCKET_H
#define SIMPLETFTP_SERVERSOCKET_H
#include "Socket.h"
#include "ExceptionSock.h"
#include "DefineType.h"

class ServerSocket : private Socket
{
public:

    ServerSocket(int port);
    ServerSocket(const ServerSocket & C);
    virtual ~ServerSocket();

    int get_sock() { return m_sock;}

};

void Send (char *, struct sockaddr_in, char *, int);
void Get (char *, struct sockaddr_in, char *, int);

bool TestInformation(char* mode, char* filename, int sock,struct sockaddr_in client);

#endif //SIMPLETFTP_SERVERSOCKET_H
