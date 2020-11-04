//
// Created by wangji on 19-8-6.
//

// socket编程 8 select模型

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/wait.h>


using namespace std;

struct packet
{
    int len;
    char buf[1024];
};

#define ERR_EXIT(m) \
        do  \
        {   \
            perror(m);  \
            exit(EXIT_FAILURE); \
        } while(0);

//客户端仅作连接
int main(int argc, char** argv) {
    
    int count = 0;
    while(1)
    {
        // 1. 创建套接字
        int sockfd;
        if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) 
        {
            sleep(4);//当客户端fd超过上限，先sleep4s再退出来，推迟close
            ERR_EXIT("socket");
        }

        // 2. 分配套接字地址
        struct sockaddr_in servaddr;
        memset(&servaddr, 0, sizeof servaddr);
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(6666);
        // servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
        // inet_aton("127.0.0.1", &servaddr.sin_addr);

        // 3. 请求链接
        if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof servaddr) < 0) {
            ERR_EXIT("connect");
        }

        struct sockaddr_in localaddr;
        socklen_t addrlen = sizeof localaddr;
        if (getsockname(sockfd, (struct sockaddr*)&localaddr, &addrlen) < 0)
        {
            ERR_EXIT("getsockname");
        }
        printf("id = %s, ", inet_ntoa(localaddr.sin_addr));
        printf("port = %d\n", ntohs(localaddr.sin_port));

        printf("count = %d\n", ++count);
    }





    return 0;
}