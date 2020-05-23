//
// Created by wangji on 19-8-15.
//

// p41 线程池

#include "condition.h"
#include "threadpool.h"
#include <iostream>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

void *run (void *arg)
{
    printf("threadpool 0x%x working task %d\n", (int)pthread_self(), *(int*)(arg));
    sleep(1);
    free(arg);
    return NULL;
}

int main(void)
{
    // 结构体指针需要初始化 不晓得为啥报错
    threadpool_t pool;
    threadpool_init(&pool, 3);//初始化3个线程

    int i;
    for (i = 0; i < 10; ++i)//向线程池，添加10个任务
    {
        int *a = (int*)malloc(sizeof(int));
        *a = i;//传递动态内存，不会使得内部的某个指针指向i变量，因为for循环后i发生了改变，不要写成：threadpool_add_task(&pool, run, &i);
        threadpool_add_task(&pool, run, a);
    }
   // sleep(15);
    threadpool_destroy(&pool);//调用threadpool_destroy的主线程也要得到通知
    return 0;
}