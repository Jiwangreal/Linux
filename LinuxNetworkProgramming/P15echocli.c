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

void ehco_cli(int sockfd)
{
//    char recvbuf[1024];
//    char sendbuf[1024];
//    // struct packet recvbuf;
//    // struct packet sendbuf;
//    memset(recvbuf, 0, sizeof recvbuf);
//    memset(sendbuf, 0, sizeof sendbuf);
//    int n = 0;
//    while (fgets(sendbuf, sizeof sendbuf, stdin) != NULL)   // 键盘输入获取
//    {
//        writen(sockfd, sendbuf, strlen(sendbuf)); // 写入服务器
//
//        int ret = readline(sockfd, recvbuf, sizeof recvbuf);    // 服务器读取
//        if (ret == -1)
//        {
//            ERR_EXIT("readline");
//        }
//        if (ret == 0)
//        {
//            printf("server close\n");
//            break;
//        }
//
//        fputs(recvbuf, stdout); // 服务器返回数据输出
//
//        // 清空
//        memset(recvbuf, 0, sizeof recvbuf);
//        memset(sendbuf, 0, sizeof sendbuf);
//    }
    fd_set rset;
    FD_ZERO(&rset);

    int nready;//检测到的事件个数
    int maxfd;
    int fd_stdin = fileno(stdin);//fileno：获取标准输入的文件描述符
    if (fd_stdin > sockfd)
    {
        maxfd = fd_stdin;
    } else {
        maxfd = sockfd;
    }

    char sendbuf[1024] = {0};
    char recvbuf[1024] = {0};
    
    int stdineof = 0;

    while (1)//检测标准输入是否有可读事件，套接口是否有可读事件
    {
        //当标准输入终止时，fgets=NULL时，fd_stdin不能加入rset集中进行监听
        if (stdineof == 0)
            FD_SET(fd_stdin, &rset);//将fd_stdin放到读的集合rset中，关心fd_stdin文件描述符的事件
        FD_SET(sockfd, &rset);//将sockfd放到读的集合rset中
        nready = select(maxfd+1, &rset, NULL, NULL, NULL);//读集合中最大文件描述+1
        //到底是哪一个套接口检测到事件？rset集合是会改变的！！返回的是哪一些io或者套接口检测到了事件
        //说明rset是输入输出参数
        if (nready == -1)
        {
            ERR_EXIT("select");
        }
        if (nready == 0)
        {
            continue;
        }
        
        //标准输入IO和网络IO
        //判断标准输入产生的可读事件，还是套接口产生的可读事件？
        if (FD_ISSET(sockfd, &rset))    // 套接口产生的可读事件
        {
            int ret = readline(sockfd, recvbuf, sizeof(recvbuf));    // 服务器读取
            if (ret == -1)
            {
                ERR_EXIT("readline");
            }
            if (ret == 0)
            {
                printf("server close\n");
                break;
            }

            fputs(recvbuf, stdout); // 服务器返回数据输出
            memset(recvbuf, 0, sizeof(recvbuf));
        }

        if (FD_ISSET(fd_stdin, &rset))//判断标准输入产生的可读事件
        {
            if (fgets(sendbuf, sizeof(sendbuf), stdin) == NULL)   //等于NULL，说明客户端已经得到一个EOF结束符
            {
                stdineof = 1;
                //测试客户端给服务端发数据，服务端sleep 4s后，客户端无法读取服务端回射的数据
                close(sockfd);
                eixt(EXIT_FAILURE);
                /*
                shutdown(sockfd, SHUT_WR);
                //仅关闭写入的一半，并不意味着不能读取数据
                //实际还能读取数据，还能读取到对方的关闭通知，即readline还能读取回射的数据
                */
            }
            else
            {
                writen(sockfd, sendbuf, strlen(sendbuf)); // 写入服务器
                memset(sendbuf, 0, sizeof(sendbuf));
            }
            

        }
    }
    // close(sockfd);
};

void handle_sigchld(int sig)
{
    // wait(NULL);
    while (waitpid(-1, NULL, WNOHANG) > 0);
}


int main(int argc, char** argv) {
    
    // signal(SIGCHLD, SIG_IGN);
    signal(SIGCHLD, handle_sigchld);
    // 1. 创建套接字
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
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

    // 4. 数据交换
    ehco_cli(sockfd);

    // 5. 断开连接
    close(sockfd);


    return 0;
}