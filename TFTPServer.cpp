//
// Created by Tom Wang on 16/6/14.
//

#include "DefineType.h"
#include "ServerSocket.h"

using namespace std;

int main (int argc, char **argv)
{
    extern char *optarg;
    int sock, n, Length, status, tid;
    char opcode, *bufindex, filename[196], mode[12];

    ServerSocket* server;
    struct sockaddr_in client;

    try{
        server = new ServerSocket(port);
    } catch (ExceptionSock Err) {
        cerr << Err.description() << endl;
        return 0;
    }

    sock = server->get_sock();
    try {
        while (1) {
            Length = sizeof(client);
            memset(NetBuffer, 0, BUFSIZ);

            n = 0;
            while (errno == EAGAIN || n == 0) {
                waitpid(-1, &status, WNOHANG);
                n = REVEFROM(NetBuffer, BUFSIZ, client);
                if (n < 0 && errno != EAGAIN) {
                    throw ExceptionSock("服务器无法收到客户端信息");
                }
                usleep(1000);
            }

            printf("来自 %s 的连接, 端口号为 %d\n", inet_ntoa(client.sin_addr), ntohs (client.sin_port));

            bufindex = NetBuffer;
            if (bufindex++[0] != 0x00) {
                throw ExceptionSock("包格式错误!");
            }
            tid = ntohs (client.sin_port);
            DealPacket(filename, mode, &opcode, &bufindex);
            switch (opcode)
            {
                case 1:
                    printf("发送文件请求\n");
                    try {
                        Send(filename, client, mode, tid);
                    } catch (ExceptionSock Err) {
                        cerr << Err.description();
                    }
                    break;
                case 2:
                    printf("接收文件请求\n");
                    try {
                        Get(filename, client, mode, tid);
                    } catch (ExceptionSock Err) {
                        cerr << Err.description();
                    }
                    break;
                default:
                    printf("无效包,忽略\n");
                    break;
            }
        }
    } catch (ExceptionSock Err) {
        cerr << Err.description();
    }
}



