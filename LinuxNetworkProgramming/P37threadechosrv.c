//
// Created by wangji on 19-8-6.
//

// p37 poxis 线程(一)

#include <iostream>
#include <stdio.h>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


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
        int ret = recv(sockfd, buf, len, MSG_PEEK); // 查看传入消息
        if (ret == -1 && errno == EINTR)
        {
            continue;
        }
        return ret;
    }
}

ssize_t readline(int sockfd, void *buf, size_t maxline)
{
    int ret;
    int nread;
    char *bufp = (char*)buf;    // 当前指针位置
    int nleft = maxline;
    while (1)
    {
        ret = recv_peek(sockfd, buf, nleft);
        if (ret < 0)
        {
            return ret;
        }
        else if (ret == 0)
        {
            return ret;
        }
        nread = ret;
        int i;
        for (i = 0; i < nread; i++)
        {
            if (bufp[i] == '\n')
            {
                ret = readn(sockfd, bufp, i+1);
                if (ret != i+1)
                {
                    exit(EXIT_FAILURE);
                }
                return ret;
            }
        }
        if (nread > nleft)
        {
            exit(EXIT_FAILURE);
        }
        nleft -= nread;
        ret = readn(sockfd, bufp, nread);
        if (ret != nread)
        {
            exit(EXIT_FAILURE);
        }
        bufp += nread;
    }
    return -1;
}

void echo_srv(int connfd)
{
    char recvbuf[1024];
    // struct packet recvbuf;
    int n;
    while (1)
    {
        memset(recvbuf, 0, sizeof recvbuf);
        int ret = readline(connfd, recvbuf, 1024);
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


void * start_routine(void *arg)
{
    pthread_detach(pthread_self());
    
    /*
    （1）方法1：
        int conn = *((int*)arg);//已连接套接字传递过来了
    （2）方法2：
        int conn = arg;//已连接套接字传递过来了
    （3）方法3：
        int conn = *((int*)arg);//已连接套接字传递过来了
        free(arg)；
    */
    int conn = arg;//已连接套接字传递过来了
    echo_srv(conn);//调用回射服务器
    printf("exiting thread...\n");
    // return (char *)"hello";
    retun NULL;
}

int main(int argc, char** argv) {
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

        pthread_t thread;

        /*
        (1)方式1,如果以前的start_routine没有将connfd取走的话，新的connfd又来了，他可能指向新的connfd，而原来的connfd就取不到了
        这称之为：race condition静态问题，所以不要用这样的指针传递，可以用下面的（2）值传递
        int ret = pthread_create(&thread, NULL,  start_routine, (void *)&connfd);//start_routine线程入口函数

        (2)方式2：但是(void *)在32bit机器上是4个字节，在64bit机器上8字节，所以是不可移植的
        int ret = pthread_create(&thread, NULL,  start_routine, (void *)connfd);//start_routine线程入口函数

        (3)方式3：避免了（1）的静态问题，还解决了（2）的可移植问题
        int *p = malloc(sizeof(int));
        *p = connfd;
        int ret = pthread_create(&thread, NULL,  start_routine, p);//start_routine线程入口函数
        */
       //每个线程处理一个客户端连接
        int ret = pthread_create(&thread, NULL,  start_routine, (void *)connfd);//start_routine线程入口函数
        if (ret != 0)
        {   
            fprintf(stderr,"pthread_create:%s\n",strerror(ret));
            ERR_EXIT("pthread_create");//可直接写成exit(EXIT_FAILURE);
        }
        /*
        pid = fork();
        if (pid == -1)
            ERR_EXIT("fork");
        if (pid == 0)//子进程处理回射服务
        {
            close(listenfd);
            echo_srv(connfd);//调用回射服务器
            exit(EXIT_SUCCESS);
        }
        else//父进程关闭连接套接字
            close(connfd);
        */

    }
    // 7. 断开连接
    close(listenfd);


    return 0;
}