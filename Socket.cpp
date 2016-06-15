//
// Created by Tom Wang on 16/6/14.
//

#include "Socket.h"
#include "DefineType.h"


Socket::Socket()
{
    m_sock = -1;
    memset ( &m_addr, 0, sizeof ( m_addr ) );
    //sockaddr_in 初始化

}

Socket::~Socket()
{
    if ( is_valid() )
        ::close ( m_sock ); //如果还保持一个连接,则需要释放连接.
}

bool Socket::create()
{
    m_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); //使用Linux内置的socket函数开始创建socket
    // AF_INET代表IPv4连接,具体含义参考socket.h
    // SOCK_STREAM表示为面向连接的数据包,用于TCP连接,UDP应使用SOCK_DGRAM
    // IPPROTO_TCP代表TCP协议,具体应查看in.h

    return is_valid(); //创建失败返回错误

}

bool Socket::bind ( int port )
{
    //进行端口binding操作.
    if ( ! is_valid() )
    {
        return false;
    }



    m_addr.sin_family = AF_INET;
    m_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    m_addr.sin_port = htons ( port );

    int bind_return = ::bind ( m_sock,
                               ( struct sockaddr * ) &m_addr,
                               sizeof ( m_addr ) );


    return bind_return != -1;

}


int SendErrPacket(int code,const char* message, char buffer[]){
    int len;
    memset (buffer, 0, sizeof(*buffer));
    len = sprintf (buffer, "%c%c%c%c%s%c", 0x00, ERR, 0x00, code, message, 0x00);
    return len;
}

