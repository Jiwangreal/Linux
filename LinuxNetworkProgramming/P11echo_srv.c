//
// Created by wangji on 19-8-6.
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

#include <sys/wait.h>

using namespace std;
//消息通过键盘输出，消息之间的边界就是/n，就不需要下面的结构体
// struct packet
// {
//     int len;
//     char buf[1024];
// };

#define ERR_EXIT(m) \
        do  \
        {   \
            perror(m);  \
            exit(EXIT_FAILURE); \
        } while(0);

ssize_t readn(int fd, void *buf, size_t count)
{
    size_t nleft = count;   // 剩余字节数
    ssize_t nread;
    char *bufp = (char*) buf;

    while (nleft > 0)
    {
        nread = read(fd, bufp, nleft);
        if (nread < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            return  -1;
        } else if (nread == 0)
        {
            return count - nleft;
        }

        bufp += nread;
        nleft -= nread;
    }
    return count;
}

ssize_t writen(int fd, const void *buf, size_t count)
{
    size_t nleft = count;
    ssize_t nwritten;
    char* bufp = (char*)buf;

    while (nleft > 0)
    {
        if ((nwritten = write(fd, bufp, nleft)) < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            return -1;
        }
        else if (nwritten == 0)
        {
            continue;
        }
        bufp += nwritten;
        nleft -= nwritten;
    }
    return count;
}

ssize_t recv_peek(int sockfd, void *buf, size_t len)
{
    while (1)
    {
        // recv有数据就返回，没有数据就阻塞
        //若对方套接口关闭，则返回为0
        //recv只能用于套接口
        int ret = recv(sockfd, buf, len, MSG_PEEK); 
        if (ret == -1 && errno == EINTR)//EINTR表示被信号中断
        {
            continue;
        }
        return ret;
    }
}

//readline只能用于套接口，因为使用了recv_peek函数
ssize_t readline(int sockfd, void *buf, size_t maxline)
{
    int ret;
    int nread;
    char *bufp = (char*)buf;    // 当前指针位置
    int nleft = maxline;//maxline一行最大的字节数，但是读取到\n就可以返回
    while (1)
    {
        ret = recv_peek(sockfd, bufp, nleft);//这里只是偷窥了缓冲区的数据，但是没有移走
        if (ret < 0)
        {
            return ret;
        }
        else if (ret == 0)//ret == 0表示对方关闭套接口
        {
            return ret;
        }
        nread = ret;

        //判断接收缓冲区是否有\n
        int i;
        for (i = 0; i < nread; i++)
        {
            if (bufp[i] == '\n')//若有\n，则将其作为一条消息读走
            {
                ret = readn(sockfd, bufp, i+1);//将数据从缓冲区移除，读取到i，说明有i+1个数据，包括\n
                if (ret != i+1)//接收到的字节数不等于i+1,说明失败
                {
                    exit(EXIT_FAILURE);
                }
                return ret;//返回一条消息
            }
        }
        
        //  若没有\n，说明还不满一条消息，也需要将数据读出来，放到缓冲区bufp

        if (nread > nleft)//从缓冲区读到的字节数要小于剩余字节数，否则有问题
        {
            exit(EXIT_FAILURE);
        }
        nleft -= nread;
        ret = readn(sockfd, bufp, nread);
        if (ret != nread)
        {
            exit(EXIT_FAILURE);
        }
        bufp += nread;//指针偏移，将数据放到屁股后面
    }
    return -1;
}

void echo_srv(int connfd)
{
    char recvbuf[1024];

    int n;
    while (1)
    {
        memset(recvbuf, 0, sizeof recvbuf);
        int ret = readline(connfd, recvbuf, 1024);//按行接收到缓冲区
        if (ret == -1)
        {
            ERR_EXIT("readline");
        }
        if (ret == 0)
        {
            printf("client close\n");
            break;
        }

        fputs(recvbuf, stdout);
        writen(connfd, recvbuf, strlen(recvbuf));
    }

}

void handle_sigchld(int sig)
{
    // wait(NULL);//捕获子进程的退出状态。man 2 wait，NULL：这里退出状态不关心
    // waitpid(-1, NULL, WNOHANG);//可以等待所有子进程,WNOHANG表示不挂起

    //轮询子进程的退出状态
    while (waitpid(-1, NULL, WNOHANG) > 0 )//将所有子进程的退出状态进行返回, >0表示等待到了一个子进程
        ;//由于指定WNOHANG，则没有子进程退出则返回-1，退出while
}


int main(int argc, char** argv) {

    // signal(SIGCHLD , SIG_IGN);//SIGCHLD可以忽略僵尸进程，不建议采用
    signal(SIGCHLD, handle_sigchld);

    // 1. 创建套接字
    int listenfd;
    if ((listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        ERR_EXIT("socket");
    }

    // 2. 分配套接字地址
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof servaddr);
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(6666);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    // servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    // inet_aton("127.0.0.1", &servaddr.sin_addr);

    int on = 1;
    // 确保time_wait状态下同一端口仍可使用
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on) < 0) {
        ERR_EXIT("setsockopt");
    }

    // 3. 绑定套接字地址
    if (bind(listenfd, (struct sockaddr *) &servaddr, sizeof servaddr) < 0) {
        ERR_EXIT("bind");
    }
    // 4. 等待连接请求状态
    if (listen(listenfd, SOMAXCONN) < 0) {
        ERR_EXIT("listen");
    }
    // 5. 允许连接
    struct sockaddr_in peeraddr;
    socklen_t peerlen = sizeof peeraddr;


    // 6. 数据交换
    pid_t pid;
    while (1) {
        int connfd;
        if ((connfd = accept(listenfd, (struct sockaddr *) &peeraddr, &peerlen)) < 0) {
            ERR_EXIT("accept");
        }

        printf("id = %s, ", inet_ntoa(peeraddr.sin_addr));
        printf("port = %d\n", ntohs(peeraddr.sin_port));

        pid = fork();

        if (pid == -1) {
            ERR_EXIT("fork");
        }
        if (pid == 0)   // 子进程
        {
            close(listenfd);
            echo_srv(connfd);
            //printf("child exit\n");
            exit(EXIT_SUCCESS);
        } else {
            //printf("parent exit\n");
            close(connfd);
        }


    }
    // 7. 断开连接
    close(listenfd);


    return 0;
}