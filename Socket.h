//
// Created by Tom Wang on 16/6/14.
//

#ifndef SIMPLETFTP_SOCKET_H
#define SIMPLETFTP_SOCKET_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string>
#include <arpa/inet.h>



class Socket
{
public:
    Socket();
    virtual ~Socket();

    // 服务端初始化程序
    bool create();
    bool bind (int port);

    // 客户端初始化程序

    bool is_valid() const { return m_sock != -1; }


//private:

    int m_sock;
    sockaddr_in m_addr;
};

static char path[64] = "/tmp/";
static int port = 169;
static unsigned short int ackfreq = 1;
static int datasize = 512;

int SendErrPacket(int code,const char* message, char buffer[]);

#endif //SIMPLETFTP_SOCKET_H
