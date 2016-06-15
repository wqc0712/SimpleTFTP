//
// Created by Tom Wang on 16/6/15.
//

#include "ClientSocket.h"
#include "ExceptionSock.h"
#include "DefineType.h"

void Send (char *pFilename, struct sockaddr_in client, char *pMode, int sock) {
    int len, Length, opcode, ssize = 0, n, i, j, bcount = 0, tid = 0;
    unsigned short int count = 0, rcount = 0;
    unsigned char filebuf[MAXDATASIZE + 1];
    char PACKETBUFFER[MAXACKFREQ][MAXDATASIZE + 12],
            RECVBUFF[MAXDATASIZE + 12];
    char filename[128], mode[12], *bufindex;
    struct sockaddr_in ack;

    FILE *fp;

    strcpy (filename, pFilename);
    strcpy (mode, pMode);

    printf ("文件发送程序开始!");

    fp = fopen (filename, "r");
    if (fp == NULL) {	//找不到文件
        throw ExceptionSock("找不到文件!");
    }
    else {
        printf ("开始发送文件 %s\n",filename);
    }

    for (j = 0; j < RETRIES - 2; j++) { //在指定重试次数下重试,代码与ServerSocket中137行相同
        Length = sizeof (ack);
        errno = EAGAIN;
        n = -1;
        for (i = 0; errno == EAGAIN && i <= TIMEOUT && n < 0; i++) {
            n = REVEFROM(RECVBUFF,sizeof(RECVBUFF),ack);
            usleep (1000);
        }
        tid = ntohs (ack.sin_port);
        client.sin_port = htons (tid);
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

            if (tid != ntohs (client.sin_port))	{ /* 检查端口 */
                printf ("无法接收文件!端口错误!)\n");
                std::string ErrMess = "端口错误!";
                len = SendErrPacket(0x05,ErrMess.c_str(),NetBuffer);
                if (SEND(NetBuffer) != len) {
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
            if (opcode != 4 || rcount != count) {
                if (opcode > 5) {
                    std::string ErrMess = "非法操作!";
                    len = SendErrPacket(0x04,ErrMess.c_str(),NetBuffer);
                    if (SEND(NetBuffer) != len) {
                        throw ExceptionSock("包大小错误\n");
                    }
                }
            }
            else {
                break;
            }
        }
    }

    memset (filebuf, 0, sizeof (filebuf));//文件写入操作,与ServerSocket260行开始相似
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

        if (((count) % ackfreq) == 0 || ssize != datasize) {
            for (j = 0; j < RETRIES; j++)
            {
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
    return;
}

void Get (char *pFilename, struct sockaddr_in client, char *pMode, int sock) {
    int len, Length, opcode, i, j, n, tid = 0, flag = 1;
    unsigned short int count = 0, rcount = 0;
    unsigned char filebuf[MAXDATASIZE + 1];
    char PACKETBUFFER[MAXDATASIZE + 12];
    extern int errno;
    char filename[128], mode[12], *bufindex, REVEBUF[512];
    struct sockaddr_in data;
    FILE *fp;

    strcpy(filename, pFilename);
    strcpy(mode, pMode);
    printf("文件接收程序开始\n");

    fp = fopen(filename, "w");
    if (fp == NULL) {//建立文件失败
        throw ExceptionSock("建立文件失败!");
    }
    else {
        printf ("开始接收文件 %s\n",filename);
    }

    memset (filebuf, 0, sizeof (filebuf));
    n = datasize + 4;
    do {
        memset (PACKETBUFFER, 0, sizeof (PACKETBUFFER));
        memset (REVEBUF, 0, sizeof (REVEBUF));
        if (n != (datasize + 4)) {
            len = sprintf (REVEBUF, "%c%c%c%c", 0x00, 0x04, 0x00, 0x00);
            REVEBUF[2] = (char) ((count & 0xFF00) >> 8);
            REVEBUF[3] = (char) (count & 0x00FF);	//填充计数器

            if (SEND(REVEBUF) != len) {
                throw ExceptionSock("包大小错误\n");
            }
            printf("成功发送ACK请求!");
            goto done;
        }
        count++;
        for (j = 0; j < RETRIES; j++) {	//在指定重试次数下重试
            Length = sizeof (data);
            errno = EAGAIN;
            n = -1;
            for (i = 0; errno == EAGAIN && i <= TIMEOUT && n < 0; i++) { //检查到达的包
                n = REVEFROM(PACKETBUFFER, sizeof(PACKETBUFFER)-1,data);
                usleep (1000);
            }

            if (!tid) {
                tid = ntohs(data.sin_port);	//获得发送端口号并设置
                client.sin_port = htons(tid);
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
                    else if (n < 516)
                        datasize = 512;
                    flag = 0;
                }
                if (opcode != 3)
                {
                    printf("无效数据包格式\n");
                    if (opcode > 5) {
                        std::string ErrMess = "无效的操作";
                        len = SendErrPacket(0x04,ErrMess.c_str(),PACKETBUFFER);
                        if (SEND(PACKETBUFFER) != len) {
                            throw ExceptionSock("包大小错误\n");
                        }
                    }
                } else {
                    len = sprintf (REVEBUF, "%c%c%c%c", 0x00, 0x04, 0x00, 0x00);
                    REVEBUF[2] = (char) ((count & 0xFF00) >> 8);
                    REVEBUF[3] = (char) (count & 0x00FF);
                    if (((count - 1) % ackfreq) == 0) {
                        if (SEND(REVEBUF) != len) {
                            throw ExceptionSock("包大小错误\n");
                        }
                    } else if (count == 1) {
                        if (SEND(REVEBUF) != len){
                            throw ExceptionSock("包大小错误\n");
                        }
                    }
                    break;
                }
            }
        }
        if (j == RETRIES) {
            fclose(fp);
            throw ExceptionSock("接收超时\n");
        }
    }
    while (fwrite (filebuf, 1, (size_t) (n - 4), fp) == n - 4);
    fclose(fp);
    sync();
    printf("文件接受失败,关闭文件\n");
    return;
    done:
    fclose(fp);
    sync();
    printf("文件接收成功,关闭文件\n");
}