//
// Created by jxq on 19-7-17.
//
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>


using namespace std;

#define ERR_EXIT(m) \
        do  \
        {   \
            perror(m);  \
            exit(EXIT_FAILURE); \
        } while(0);

void do_service(int connfd)
{
    char recvbuf[1024];
    while (1)
    {
        memset(recvbuf, 0, sizeof recvbuf);
        int ret = read(connfd, recvbuf, sizeof recvbuf);
        if (ret == 0)
        {
            printf("client close\n");
            break;
        } else if (ret == -1)
        {
            ERR_EXIT("read");
        }
        fputs(recvbuf, stdout);
        write(connfd, recvbuf, ret);
    }

}

void handler(int sig)
{
    printf("recv a sig = %d\n", sig);
    exit(EXIT_SUCCESS);
}

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

    // 6. 数据交换
    pid_t pid;
    pid = fork();
    
    
    if (pid == -1)
    {
        ERR_EXIT("fork");
    }
    if (pid == 0)   // 子进程
    {//接收数据进程
        while (1)
        {
            char recvbuf[1024];
            memset(recvbuf, 0, sizeof recvbuf);
            int ret = read(sockfd, recvbuf, sizeof recvbuf);
            if (ret == -1)
            {
                ERR_EXIT("read");
            }
            else if (ret == 0)
            {
                printf("peer close\n");
                break;
            }
            fputs(recvbuf, stdout);
        }
        close(sockfd);
        kill(getppid(), SIGUR1);//getppid获取父进程pid，这里通知父进程
    }
    else//发送数据的进程
    {
        signal(SIGUR1, handler);
        char sendbuf[1024];
        while (fgets(sendbuf, sizeof(sendbuf), stdin) != NULL)
        {
            write(sockfd, sendbuf, sizeof(sendbuf_);
            memset(sendbuf, 0, sizeof(sendbuf));//发送完数据清空，因为防止后面消息覆盖前面的消息
        }
        printf("child exit\n");
        close(sockfd);
    }

    // 7. 断开连接
    close(sockfd);




    return 0;
}
