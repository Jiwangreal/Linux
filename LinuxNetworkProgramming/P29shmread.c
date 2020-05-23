//
// Created by wangji on 19-8-13.
//

// p29 system v共享内存

#include <sys/ipc.h>
#include <sys/shm.h>
#include <iostream>
#include <string>
#include <errno.h>
#include <string.h>

using namespace std;

#define ERR_EXIT(m) \
        do \
        {   \
            perror(m);  \
            exit(EXIT_FAILURE); \
        } while (0);

struct student
{
    char name[32];
    int age;
}STU;

int main(int argc, char** argv)
{
    int shmid;  // 共享内存标识符

    // 打开共享内存
    shmid = shmget((key_t)1234, 0, 0);//shmget((key_t)1234, sizeof(STU), 0)这样写也行
    if (shmid == -1)
    {
        ERR_EXIT("shmget");
    }

    STU *p;
    p = shmat(shmid, NULL, 0);//将共享内存映射到进程的地址空间
    if (p == (void *)-1)
    {
        ERR_EXIT("shmat");
    }

    //将共享内存的内容print出来
    //student *shared = (struct student*) shm;
    printf("student name: %s, age: %d\n", p->name, p->age);

    memcpy(p, "quit", 4);

    if (shmdt(p) == -1)
    {
        ERR_EXIT("shmdt");
    }

    return 0;
}
