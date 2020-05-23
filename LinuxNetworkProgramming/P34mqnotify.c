//
// Created by wangji on 19-8-14.
//

// p34 poxis消息队列

#include <errno.h>
#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include <string.h>

#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <mqueue.h>
#include<signal.h>

using namespace std;

#define ERR_EXIT(m) \
        do  \
        {   \
            perror(m);  \
            exit(EXIT_FAILURE); \
        } while (0);

struct student
{
    char name[32];
    int age;
};

size_t size;
mqd_t mqid;
struct sigevent sigv;


//提供函数的实现
void handler(int s)
{
    mq_notify(mqid, &sigv);//多次注册
    student stu;

    if (mq_receive(mqid, (char*)&stu, size, NULL) == -1)
    {
        ERR_EXIT("mq_receive");
    }

    printf("student name = %s, age = %d\n", stu.name, stu.age);
}

int main(int argc, char** argv)
{
    // mq_overview
    mqid = mq_open("/abc",  O_RDONLY);
    if (mqid == ((mqd_t) -1))
    {
        ERR_EXIT("mq_open");
    }

    //printf("mqopen success\n");
    struct mq_attr attr;
    if (mq_getattr(mqid, &attr) == -1)
    {
        ERR_EXIT("mq_getattr");
    }
    size = attr.mq_msgsize;//获取最大消息的大小保存在size

    //注册一个信号，安装一个信号
    signal(SIGUSR1, handler); //定义一个信号函数，当SIGALRM信号发过来时，执行handler函数

    sigv.sigev_notify = SIGEV_SIGNAL;
    sigv.sigev_signo = SIGUSR1;//SIGUSR1是十号信号

    //注册一个消息到达的通知
    mq_notify(mqid, &sigv);

    while (true)//要有死循环，否则通知到，就没收时间去接收他
    {
        pause();
    }

    mq_close(mqid);

    return 0;
}