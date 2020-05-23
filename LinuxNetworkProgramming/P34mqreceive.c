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
}STU;

int main(int argc, char** argv)
{
    // mq_overview
    mqd_t mqid = mq_open("/abc",  O_RDONLY);
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

    size_t size = attr.mq_msgsize;
    STU stu;
    unsigned prio;

    //len应该是消息队列中每条消息长度的最大值
    if (mq_receive(mqid, (char*)&stu, size, prio) == (mqid)-1)//mq_receive(mqid, (char*)&stu, size, NULL)
    {
        ERR_EXIT("mq_receive");
    }

    printf("student name = %s, age = %d\n, prio=%u", stu.name, stu.age,prio);
    // printf("student name = %s, age = %d\n", stu.name, stu.age);
    mq_close(mqid);

    return 0;
}