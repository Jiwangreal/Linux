//
// Created by jxq on 19-8-13.
//

// p33 system v共享内存与信号量综合

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>

#include <iostream>
#include <string>
#include <cstring>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#include "shmfifo.h"

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

shmfifo_t* shmfifo_init(int key, int blocksize, int blocks)
{
    shmfifo_t* shmfifo1 = (shmfifo_t*) malloc(sizeof(shmfifo_t));
    assert(shmfifo1 != NULL);//assert断言不为空，为空就崩溃

    memset(shmfifo1, 0, sizeof(shmfifo_t));
    int shmid = shmget(key, 0, 0);//打开共享内存
    
    int size = sizeof(shmhead_t) + blocks * blocksize;//blocks * blocksize：每块大小乘以总块数
    if (shmid == -1)//打不开共享内存，则创建
    {
        // 创建共享内存
        shmfifo1->shmid = shmget((key_t)key, size,  0666 | IPC_CREAT);
        if (shmfifo1->shmid == -1)
        {
            ERR_EXIT("shmget");
        }

        //初始化共享内存的头部指针
        shmfifo1->p_shm= (shmhead_t *)shmat(shmfifo1->shmid , NULL, 0);//NULL：表示内核自动选择，0:这段共享内存段既可读也可写
        if (shmfifo1->p_shm == (void *)-1)
        {
            ERR_EXIT("shmat");
        }
   
        //system v共享内存的key可以和信号量的key可以一样的，但是2个共享内存或者2个信号量的key不能是一样的
        // 创建信号量并赋值
        shmfifo1->sem_mutex = sem_creat(key+1);//shmfifo1->sem_mutex = sem_creat(key);
        shmfifo1->sem_full = sem_creat(key+2);//shmfifo1->sem_mutex = sem_creat(key+1);
        shmfifo1->sem_empty = sem_creat(key+3);//shmfifo1->sem_mutex = sem_creat(key+2);

        sem_setval(shmfifo1->sem_mutex, 1);
        sem_setval(shmfifo1->sem_full, blocks);//初始化仓库的大小
        sem_setval(shmfifo1->sem_empty, 0);//可以消费的产品个数为0

        shmfifo1->p_payloadd = (char*)(shmfifo1->p_shm + 1);//有效负载的位置


        // 初始化其他成员变量
        memset(shmfifo1->p_shm, 0, sizeof(shmhead_t));
        shmfifo1->p_shm->blocks = blocks;
        shmfifo1->p_shm->blocksize = blocksize;
        shmfifo1->p_shm->rd_index = 0;
        shmfifo1->p_shm->wr_index = 0;
        
    }
    else//打开共享内存，则连接
    {
        shmfifo1->shmid = shmid;
        shmfifo1->p_shm=  (shmhead_t *)shmat(shmfifo1->shmid, NULL, 0);
        if (shm == (void *)-1)
        {
            ERR_EXIT("shmat");
        }

        //因为共享内存存在，则信号量也存在
        //获取信号量
        shmfifo1->sem_mutex = sem_open(key + 1);
        shmfifo1->sem_full = sem_open(key + 2);
        shmfifo1->sem_empty = sem_open(key + 3);
        shmfifo1->p_payloadd = (char*)(shmfifo1->p_shm + 1);
    }
    return shmfifo1;
}

//往共享内存存放数据
void shmfifo_put(shmfifo_t* fifo, const void *buf)
{
    sem_p(fifo->sem_full);
    sem_p(fifo->sem_mutex);

    //生产产品，要生产的位置=
    memcpy(fifo->p_payloadd+fifo->p_shm->blocksize*fifo->p_shm->wr_index, 
            buf, fifo->p_shm->blocksize);

    //下一次要生产的位置要发生改变，若加到了队列末尾，需要循环到头部开始(环形缓冲区)，所以模%上总块数
    fifo->p_shm->wr_index = (fifo->p_shm->wr_index + 1) % fifo->p_shm->blocks;
    
    sem_v(fifo->sem_mutex);
    sem_v(fifo->sem_empty);
}


void shmfifo_get(shmfifo_t* fifo, void* buf)
{
    sem_p(fifo->sem_empty);
    sem_p(fifo->sem_mutex);

    //将共享内存的数据拷贝到缓冲区当中
    memcpy(buf, fifo->p_payloadd+fifo->p_shm->blocksize*fifo->p_shm->rd_index,  
            fifo->p_shm->blocksize);

    fifo->p_shm->rd_index = (fifo->p_shm->rd_index + 1) % fifo->p_shm->blocks;
    sem_v(fifo->sem_mutex);
    sem_v(fifo->sem_full);
}
void shmfifo_destroy(shmfifo_t* fifo)
{
    sem_d(fifo->sem_mutex);
    sem_d(fifo->sem_empty);
    sem_d(fifo->sem_full);

    shmdt(fifo->p_shm);
    shmctl(fifo->shmid, IPC_RMID, 0);
    free(fifo);
}