//
// Created by wangji on 19-8-15.
//

// p39 posix信号量与互斥锁

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <semaphore.h>
#include <pthread.h>

using namespace std;

#define ERR_EXIT(m) \
        do  \
        {   \
            perror(m);  \
            exit(EXIT_FAILURE); \
        } while(0);

#define CONSUMERS_COUNT 1   //消费者个数
#define PRODUCERS_COUNT 5  //生产者个数
// #define PRODUCERS_COUNT 1 

#define BUFFSIZE 10

int g_buffer[BUFFSIZE];//环形缓冲区

unsigned short in = 0;//从0开始放
unsigned short out = 0;//从0开始取
unsigned short produce_id = 0;
unsigned short consume_id = 0;

sem_t g_sem_full;
sem_t g_sem_empth;
pthread_mutex_t g_mutex;

pthread_t g_thread[CONSUMERS_COUNT + PRODUCERS_COUNT];//线程个数

void *produce (void *arg)
{
    int num = (int)arg;
    int i;
    while (1)
    {
        //等待缓冲区不满，就可以生产
        printf("%d wait buffer not full!\n", num);
        sem_wait(&g_sem_full);//等待一个g_sem_full满的信号量，不满，则生产产品，类似p操作
        pthread_mutex_lock(&g_mutex);

        //生产之前，打印仓库状态
        for (i = 0; i < BUFFSIZE; ++i)
        {
            printf("%02d ", i);//仓库序号
            printf("%d",g_buffer[i]);//仓库当前产品id
            if (g_buffer[i] == -1)
            {
                printf("%s ", "null");
            }
            else
            {
                printf("%d ", g_buffer[i]);
            }
            if (i == in)
            {
                printf("\t<--produce");
            }
            printf("\n");
        }

        printf("%d produce begin produce product %d\n", num, produce_id);//num：哪个线程在生产
        g_buffer[in] = produce_id;
        cout << g_buffer[in] << endl;
        in = (in + 1) % BUFFSIZE;//因为是环形缓冲区
        printf("%d produce end produce product %d\n", num, produce_id++);//下一个产品id++
        pthread_mutex_unlock(&g_mutex);
        sem_post(&g_sem_empth);//类似v操作
        sleep(5);//生产者生产一个产品要5s，相比较于消费者消费一个产品要1s
        //生产者速度满，消费快，那么生产者就有可能会阻塞
    }
    return NULL;
}

void *consume (void *arg)
{
    

    int num = (int)arg;//这里直接是值传递进来的，不是传递指针地址，不要写成int num = *((int *)arg);
    int i;
    while (1)
    {
        //等待缓冲区不空，就可以消费
        printf("%d wait buffer not empty\n", num);
        sem_wait(&g_sem_empth);//类似p操作，等待一个g_sem_empty的信号量，不空，则可以消费产品
        pthread_mutex_lock(&g_mutex);

        //消费之前，打印仓库状态
        for (i = 0; i < BUFFSIZE; ++i)
        {
            printf("%02d ", i);
            if (g_buffer[i] == -1)
            {
                printf("%s", "null");
            }
            else
            {
                printf("%d", g_buffer[i]);
            }
            if (i == out)
            {
                printf("\t<--consume");
            }
            printf("\n");
        }
        consume_id = g_buffer[out];
        printf("%d consume begin consume product %d\n", num, consume_id);
        g_buffer[out] = -1;
        out = (out + 1) % BUFFSIZE;//下一个消费的位置++
        printf("%d consume end consume product %d\n", num, consume_id);
        pthread_mutex_unlock(&g_mutex);
        sem_post(&g_sem_full);//类似v操作
        sleep(1);//消费者消费一个产品要1s
    }
    return NULL;
}

int main(int argc, char** argv)
{
    //初始化仓库
    for (int i = 0; i < BUFFSIZE; ++i)
    {
        g_buffer[i] = -1;
    }

    //g_sem_full只用于当前进程的不同线程间的通信，前提是处于全局数据区
    //g_sem_full信号量的初始值为BUFFSIZE
    //g_sem_empth信号量的初始值为0，表示当前没有产品可以消费
    sem_init(&g_sem_full, NULL, BUFFSIZE);
    sem_init(&g_sem_empth, NULL, 0);

    //g_mutex只能在同一个进程的不同线程间进行通信，还取决于其属性attr，默认为NULL
    pthread_mutex_init(&g_mutex, NULL);

    int i;
    for (i = 0; i < CONSUMERS_COUNT; ++i)
    {
        pthread_create(&g_thread[i], NULL, consume, (void *)i);
    }

    for (i = 0; i < PRODUCERS_COUNT; ++i)
    {
        pthread_create(&g_thread[CONSUMERS_COUNT+i], NULL, produce,  (void *)i);
    }

    //等待生产者和消费者线程退出
    for (i = 0; i < CONSUMERS_COUNT + PRODUCERS_COUNT; ++i)
    {
        pthread_join(g_thread[i], NULL);
    }

    sem_destroy(&g_sem_full);
    sem_destroy(&g_sem_empth);
    pthread_mutex_destroy(&g_mutex);

    return 0;
}