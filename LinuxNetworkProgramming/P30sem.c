//
// Created by wangji on 19-8-13.
//

// p30 system v信号量（一）

#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>


#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

// https://blog.csdn.net/a1414345/article/details/64513946

#define ERR_EXIT(m) \
        do \
        { \
             perror(m); \
             exit(EXIT_FAILURE);    \
        } while (0);

//来自man semctl
union semun {
    int              val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO
                                           (Linux-specific) */
};

// 仅仅创建信号量集
int sem_creat(key_t key)
{
    int semid;
    semid = semget(key, 1, IPC_CREAT | IPC_EXCL | 0666);//IPC_EXCL表示只能创建1次
    if (semid == -1)
    {
        ERR_EXIT("semget");
    }
    return semid;//返回信号量集的id
}

//仅仅打开一个创建的信号量集
int sem_open(key_t key)
{
    int semid;
    semid = semget(key, 0, 0);//由于信号量集已经存在了，可以不指定信号量的个数；打开选项也可以不必关心
    if (semid == -1)
    {
        ERR_EXIT("semget");
    }
    return semid;
}

// 设置信号量中信号量的值
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

// 获取信号量中信号量的值
int sem_getval(int semid)
{
    int ret;
    ret = semctl(semid, 0, GETVAL,0);//第4个参数忽略了，填写0
    if (ret == -1)
    {
        ERR_EXIT("getval")
    }
    printf("sem.val = %d\n", ret);
    return ret;
}

//只能删除信号量集，不能删除信号量集中的某个信号量
int sem_d(int semid)
{
    int ret;
    ret = semctl(semid, 0, IPC_RMID, NULL);//信号量个数不知道的话填写0
    if (ret == -1)
    {
        ERR_EXIT("rm_sem")
    }
    return 0;
}

// P操作
int sem_p(int semid)
{
    //struct sembuf sembuf={0,-1,0};
    struct sembuf sembuf;//man semop
    sembuf.sem_num = 0;//信号量集中第1个信号
    sembuf.sem_op = -1;//对信号量的计数减去1
    sembuf.sem_flg = 0;//IPC_NOWAIT,SEM_UNDO
    int ret;
    ret = semop(semid, &sembuf, 1);
    if (ret == -1)
    {
        ERR_EXIT("sem_p")
    }
    return ret;
}

// V操作
int sem_v(int semid)
{
    struct sembuf sembuf;
    sembuf.sem_num = 0;
    sembuf.sem_op = 1;
    sembuf.sem_flg = 0;//IPC_NOWAIT,SEM_UNDO
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
    struct semid_ds sem;//若设置IPC_STAT，获取的数据将保存在sem     struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    su.buf = &sem;//定义一个指针指向该数据结构
    int ret =semctl(semid, 0, IPC_STAT, su);
    if (ret == -1)
    {
        ERR_EXIT("semctl");
    }
    printf("current permissions is %o\n", su.buf->sem_perm.mode);//%o为8进制打印

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

void usage()
{
//    创建信号量集 -c
    fprintf(stderr, "semtool -c\n");
//    删除信号量集	  -d
    fprintf(stderr, "semtool -d\n");
//    获得信号量集中信号量的计数值    -g
    fprintf(stderr, "semtool -g\n");
//    对信号量集中的信号量设置初始计数值    -s <val>
    fprintf(stderr, "semtool -s <val>\n");
//    p操作	 -p
    fprintf(stderr, "semtool -p\n");
//    v操作     -v
    fprintf(stderr, "semtool -v\n");
//    查看信号量集的权限   -f
    fprintf(stderr, "semtool -f\n");
//    更改权限   -m <mode>
    fprintf(stderr, "semtool -m <mode>\n");
}

int main(int argc, char** argv)
{
    int opt;
    /*
    int semid;
    semid=sem_creat(1234);
    sleep(5);//过5s删除该信号量集
    sem_d(semid);
    */
    opt = getopt(argc, argv, "cdgpvs:fm:");//解析参数，执行./semtool s 2因为s后面要跟参数，所以加：
    if (opt == '?')
    {
        exit(EXIT_FAILURE);
    }
    if (opt == -1)
    {
       usage();
       exit(EXIT_FAILURE);
    }

    // 为了获取一个独一无二的通信对象，必须使用键（可使用ftok()函数生成，返回值key）。
    // 这里的键是用来定位I P C 对象的标识符的，msqget，shmget
    key_t key = ftok(".", 's');//(必须是存在的路径，低序的8位不能为0)，这里s是8位的，一个字节，低8位不为0
    int semid;
    switch (opt)
    {
        case 'c':
            sem_creat(key);
            break;
        case 'p':
            semid = sem_open(key);
            sem_p(semid);
            sem_getval(semid);
            break;
        case 'v':
            semid = sem_open(key);
            sem_v(semid);
            sem_getval(semid);
            break;
        case 'd':
            semid = sem_open(key);
            sem_d(semid);
            break;
        case 's':
            semid = sem_open(key);
            sem_setval(semid, atoi(optarg));//-s后面的参数可以通过optarg来获取
            break;
        case 'g':
            semid = sem_open(key);
            sem_getval(semid);
            break;
        case 'f':
            semid = sem_open(key);
            sem_getmode(semid);
            break;
        case 'm':
            semid = sem_open(key);
            sem_setmode(semid, argv[2]);
            break;
    }
    return 0;
}