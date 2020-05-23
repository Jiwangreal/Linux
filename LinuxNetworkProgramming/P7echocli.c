//
// Created by jxq on 19-7-17.
//

// 视频 1

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define ERR_EXIT(m) (perror(m),exit(EXIT_FAILURE))
using namespace std;

#define ERR_EXIT(m) \
        do  \
        {   \
            perror(m);  \
            exit(EXIT_FAILURE); \
        } while(0);

int main(int argc, char** argv) {
    // 1. 创建套接字
    int sockfd;
    //0表示内核去自动选择协议
    //if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        ERR_EXIT("socket");
    }

    // 2. 分配套接字地址
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof servaddr);
    servaddr.sin_family = AF_INET;//地址族一般用AF_INET
    servaddr.sin_port = htons(6666);//需要网络字节序的端口号
    // servaddr.sin_addr.s_addr = htonl(INADDR_ANY);//INADDR_ANY表示本机任意地址
     servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    // inet_aton("127.0.0.1", &servaddr.sin_addr);


    // 3. 请求链接
    if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        ERR_EXIT("connect");
    }

    // 4. 数据交换
    char recvbuf[1024]={0};
    char sendbuf[1024]={0};
    while (fgets(sendbuf, sizeof(sendbuf), stdin) != NULL)   // 键盘输入获取，fget一行数据默认有换行符
    {
//        memset(recvbuf, 0, sizeof recvbuf);
//        memset(sendbuf, 0, sizeof sendbuf);
        write(sockfd, sendbuf, sizeof sendbuf); // 写入服务器
        int ret = read(sockfd, recvbuf, sizeof recvbuf);    // 服务器读取

        fputs(recvbuf, stdout); // 服务器返回数据输出

        // 清空缓冲区
        memset(recvbuf, 0, sizeof(recvbuf));
        memset(sendbuf, 0, sizeof(sendbuf));
    }

    // 5. 断开连接
    close(sockfd);



    return 0;
}
