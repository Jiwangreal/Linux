//
// Created by wangji on 19-7-17.
//
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

using namespace std;

#define ERR_EXIT(m) \
        do  \
        {   \
            perror(m);  \
            exit(EXIT_FAILURE); \
        } while(0);

int main(int argc, char** argv) {
    // 1. 创建套接字
    int listenfd;
    //0表示内核去自动选择协议
    //if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    if ((listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        ERR_EXIT("socket");
    }

    // 2. 分配套接字地址
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof servaddr);
    servaddr.sin_family = AF_INET;//地址族一般用AF_INET
    servaddr.sin_port = htons(6666);//需要网络字节序的端口号
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);//INADDR_ANY表示本机任意地址
    // servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");//接收一个网络字节序的ip地址：servaddr.sin_addr.s_addr
    // inet_aton("127.0.0.1", &servaddr.sin_addr);

    int on = 1;//表示开启
    // 确保time_wait状态下同一端口仍可使用
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
    {
        ERR_EXIT("setsockopt");
    }

    // 3. 绑定套接字地址
    if (bind(listenfd, (struct sockaddr*) &servaddr, sizeof(servaddr)) < 0) {
        ERR_EXIT("bind");
    }
    // 4. 等待连接请求状态
    //SOMAXCONN监听队列最大值
    //listen将listenfd变成被动套接字，默认是主动套接字
    /*
    被动套接字：accept连接
    主动套接字：connect发起连接
    */
    if (listen(listenfd, SOMAXCONN) < 0) {
        ERR_EXIT("listen");
    }
    // 5. 允许连接
    struct sockaddr_in peeraddr;
    socklen_t peerlen = sizeof(peeraddr);
    int connfd;
    //从已连接条目的队头得到一个连接，三次握手完成后，会在已完成连接条目会有一条记录，
    //accept会将这条记录移走，以便更多的客户能连接过来
    //accept会返回一个新的套接字，connfd这里是主动套接字
    if ((connfd = accept(listenfd, (struct sockaddr *) &peeraddr, &peerlen)) < 0) {
        ERR_EXIT("accept");
    }

    printf("id = %s, ", inet_ntoa(peeraddr.sin_addr));//转换成点分十进制
    printf("port = %d\n", ntohs(peeraddr.sin_port));

    // 6. 数据交换
    char recvbuf[1024];
    while (1)
    {
        memset(recvbuf, 0, sizeof recvbuf);
        int ret = read(connfd, recvbuf, sizeof recvbuf);

        fputs(recvbuf, stdout);//man fputs
        write(connfd, recvbuf, ret);
    }

    // 7. 断开连接
    close(connfd);
    close(listenfd);



    return 0;
}












