//
// Created by wangji on 19-8-13.
//

// p30 system v信号量（二）

#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/wait.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <wait.h>

#define ERR_EXIT(m) \
        do \
        { \
             perror(m); \
             exit(EXIT_FAILURE);    \
        } while (0);

union semun {
    int              val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO
                                           (Linux-specific) */
};

// -c
int sem_creat(key_t key)
{
    int semid;
    semid = semget(key, 1, IPC_CREAT | IPC_EXCL | 0666);
    if (semid == -1)
    {
        ERR_EXIT("semget");
    }
    return semid;
}


int sem_open(key_t key)
{
    int semid;
    semid = semget(key, 0, 0);
    if (semid == -1)
    {
        ERR_EXIT("semget");
    }
    return semid;
}

// -s <val>
int sem_setval(int semid, int val)
{
    union semun su;
    su.val = val;
    int ret;
    ret = semctl(semid, 0, SETVAL, su);
    if (ret == -1)
    {
        ERR_EXIT("setval")
    }
    return 0;
}

// -g
int sem_getval(int semid)
{
    int ret;
    ret = semctl(semid, 0, GETVAL);
    if (ret == -1)
    {
        ERR_EXIT("getval")
    }
    printf("sem.val = %d\n", ret);
    return ret;
}

// -d
int sem_d(int semid)
{
    int ret;
    ret = semctl(semid, 0, IPC_RMID, NULL);
    if (ret == -1)
    {
        ERR_EXIT("rm_sem")
    }
    return 0;
}

// -p
int sem_p(int semid)
{
    struct sembuf sembuf;
    sembuf.sem_num = 0;
    sembuf.sem_op = -1;
    sembuf.sem_flg = 0;
    int ret;
    ret = semop(semid, &sembuf, 1);
    if (ret == -1)
    {
        ERR_EXIT("sem_p")
    }
    return ret;
}

// -v
int sem_v(int semid)
{
    struct sembuf sembuf;
    sembuf.sem_num = 0;
    sembuf.sem_op = 1;
    sembuf.sem_flg = 0;
    int ret;
    ret = semop(semid, &sembuf, 1);
    if (ret == -1)
    {
        ERR_EXIT("sem_v")
    }
    return ret;
}

// -f
int sem_getmode(int semid)
{
    union semun su;
    struct semid_ds sem;
    su.buf = &sem;
    int ret =semctl(semid, 0, IPC_STAT, su);
    if (ret == -1)
    {
        ERR_EXIT("semctl");
    }
    printf("current permissions is %o\n", su.buf->sem_perm.mode);

    return ret;
}


// -m <mode>
int sem_setmode(int semid, char* mode)
{
    union semun su;
    struct semid_ds sem;
    su.buf = &sem;

    int ret =semctl(semid, 0, IPC_STAT, su);
    if (ret == -1)
    {
        ERR_EXIT("semctl");
    }
    printf("current permissions is %o\n", su.buf->sem_perm.mode);

    sscanf(mode, "%o", (unsigned int*)&su.buf->sem_perm.mode);
    ret = semctl(semid, 0, IPC_SET, su);
    if (ret == -1)
    {
        ERR_EXIT("semctl");
    }

    printf("permission updated..\n");
    return ret;
}

void print(int sigmid, char* s)
{
    int i = 0;
    int pause_time;
    srand(getpid());//以当前进程最为随机数的发生种子
    for (int i = 0; i < 10; ++i)
    {
        sem_p(sigmid);//互斥操作

        //临界区开始,同时只有一个进程能够使用它
        printf("%s", s);
        fflush(stdout);//立刻输出到标准输出上来，清空缓冲区
        pause_time = rand() % 3;//去个随机数0，1，2，不会超处3s
        sleep(pause_time);//即使该进程属于slepp状态，也不能被其它进程所使用
        printf("%s", s);
        fflush(stdout);
        //临界区结束：

        sem_v(sigmid);//互斥操作

        pause_time = rand() % 2;
        sleep(pause_time);
    }
}





int main(int argc, char** argv)
{
    int sigmid = sem_creat(IPC_PRIVATE);//父子进程间互斥可以用IPC_PRIVATE，私有的信号量集
    sem_setval(sigmid, 0);//由于信号量集初始值=0，那么任何一个进程都不能进入到临界区，p操作的时候就以及阻塞了
    pid_t pid = fork();

    if (pid == -1)
    {
        ERR_EXIT("fork");
    }

    if (pid > 0)
    {
        sem_setval(sigmid, 1);//父进程就能够进入临界区打印O了
        print(sigmid, "o");//父进程睡眠几秒钟，时间片轮转给其它进程，子进程也无法进入临界区
        wait(nNULL);//父进程等待子进程退出
        sem_d(sigmid);
    }
    else
    {
        print(sigmid, "x");
    }
    return 0;
}