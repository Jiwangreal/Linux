#include <stdio.h>
#include <errno.h>
#include <string.h>

// written by wangji
#define ERR_EXIT(m) \
        do  \
        {   \
            perror(m);  \
            exit(EXIT_FAILURE); \
        } while(0);



int main(void)
{
    pthread_attr_t attr;
    pthread_attr_init(&attr);//默认，拥有很多种属性的默认值

    int state;
    pthread_attr_getdetachstate(&attr,&state);//线程的分离属性的默认值保存在state
    if (state == PTHREAD_CREAT_JOINABLE)
        printf("detachstate:PTHREAD_CREAT_JOINABLE");
    else if (state == PTHREAD_CREAT_DETACHED)
        printf("detachstate:PTHREAD_CREAT_DETACHED");

    size_t size;
    pthread_attr_getstachsize(&attr, &size);
    printf("stacksize:%d\n", size);

    pthread_attr_getguardsize(&attr, &size);
    printf("guardsize:size\n",size);


    int scope;
    pthread_attr_getscope(&atte, &scope);
    if (scope == PTHREAD_SCOPE_PROCESS)//线程的竞争范围在进程内
        printf("scope:PTHREAD_SCOPE_PROCESS\n");
    if (  scope == PTHREAD_SCOPE_SYSTEM )//线程的竞争范围在系统范围内
        printf("scope:PTHREAD_SCOPE_SYSTEM\n");

    //线程的调度策略
    int policy;
    pthread_attr_getschedpolicy(&attr,&policy);
    if ( policy == SCHED_FIFO)
        printf("policy:SCHED_FIFO\n");//若线程优先级相同，按照陷入先出的方式调度
    else if ( policy == SCHED_RR)
        printf("policy:SCHED_RR\n");//即使线程优先级相同，抢占式调度
    else if ( policy == SCHED_OTHER)
        printf("policy:SCHED_OTHER\n");

    int inheritsched;
    pthread_attr_getinheritsched(&attr, &inheritsched);
    if ( inheritsched == PTHREAD_INHERIT_SCHED)//新创建的线程将继承调用者线程的调度策略属性
        printf("inheritsched:PTHREAD_INHERIT_SCHED\n");
    else if ( inheritsched == PTHREAD_EXPLICIT_SCHED )//表示新创建的线程属性需要自己设置，由setschedpolicy设置
        printf("inheritsched:PTHREAD_EXPLICIT_SCHED\n");

    struct sched_param param;
    pthread_attr_getschedparam(&attr, &param);
    printf("sched priority:%d\n", param.sched_priority);

    pthread_attr_destroy(&attr);

    int level;
    level = pthread_getconcurrency();//并发级别
    printf("level:%d\n", level);


    return 0;
}













