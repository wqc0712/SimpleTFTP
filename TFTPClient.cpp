//
// Created by Tom Wang on 16/6/14.
//


#include "DefineType.h"
#include "ClientSocket.h"
#include "ExceptionSock.h"

string ProgType = "Client";
char Output[255];
int main (int argc, char **argv) {

    extern char *optarg;
    int sock,len, opt;
    char opcode = 0, filename[196], mode[12] = "octet";
    struct hostent *host;
    struct sockaddr_in client;
    FILE *fp;

    if (!(host = gethostbyname (argv[1]))) {
        throw ExceptionSock("Client could not get host address information");
    }

    while ((opt = getopt (argc, argv, "p:g:")) != -1)
    {
        switch (opt) {
            case 'p':
                strncpy (filename, optarg, sizeof (filename) - 1);
                opcode = WRQ;
                fp = fopen (filename, "r");
                if (fp == NULL) {
                    throw ExceptionSock("文件无法打开\n");
                }
                printf ("成功打开文件%s\n", filename);
                fclose (fp);
                break;
            case 'g':
                strncpy (filename, optarg, sizeof (filename) - 1);
                opcode = RRQ;
                fp = fopen (filename, "w");	/*opened the file for writting */
                if (fp == NULL) {
                    throw ExceptionSock("无法创建文件\n");
                }
                printf ("成功创建文件%s\n", filename);
                fclose (fp);
                break;
            default:
                throw ExceptionSock("错误的输入参数!");
        }		
    }	

    /*创建并bind socket,与clientSocket中操作相同,此处未封装*/
    if ((sock = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
       throw ExceptionSock("无法创建连接!");
    }
    memset (&client, 0, sizeof (client));
    client.sin_family = AF_INET;
    memcpy (&client.sin_addr, host->h_addr, (size_t) host->h_length);
    client.sin_port = htons (port);



    memset (NetBuffer, 0, BUFSIZ);
    len = sprintf (NetBuffer, "%c%c%s%c%s%c", 0x00, opcode, filename, 0x00, mode, 0x00);
    if (SEND(NetBuffer) != len) {
        throw ExceptionSock("包大小错误!\n");
    }
    sprintf(Output,"连接到 %s, 端口号为 %d\n", inet_ntoa(client.sin_addr), ntohs (client.sin_port));
    cout << Output;
    PrintLog(Output);
    switch (opcode) {
        case RRQ:
            printf("接收文件请求\n");
            GetC(filename, client, mode, sock);
            break;
        case WRQ:
            printf("发送文件请求\n");
            SendC(filename, client, mode, sock);
            break;
        default:
            printf("无效包,忽略\n");
    }
    close (sock);
    CloseLog();
    return 1;
}

