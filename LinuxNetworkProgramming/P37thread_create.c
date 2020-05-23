//
// Created by wangji on 19-8-14.
//

// p37 poxis 线程(一)

#include <iostream>
#include <stdio.h>
#include <cstring>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

using namespace std;


#define ERR_EXIT(m) \
        do  \
        {   \
            perror(m);  \ //perror检查的是全局的error变量
            exit(EXIT_FAILURE); \
        } while(0);

void * start_routine (void *arg)
{
    pthread_detach(pthread_self());
    for (int i = 0; i < 20; ++i)
    {
        printf( "B");
        fflush(stdout);
        usleep(20);
        if(i == 3)
            pthread_exit("ABC");//pthread_exit退出
    }
    sleep(3);
    //return (char *)"hello";//整个代码执行完毕退出
}

int main(int argc, char** argv) {
    int ret;
    pthread_t tid;

    ret = pthread_create(&tid, NULL, start_routine, NULL);//线程id由tid返回
    if (ret != 0)
    {
        fprintf(stderr,"pthread_creat:%s\n",strerror(ret));//错误码通过函数返回
        ERR_EXIT("pthread_create");
    }

    //主线程结束，整个进程就会结束
    for (int i = 0; i < 20; ++i)
    {
        printf( "A");
        fflush(stdout);
        usleep(20);
    }

     void *retval;
    // if (pthread_join(tid, &retval) != 0)
    // {
    //     ERR_EXIT("pthread_join");
    // }

    //return (char *)"hello"中的hello也会传递到retval
    if ((ret = pthread_join(tid, &retval)) != 0 )//等待新创建的线程结束,将pthread_exit中的ABC传递到retval，类似waitpid
    {
        fprintf(stderr,"pthread_join:%s\n",strerror(ret));//错误码通过函数返回
        ERR_EXIT("pthread_create");
    } 
    printf("return msg=%s\n", (char *)retval);

    return 0;
}