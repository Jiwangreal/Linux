//
// Created by wangji on 19-8-14.
//

// p37 poxis 线程(二)

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

using namespace std;


#define ERR_EXIT(m) \
        do  \
        {   \
            perror(m);  \
            exit(EXIT_FAILURE); \
        } while(0);

struct tsd
{
    pthread_t id;
    char* arg;
}tsd_t;

pthread_key_t thread_key;//全局的TSD数据key

pthread_once_t once = PTHREAD_ONCE_INIT;

void destr_function(void *)
{
    printf("destroy...\n");
    free(value);
}

void once_run(void)
{
    pthread_key_create(&thread_key, destr_function);
    // cout<<"once_run in thread "<<(unsigned int )pthread_self()<<endl;
    printf("key init ... \n");
}

void * start_routine (void *arg)
{
    // pthread_once(&once, once_run);//只在第一个第一个线程进入时执行once_run,意味着key只创建1次
    tsd_t *value = (tsd_t*)malloc(sizeof(tsd_t));
    value->arg = (char *)arg;
    value->id = pthread_self();tsd_t

    //给线程设定特定数据
    pthread_setspecific(thread_key, value);
    printf("%s setspecific %p\n", (char*)arg, value);

    value = (struct tsd_t*)pthread_getspecific(thread_key);
    printf("tid = 0x%x str = %s\n", (int)value->id, value->arg);//打印线程ID和str
    sleep(2);
    value = (struct tsd_t*)pthread_getspecific(thread_key);
    printf("tid = 0x%x str = %s\n", (int)value->id, value->arg);

    return NULL;
}



int main(int argc, char** argv) {

    //thread_key：key是每个线程的全局变量，但是key指向的变量是每个线程所独享的
    //每个线程都有thread_key变量
    //destr_function主要是用来销毁key所指向的value的数据的
    //当下面2个线程退出时，都会执行destr_function销毁数据，即执行2次
    pthread_key_create(&thread_key, destr_function);

    pthread_t thread1;
    pthread_t thread2;

    pthread_create(&thread1, NULL, start_routine, (char *)"thread1");
    pthread_create(&thread2, NULL, start_routine, (char *)"thread2");


    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    //当两个线程退出时，删除key
    pthread_key_delete(thread_key);
    return 0;
}