//
// Created by Tom Wang on 16/6/14.
//

#include "ServerSocket.h"


ServerSocket::ServerSocket(int port) {
    if ( ! Socket::create() )
    {
        throw ExceptionSock ( "Could not create server socket." );
    }

    if ( ! Socket::bind ( port ) )
    {
        throw ExceptionSock ( "Could not bind to port." );
    }
}

ServerSocket::ServerSocket(const ServerSocket& C) {
    Socket::m_sock = C.Socket::m_sock;
}

ServerSocket::~ServerSocket() {

}

bool TestInformation(char* mode, char* filename, int sock,struct sockaddr_in client){
    int len;
    char PACKETBUFFER[MAXACKFREQ][MAXDATASIZE+12];
    if (!strncasecmp (mode, "octet", 5) && !strncasecmp (mode, "netascii", 8)) {//检查模式是否合法
        printf("模式错误!\n");
        std::string ErrMess = "错误的模式(";
        ErrMess = ErrMess+mode+")";
        len = SendErrPacket(0x04,ErrMess.c_str(),PACKETBUFFER[0]);
        if (SEND(PACKETBUFFER[0]) != len) {
            throw ExceptionSock("包大小错误\n");
        }
        return false;
    }
    if (strchr (filename, 0x5C) || strchr (filename, 0x2F)) {//检查文件名是否合法
        printf ("文件名非法!\n");
        std::string ErrMess = "错误的文件名(";
        ErrMess = ErrMess+filename+")";
        len = SendErrPacket(0x01,ErrMess.c_str(),PACKETBUFFER[0]);
        if (SEND(PACKETBUFFER[0]) != len) {
            throw ExceptionSock("包大小错误\n");
        }
        return false;
    }
    return true;
}

void Get (char *pFilename, struct sockaddr_in client, char *pMode, int tid) {
    int sock, len = 0, Length, opcode, i, j, n, flag = 1;
    unsigned short int count = 0, rcount = 0;
    unsigned char filebuf[MAXDATASIZE + 1];
    char PACKETBUFFER[MAXDATASIZE + 12];
    extern int errno;
    char filename[128], mode[12], fullpath[196], *bufindex, REVEBUF[512];
    struct sockaddr_in data;
   // char *ipaddress;

    FILE *fp;
    strcpy(filename, pFilename);
    strcpy(mode, pMode);
    printf("文件接收程序开始\n");

    if ((sock = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        throw ExceptionSock("无法连接到服务器!\n");
    }
    if (!TestInformation(mode,filename,sock,client)){
        return;
    }

    strcpy(fullpath, path);
    strncat(fullpath, filename, sizeof (fullpath) - 1);	//调整文件名
    if (!access(fullpath,0)) {
        printf("文件已存在!");
        std::string ErrMess = "文件已存在";
        len = SendErrPacket(0x06,ErrMess.c_str(),PACKETBUFFER);
        if (SEND(PACKETBUFFER) != len) {
            throw ExceptionSock("包大小错误\n");
        }
        return;
    }
    fp = fopen(fullpath, "w");

    if (fp == NULL) {//建立文件失败
        printf ("建立文件失败!");
        std::string ErrMess = "文件无法打开";
        len = SendErrPacket(0x03,ErrMess.c_str(),PACKETBUFFER);
        if (SEND(PACKETBUFFER) != len) {
            throw ExceptionSock("包大小错误\n");
        }
        return;
    }
    else {
        printf ("开始接收文件 %s\n",fullpath);
    }

    memset (filebuf, 0, sizeof (filebuf));
    n = datasize + 4;
    do {
        memset (PACKETBUFFER, 0, sizeof (PACKETBUFFER));
        memset (REVEBUF, 0, sizeof (REVEBUF));
        if (count == 0 || (count % ackfreq) == 0 || n != (datasize + 4)) {
            len = sprintf (REVEBUF, "%c%c%c%c", 0x00, 0x04, 0x00, 0x00);
            REVEBUF[2] = (char) ((count & 0xFF00) >> 8);
            REVEBUF[3] = (char) (count & 0x00FF);	//填充计数器

            if (SEND(REVEBUF) != len) {
                throw ExceptionSock("包大小错误\n");
            }
        }
        else {
            printf ("未收到ACK请求,计数:%d\n", count);
        }
        if (n != (datasize + 4)) {	//文件已经成功接收
            printf("文件接收成功,大小: %d. 退出循环\n", n - 4);
            goto done;
        }
        memset (filebuf, 0, sizeof (filebuf));
        count++;

        for (j = 0; j < RETRIES; j++) {	//在指定重试次数下重试
            Length = sizeof (data);
            errno = EAGAIN;
            n = -1;
            for (i = 0; errno == EAGAIN && i <= TIMEOUT && n < 0; i++) { //检查到达的包
                n = REVEFROM(PACKETBUFFER, sizeof(PACKETBUFFER)-1,data);
                usleep (1000);
            }

            if (n < 0 && errno != EAGAIN) {	/* 收不到信息*/
                printf("服务器无法接收客户端信息! (errno: %d n: %d)\n", errno, n);

            } else if (n < 0 && errno == EAGAIN) {	/* 等待超时 */
                printf ("等待超时! (errno: %d == %d n: %d)\n", errno, EAGAIN, n);
            } else {
                if (client.sin_addr.s_addr != data.sin_addr.s_addr) {	/* 检查地址 */
                    printf("无法接收文件!网络地址错误!\n");
                    j--;
                    continue;
                }
               // ipaddress = inet_ntoa(client.sin_addr);
                if (tid != ntohs (client.sin_port))	{ /* 检查端口 */
                    printf ("无法接收文件!端口错误!)\n");
                    std::string ErrMess = "端口错误!";
                    len = SendErrPacket(0x05,ErrMess.c_str(),PACKETBUFFER);
                    if (SEND(PACKETBUFFER) != len) {
                        throw ExceptionSock("包大小错误\n");
                    }
                    j--;
                    continue;
                }

                bufindex = (char *) PACKETBUFFER;
                if (bufindex++[0] != 0x00)
                    printf ("包头错误!\n");
                opcode = *bufindex++;
                rcount = (unsigned short) (*bufindex++ << 8);
                rcount &= 0xff00;
                rcount += (*bufindex++ & 0x00ff);
                memcpy ((char *) filebuf, bufindex, (size_t) (n - 4));	/* 将信息保存在数组里 */
                if (flag) {
                    if (n > 516)
                        datasize = n - 4;
                    flag = 0;
                }
                if (opcode != 3 || rcount != count) {
                    printf("无效数据包格式\n");
                    if (opcode > 5) {
                        std::string ErrMess = "无效的操作";
                        len = SendErrPacket(0x04,ErrMess.c_str(),PACKETBUFFER);
                        if (SEND(PACKETBUFFER) != len) {
                            throw ExceptionSock("包大小错误\n");
                        }
                    }

                }
                else {
                    break;
                }
            }
            if (SEND(REVEBUF) != len) {
                throw ExceptionSock("包大小错误\n");
            }
        }
        if (j == RETRIES) {
            fclose (fp);
            throw ExceptionSock("接收超时\n");
        }
    }

    while(fwrite (filebuf, 1, (size_t) (n - 4), fp) == n - 4);
    fclose (fp);
    sync ();
    printf("文件接受失败,关闭文件\n");
    return;
    done:
    fclose(fp);
    sync();
    printf("文件接收成功,关闭文件\n");
    char Data[255];
    sprintf(Data,"::文件发送成功,文件名:%s\n",filename);
    PrintLog(Data);
    return;
}

void Send (char *pFilename, struct sockaddr_in client, char *pMode, int tid) {
    int sock, len, Length, opcode, ssize = 0, n, i, j = 0, bcount = 0;
    unsigned short int count = 0, rcount = 0;
    unsigned char filebuf[MAXDATASIZE + 1];
    char PACKETBUFFER[MAXACKFREQ][MAXDATASIZE + 12], RECVBUFF[MAXDATASIZE + 12];
    char filename[128], mode[12], fullpath[196], *bufindex;
    struct sockaddr_in ack;
  //  char *ipaddress;
    FILE *fp;

    strcpy (filename, pFilename);
    strcpy (mode, pMode);

    printf ("文件发送程序开始!");


    if ((sock = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        throw ExceptionSock("无法连接到服务器!\n");
    }
    if (!TestInformation(mode,filename,sock,client)){
        return;
    }
    strcpy (fullpath, path);
    strncat (fullpath, filename, sizeof (fullpath) - 1);
    fp = fopen (fullpath, "r");

    if (fp == NULL) {	//建立文件失败
        printf ("无此文件!");
        std::string ErrMess = "文件无法找到";
        len = SendErrPacket(0x01,ErrMess.c_str(),PACKETBUFFER[0]);
        if (SEND(PACKETBUFFER[0]) != len) {
            throw ExceptionSock("包大小错误\n");
        }
        return;
    }
    else {
        printf ("开始发送文件 %s\n",fullpath);
    }
    memset (filebuf, 0, sizeof (filebuf));

    while (1) {

        ssize = (int) fread (filebuf, 1, (size_t) datasize, fp);
        printf ("文件大小: %d\n", ssize);
        count++;
        if (count == 1)		//对前两个包,标记bcount为0
            bcount = 0;
        else if (count == 2)
            bcount = 0;
        else
            bcount = (count - 2) % ackfreq;

        sprintf ((char *) PACKETBUFFER[bcount], "%c%c%c%c", 0x00, 0x03, 0x00, 0x00);
        memcpy ((char *) PACKETBUFFER[bcount] + 4, filebuf, (size_t) ssize);
        len = 4 + ssize;
        PACKETBUFFER[bcount][2] = (char) ((count & 0xFF00) >> 8);
        PACKETBUFFER[bcount][3] = (char) (count & 0x00FF);	//填充计数器

        if (SEND(PACKETBUFFER[bcount]) != len) {
            throw ExceptionSock("包大小错误\n");
        }


        if ((count - 1) == 0 || ((count - 1) % ackfreq) == 0 || ssize != datasize) {
            for (j = 0; j < RETRIES; j++) {
                Length = sizeof (ack);
                errno = EAGAIN;
                n = -1;
                for (i = 0; errno == EAGAIN && i <= TIMEOUT && n < 0; i++) {
                    n = REVEFROM(RECVBUFF,sizeof(RECVBUFF),ack);
                    usleep (1000);
                }
                if (n < 0 && errno != EAGAIN) {	/* 收不到信息*/
                    printf("服务器无法接收客户端信息! (errno: %d n: %d)\n", errno, n);
                } else if (n < 0 && errno == EAGAIN) {	/* 等待超时 */
                    printf ("等待超时! (errno: %d == %d n: %d)\n", errno, EAGAIN, n);
                } else {
                    if (client.sin_addr.s_addr != ack.sin_addr.s_addr) {	/* 检查地址 */
                        printf("无法接收文件!网络地址错误!\n");
                        j--;
                        continue;
                    }
                //    ipaddress = inet_ntoa(client.sin_addr);
                    if (tid != ntohs (client.sin_port)) {	/* 检查端口 */
                        printf ("无法接收文件!端口错误!)\n");
                        std::string ErrMess = "错误的端口号";
                        len = SendErrPacket(0x05,ErrMess.c_str(),RECVBUFF);
                        if (SEND(RECVBUFF)!= len) {
                            throw ExceptionSock("包大小错误\n");
                        }
                        j--;
                        continue;
                    }

                    bufindex = (char *) RECVBUFF;
                    if (bufindex++[0] != 0x00)
                        printf ("包头错误!\n");
                    opcode = *bufindex++;
                    rcount = (unsigned short) (*bufindex++ << 8);
                    rcount &= 0xff00;
                    rcount += (*bufindex++ & 0x00ff);
                    if (opcode != 4 || rcount != count) {	/* 检查客户端反馈 */
                        if (opcode > 5) {
                            std::string ErrMess = "非法操作!";
                            len = SendErrPacket(0x04,ErrMess.c_str(),RECVBUFF);
                            if (SEND(RECVBUFF) != len) {
                                throw ExceptionSock("包大小错误\n");
                            }
                        }
                    }
                    else {
                        break;
                    }
                }
                for (i = 0; i <= bcount; i++) {
                    if (SEND(PACKETBUFFER[i]) != len) {
                        throw ExceptionSock("包大小错误\n");
                    }
                    printf ("无应答,重新发送%d包\n", count - bcount + i);
                }
                printf ("重新发送完成\n");
            }
        } else {
            n = REVEFROM(RECVBUFF,sizeof(RECVBUFF),ack);
        }
        if (j == RETRIES) {
            fclose (fp);
            throw ExceptionSock("接收超时\n");
        }
        if (ssize != datasize)
            break;
        memset (filebuf, 0, sizeof (filebuf));	/*给包尾部填充空*/
    }
    fclose (fp);
    printf ("文件成功发送\n");
    char Data[255];
    sprintf(Data,"::文件发送成功,文件名:%s\n",pFilename);
    PrintLog(Data);
    return;
}