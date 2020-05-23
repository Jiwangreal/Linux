//
// Created by wangji on 19-8-13.
//

// p30 system v信号量（二）

#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>


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

int semid;
//睡眠的时间为1-5s
#define DELAY (rand() % 5 + 1 )

//模拟死锁
int wait_1fork(int no)
{
    struct sembuf sb={no, -1 ,0};
    int ret;
    ret=semop(semid, &sb, 1);
    if (ret == -1)
        ERR_EXIT("semop");
    return ret;
}



void get2fork(int pid)
{
    unsigned short int left = pid;//左边的刀叉编号=哲学家的id编号
    unsigned short int right = (pid + 1) % 5;//右边的刀叉编号=哲学家id编号+1，以此，得出来的经验
    struct sembuf sembuf[2] = {{left, -1, 0}, {right, -1, 0}};//刀叉的编号就是信号的序号
    int ret = semop(semid, sembuf, 2);//对信号集中的2个信号量进行操作，要么同时得到2个叉子，要么都得不到，就处于等待状态，等待别的哲学家归还叉子
    if (ret == -1)
    {
        ERR_EXIT("get2fork")
    }
};

void put2fork(int pid)
{
    unsigned short int left = pid;
    unsigned short int right = (pid + 1) % 5;
    struct sembuf sembuf[2] = {{left, 1, 0}, {right, 1, 0}};
    int ret = semop(semid, sembuf, 2);
    if (ret == -1)
    {
        ERR_EXIT("get2fork")
    }
};


void philosopher(int no)
{
    srand(getpid());//用了rand()随机数，这里给随机数一个种子
    while (1)
    {
        printf("%d is thinking\n", no);//表示哲学家思考
        sleep(DELAY);//哲学家思考一段时间,哲学家饿了
        printf("%d is hungry\n", no);
        get2fork(no);//获取2个叉子
        printf("%d is eating\n", no);//吃饭
        sleep(DELAY);//吃完饭了
        put2fork(no);//释放2个叉子




       /*模拟死锁
        unsigned short int left = pid;
        unsigned short int right = (pid + 1) % 5;
        printf("%d is thinking\n", no);//表示哲学家思考
        sleep(DELAY);//哲学家思考一段时间,哲学家饿了
        printf("%d is hungry\n", no);
        wait_1fork(left); //哲学家看到刀叉就拿起来
        sleep(DELAY);//为了更快的产生死锁
        wait_1fork(right); //哲学家看到刀叉就拿起来
        printf("%d is eating\n", no);//吃饭
        sleep(DELAY);//吃完饭了
        put2fork(no);//释放2个叉子
        */
    }
}


int main(int argc, char** argv)
{
    semid = semget(IPC_PRIVATE, 5, IPC_CREAT | IPC_EXCL | 0666);//创建的信号量集中有5个信号量，模拟5个叉子
    if (semid == -1)
    {
        ERR_EXIT("semid");
    }

    union semun su;
    su.val = 1;
    for (int i = 0; i < 5; ++i)
    {
        semctl(semid, i, SETVAL, su);//初始化为1表示都是可用的状态
    }

    int no = 0;
    pid_t pid;
    for (int i = 1; i < 5; ++i)//创建4个子进程
    {
        pid = fork();
        if (pid == -1)
            ERR_EXIT("fork");

        if (pid == 0)
        {
            no = i;
            break;
        }
    }

    philosopher(no);
    return 0;
}