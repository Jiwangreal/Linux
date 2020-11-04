//
// Created by jxq on 19-8-10.
//

//
// Created by jxq on 19-8-10.
//

// socket编程 17

#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;

#define ERR_EXIT(m) \
        do \
        { \
            perror(m); \
            exit(EXIT_FAILURE); \
        } while(0);



void send_fd(int sock_fd, int send_fd)
{
    int ret;
    struct msghdr msg;
    struct cmsghdr *p_cmsg;//p_cmsg一定要指向某块缓冲区
    struct iovec vec;
    char cmsgbuf[CMSG_SPACE(sizeof(send_fd))];  // 获取辅助数据对象的空间
    int *p_fds;
    char sendchar = 0;


    //第一步指定缓冲区：msg_iov是指向真正要发送的数据，类似send的缓冲区
    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = &vec;//指定缓冲区，msg_iov指向vec结构体
    msg.msg_iovlen = 1; // 主要目的不是传递数据，是为了发送辅助数据，故只传1个字符
    msg.msg_flags = 0;

    //填充vec结构体
    vec.iov_base = &sendchar;
    vec.iov_len = sizeof(sendchar);


    //第二步：填充辅助数据开始
    //msg_control指向辅助数据
    msg.msg_control = cmsgbuf;
    msg.msg_controllen = sizeof(cmsgbuf);

    p_cmsg = CMSG_FIRSTHDR(&msg);   // CMSG_FIRSTHDR宏可以得到首个 cmsghdr 结构体的头部指针
    p_cmsg->cmsg_level = SOL_SOCKET;//表示要传递fd
    p_cmsg->cmsg_type = SCM_RIGHTS;//表示要传递fd
    p_cmsg->cmsg_len = CMSG_LEN(sizeof(send_fd));

    // 传入结构体指针 p_cmsg ，宏CMSG_DATA 可以得到准备存放send_fd 的位置指针，将send_fd 放进去
    p_fds = (int *)CMSG_DATA(p_cmsg);//通过CMSG_DATA函数得到数据数据的首地址
    *p_fds = send_fd; // 将send_fd放到4个字节的*p_fds空间中 
    //填充辅助数据结束




    ret = sendmsg(sock_fd, &msg, 0);

    if (ret != 1)
        ERR_EXIT("sendmsg");
}

int recv_fd(const int sock_fd)
{
    int ret;
    struct msghdr msg;
    char recvchar;
    struct iovec vec;
    int recv_fd;

    char cmsgbuf[CMSG_SPACE(sizeof(recv_fd))];
    struct cmsghdr *p_cmsg;
    int *p_fd;

    vec.iov_base = &recvchar;
    vec.iov_len = sizeof(recvchar);

    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = &vec;
    msg.msg_iovlen = 1;
    msg.msg_control = cmsgbuf;
    msg.msg_controllen = sizeof(cmsgbuf);
    msg.msg_flags = 0;

    p_fd = (int *)CMSG_DATA(CMSG_FIRSTHDR(&msg));
    *p_fd = -1;

    ret = recvmsg(sock_fd, &msg, 0);
    if (ret != 1)
        ERR_EXIT("recvmsg");

    //取出辅助数据
    p_cmsg = CMSG_FIRSTHDR(&msg);   // 首个cmsghdr地址
    if (p_cmsg == NULL)
        ERR_EXIT("no passed fd");

    //真正的数据部分取出来
    p_fd = (int *)CMSG_DATA(p_cmsg);    // 文件描述符
    recv_fd = *p_fd;
    if (recv_fd == -1)
        ERR_EXIT("no passed fd");

    return recv_fd;
}

int main(int argc, char** argv)
{
    int socketfd[2];
    int r = socketpair(AF_UNIX, SOCK_STREAM, 0, socketfd);

    if (r < 0)
    {
        ERR_EXIT("socketpair");
    }

    pid_t pid = fork();
    
    //如果是子进程打开的fd，父进程只能通过fd的传递方法来实现共享子进程的fd
    //fd的传递只能通过UNIX域协议socket，tcp和udp是不能传递fd的
    if (pid == -1)
    {
        ERR_EXIT("fork");
    }
    if (pid > 0)
    {
        int val = 0;
        close(socketfd[1]);
        int fd = recv_fd(socketfd[0]);
        char buf[1024] = {0};
        read(fd, buf, sizeof buf);
        printf("buf =  %s\n", buf);
    }
    else if (pid == 0)
    {
        close(socketfd[0]);
        int fd;
        fd = open("test.txt", O_RDONLY);
        if (fd == -1)
        {
            ERR_EXIT("open");
        }
        send_fd(socketfd[1], fd);//将子进程的fd传递给父进程
    }
    return 0;
}