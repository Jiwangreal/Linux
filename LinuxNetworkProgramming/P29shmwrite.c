//
// Created by wanji on 19-8-13.
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

//共享内存的大小是36个字节，可以从ipcs看出
struct student
{
    char name[32];
    int age;
}STU;

int main(int argc, char** argv)
{
    int shmid;  // 共享内存标识符

    // 创建共享内存
    shmid = shmget((key_t)1234, sizeof(STU), 0666 | IPC_CREAT);
    if (shmid == -1)
    {
        ERR_EXIT("shmget");
    }

    // 第一次创建完共享内存时，它还不能被任何进程访问，shmat()函数的作用就是用来启动对该共享内存的访问，并把共享内存连接到当前进程的地址空间
    // 将共享内存链接到当前进程的地址空间
    STU *p;
    p = shmat(shmid, NULL, 0);//对共享内存的访问相当于对指针的访问
    if (p == (void *)-1)
    {
        ERR_EXIT("shmat");
    }

    // 对指针的访问可以按照普通的访问即可，就相当于对共享内存操作
    //student *shared = (struct student*) shm;
    strcpy(p->name, "hello");
    p->age = 20;

    while (1)
    {
        if (memcmp(p->name, "quit", 4) == 0)//内存比较
        {
            break;
        }
    }
    
    // 把共享内存从当前进程中分离，解除映射
    if (shmdt(p) == -1)
    {
        ERR_EXIT("shmdt");
    }


    //删除共享内存
    shmctl(shmid, IPC_RMID, NULL);
    return 0;
}
