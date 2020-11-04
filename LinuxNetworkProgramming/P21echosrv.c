#include <unistd.h>
#include <sys/types.h>
// #include <netinet/in.h>//这是网际协议的头文件

//man unix得到
#include <sys/un.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>



#define ERR_EXIT(m) \
    do \
    { \
        perror(m); \
        exit(EXIT_FAILURE); \
    } while (0);
    
void echo_srv(int conn)
{
    char recvbuf[1024];
    int n;
    while (1)
    {
        memset(recvbuf, 0, sizeof(recvbuf));
        n = read(conn, recvbuf, sizeof(recvbuf));
        if (n == -1)
        {
            if (n == EINTR)
                continue;
            ERR_EXIT("read");
        }
        else if (n == 0)
        {
            printf("client close");
            break;
        }
        fputs(recvbuf, stdout);
        write(conn, recvbuf, strlen(recvbuf));
    }
    close(conn);
}

int main(void)
{
    int listenfd;
    if ((listenfd = socket(PF_UNIX, SOCK_STREAM, 0)) < 0)
        ERR_EXIT("socket");
    
    unlink("test_socket");//解决服务端再次创建时，出现：bind： Address already in use
    //UNIX域协议套接字
    //0表示内核自动选择UNIX域协议套接字协议
    struct sockaddr_un servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servadr.sun_family = AF_UNIX;
    strcpy(servadr.sun_path, "test_socket");//一般放在"/tmp/test_socket"

    //初始化地址结构，与tcp不一样，不需要设置地址重复利用
    //绑定监听
    //在bind时，会在当前目录下产生test_socket文件
    if ((bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) < 0)
        ERR_EXIT("bind");
    //SOMAXCONN：监听队列最大值
    if (listen(listenfd, SOMAXCONN) < 0)
        ERR_EXIT("listen");

    //接收客户端连接
    int conn;
    pid_t pid;
    //accept(listenfd,对等方地址，对等方地址长度)
    while (1)
    {
        conn = accept(listenfd, NULL, NULL);
        if (conn == -1)
        {
            if (conn == EINTR)
                continue;
            ERR_EXIT("accept");
        }
        pid = fork();
        if (pid == -1)
            ERR_EXIT("fork");
        
        if (pid == 0)
        {
            close(listenfd);//子进程不许呀处理监听fd
            echo_srv(conn);
            exit(EXIT_SUCCESS);
        }
        close(conn);//父进程不需要处理连接fd

    }



    return 0;
}