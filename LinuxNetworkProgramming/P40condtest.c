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

#define CONSUMERS_COUNT 2
#define PRODUCERS_COUNT 1
// #define BUFFSIZE 10//缓冲区是无限大的，不需要缓冲区

pthread_mutex_t g_mutex;
pthread_cond_t g_cond;

pthread_t g_thread[CONSUMERS_COUNT + PRODUCERS_COUNT];

int ready = 0;//表示当前缓冲区没有产品

void *produce (void *arg)
{
    int num = *((int *)arg);
    int i;
    while (1)
    {
        //在生产时，并没有判定缓冲区是不是满的，假定缓冲区是无线大的，可以一直生产产品，利用互斥量+条件变量
        pthread_mutex_lock(&g_mutex);
        printf("%d produce begin produce product %d\n", num);
        ++ready;
        printf("%d produce end produce product %d\n", num);
        if (ready > 0)
            pthread_cond_signal(&g_cond);
        printf("%d signal ...\n", num);//某个线程向等待中的消费者线程发起通知
        
        pthread_mutex_unlock(&g_mutex);
        sleep(5);
    }
    return NULL;
}

void *consume (void *arg)
{
    int num = *((int *)arg);
    int i;
    while (1)
    {
        
        pthread_mutex_lock(&g_mutex);
       
        //当等待条件的时候，需要对g_cond进行解锁操作，以便其它线程可以进入临界区，修改ready的值
        /*
        当消费速度不如生产速度时，消费者等待的频率就变少了，下面的while循环就不被运行了
         当消费速度大于生产速度时，就会一直进入等待条件的状态，直到生产者线程生产一个产品，向消费者发起了一个通知
        */
        while ( ready == 0 )//不用if (ready == 0 )的原因如下
        {
            printf("%d begin wait a contion...\n", num);//输出等待条件的状态
            pthread_cond_wait(&g_cond, &g_thread);
        }
        printf("%d end wait a contion...\n", num);//输出等待条件的状态
        printf("%d consume begin consume product %d\n", num);
        --ready;//消费产品
        printf("%d consume end consume product %d\n", num);
        
        pthread_mutex_unlock(&g_mutex);
        sleep(1);
    }
    return NULL;
}

int main(int argc, char** argv)
{

    pthread_mutex_init(&g_mutex, NULL);
    pthread_cond_init(&g_cond, NULL);

    int i;
    for (i = 0; i < CONSUMERS_COUNT; ++i)
    {
        pthread_create(&g_thread[i], NULL, consume, &i);
    }
    sleep(1);
    for (i = 0; i < PRODUCERS_COUNT; ++i)
    {
        pthread_create(&g_thread[CONSUMERS_COUNT+i], NULL, produce,  &i);
    }

    for (i = 0; i < CONSUMERS_COUNT + PRODUCERS_COUNT; ++i)
    {
        pthread_join(g_thread[i], NULL);
    }

    pthread_mutex_destroy(&g_mutex);
    pthread_cond_destroy(&g_cond);

    return 0;
}
/*重要！！
pthread_cond_wait操作的原语：
（1）对g_mutex进程解锁
原因：可以让其它线程进入临界区修改ready，若不解锁，其它线程则无法进入临界区，会出现死锁；
多个线程可以等待同一个条件(消费者线程可以有多个)
（2）等待条件，直到有线程向他发起通知
（3）当pthread_cond_wait返回时，需要重新对g_mutex进行加锁操作



pthread_cond_signal
（1）向第一个等待条件的线程发送通知，如果没有任何一个心城处理等待条件的状态，这个通知将被忽略
没关系，因为下一次进入临界区时，判断条件已经满足了，将不处于等待的状态



pthread_cond_broadcast
向所有等待线程发起通知



不用if (ready == 0 )的原因
pthread_cond_wait会自动重启，就像该信号没有发生过一样
pthread_cond_wait可能会被虚假唤醒，ready变量可能没有改变
*/
