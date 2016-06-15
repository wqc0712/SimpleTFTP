//
// Created by Tom Wang on 16/6/14.
//

#ifndef SIMPLETFTP_DEFINETYPE_H
#define SIMPLETFTP_DEFINETYPE_H

#include <iostream>
#include <string>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/wait.h>
#include <dirent.h>
#include <cstring>

#define ERROR -1

#define RRQ 1
#define WRQ 2
#define DATA 3
#define ACK 0x04
#define ERR 0x05

#define TIMEOUT 5000 /*最大超时时间*/
#define RETRIES 5 /*最大重试次数 */
#define MAXACKFREQ 16 /*最大允许包数量 */
#define MAXDATASIZE 1024 /* 最大允许数据大小 */
#define SEND(BUFF) sendto(sock,BUFF,len,0,(struct sockaddr*)&client,sizeof(client)) /*发送包*/
#define REVEFROM(BUFF,SIZE,SOCKADDR) (int)recvfrom(sock,BUFF,SIZE,MSG_DONTWAIT,(struct sockaddr*)&SOCKADDR, (socklen_t *)&Length)

static char NetBuffer[BUFSIZ];



#endif //SIMPLETFTP_DEFINETYPE_H
