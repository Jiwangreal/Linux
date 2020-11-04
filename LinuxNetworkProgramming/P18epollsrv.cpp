//
// Created by jxq on 19-8-7.
//

// socket编程 13 epoll 模型

#include <iostream>
#include <stdio.h>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <poll.h>
#include <vector>
#include <sys/stat.h>
#include <fcntl.h>
#include <algorithm>


using namespace std;

typedef vector<struct epoll_event> EventList;//vector：动态数组

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

void activate_nonblock(int fd)
{
    int ret;
    int flags = fcntl(fd, F_GETFL);
    if(flags == -1)
        ERR_EXIT("fcntl");
    flags |= O_NONBLOCK;
    ret = fcntl(fd, F_SETFL, flags);
    if(ret == -1)
        ERR_EXIT("fcntl");
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
    socklen_t peerlen;


    // 6. 数据交换
    int nready;
    int connfd;
    int i;
    vector<int> clients;//保存客户端的已连接套接字
    int epollfd;
    //EPOLL_CLOEXEC含义：该进程被替换的时候，文件描述会被关闭
    epollfd = epoll_create1(EPOLL_CLOEXEC); // 创建一个epoll的实例

    struct epoll_event event;
    //typedef union epoll_data{...};
    //struct epoll_enent{_uint32_t enents; epoll_data data};
    //感兴趣的fd是监听listenfd
    event.data.fd = listenfd;//data是一个联合体，共用体，共用体的大小是8个字节
    event.events = EPOLLIN | EPOLLET;//listenfd感兴趣的事件：EPOLLIN，是否事件到来，EPOLLET表示边沿方式触发
    epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &event);//（epoll实例句柄，操作方式，将fd添加至epoll来管理，该fd感兴趣的事件）

    EventList events(16);//events数组的初始值为16

    while (1)
    {
        //epoll_wait检测哪一些IO产生了事件，（epoll实例句柄，哪些事件产生了感兴趣的事件，能够返回的最大事件个数，超时时间）
        // 等侍注册在epfd上的socket fd的事件的发生，如果发生则将发生的sokct fd和事件类型放入到events数组中
        //events.begin()是一个迭代器，可以看成一个指针，*events.begin()：取数组第一个元素，类型就是struct epoll_event
        //&*events.begin()：动态数组的首地址
        //不直接使用vents.begin()，是因为他类型是一个迭代器，类型不匹配，编译不通过，取&*就等价于struct epoll_event*
        nready = epoll_wait(epollfd, &*events.begin(), static_cast<int>(events.size()), -1);    // -1：一直等待，直到有事件产生

        if (nready == -1)
        {
            if (errno == EINTR)
            {
                continue;
            }
            ERR_EXIT("epoll_wait");
        }

        if (nready == 0)
        {
            continue;
        }

        //nready：等待到的事件个数
        if ((size_t)nready == events.size())//说明容器容量不够大，需要调整容器容量的大小
        {
            events.resize(events.size()*2);
        }

        //返回的事件，保存在&*events.begin()中
        for (i = 0; i < nready; ++i)//返回了nready个事件
        {
            //epoll效率比select和poll高的核心：返回的事件保存在events中，通过events可以找到fd，也就是说这些fd产生了事件
            //就不需要遍历哪些socket在某个集合中产生了可读事件，因为events指示的fd已经产生了事件
            if (events[i].data.fd == listenfd)//listenfd产生了可读事件
            {
                peerlen = sizeof(peeraddr);
                connfd = accept(listenfd, (struct sockaddr*)&peeraddr, &peerlen);
                if (connfd == -1)
                {
                    ERR_EXIT("accept");
                }
                printf("id = %s, ", inet_ntoa(peeraddr.sin_addr));
                printf("port = %d\n", ntohs(peeraddr.sin_port));
                clients.push_back(connfd);
                activate_nonblock(connfd);

                event.data.fd = connfd;
                event.events = EPOLLIN | EPOLLET;
                epoll_ctl(epollfd, EPOLL_CTL_ADD, connfd, &event);

                //下一次epoll_wait就有可能产生监听fd和连接fd产生可读事件

            }//只需要判断events产生了哪些事件 ，可读，可写？
            //不需要遍历哪个fd产生了事件，因为events[i].data.fd中的fd就是产生事件的套接字
            else if (events[i].events & EPOLLIN)//已连接fd产生了可读事件
            {
                connfd = events[i].data.fd;//取出已连接fd
                if (connfd < 0)
                {
                    continue;
                }
                char recvbuf[1024];
                int ret = readline(connfd, recvbuf, sizeof(recvbuf));
                if (ret == -1)
                {
                    ERR_EXIT("readline");
                }
                if (ret == 0)//表示对方关闭
                {
                    printf("client close\n");
                    close(connfd);

                    event = events[i];//对方关闭，要将该events[i]从epollfd中删除
                    epoll_ctl(epollfd, EPOLL_CTL_DEL, connfd, &event);
                    //删除已连接fd
                    clients.erase(std:remove(clients.begin(), clients.end(), connfd),clients.end());
                    // clients.erase(
                    //         remove_if(clients.begin(), clients.end(), [connfd](int n){return n == connfd;}),
                    //         clients.end());
                }
                fputs(recvbuf, stdout);
                writen(connfd, recvbuf, strlen(recvbuf));
            }
        }

    }
    // 7. 断开连接
    close(listenfd);
    return 0;
}