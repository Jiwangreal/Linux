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


#define ERR_EXIT(m) \
        do \
        { \
            perror(m); \
            exit(EXIT_FAILURE); \
        } while(0);


int main(int argc, char** argv)
{
    int fd[2];
    int r = socketpair(AF_UNIX, SOCK_STREAM, 0, fd);

    if (r < 0)
    {
        ERR_EXIT("socketpair");
    }

    pid_t pid；
    pid = fork();
    if (pid == -1)
        ERR_EXIT("fork");

    if (pid > 0)
    {
        //父进程使用fd[0],既可以接收，也可以发送
        int val = 0;
        close(fd[1]);
        while (1)
        {
            ++val;//父进程++
            printf("sending data: %d\n", val);
            write(fd[0], &val, sizeof val);//父子进程通信，不需要转换成网络字节序
            read(fd[0], &val, sizeof val);
            printf("recver data: %d\n", val);
            sleep(1);
        }
    }
    else if (pid == 0)
    {
        close(fd[0]);
        int val;
        while (1)
        {
            read(fd[1], &val, sizeof val);
            val++;//子进程++操作
            write(fd[1], &val, sizeof val);
        }
    }

    return 0;
}