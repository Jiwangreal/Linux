//
// Created by wangji on 19-8-12.
//

// p28 共享内存介绍（一）

#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>

#define ERR_EXIT(m) \
        do \
        { \
             perror(m); \
             exit(EXIT_FAILURE);    \
        } while (0);


typedef struct stu
{
    char name[4];
    int age;
} STU;

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int fd;
    // O_CREAT 以创建方式打开
    // O_RDWR  以读写方式打开
    // O_TRUNC  以清空方式打开
    //权限为666
    //man 2 open可以查看open使用哪些头文件
    fd = open(argv[1], O_CREAT | O_RDWR | O_TRUNC, 0666);
    if (fd == -1)
    {
        ERR_EXIT("open");
    }

    // SEEK_SET 参数 offset 即为新的读写位置
    //STU结构体是8个字节，定位到39个字节的位置，SEEK_SET从头开始定位
    lseek(fd, sizeof(STU) * 5 - 1, SEEK_SET);
    write(fd, "", 1);
    //上述总共是写入40个字节的文件

    //对文件的操作就像是对内存的访问
    STU *p;
    p = (STU*)mmap(NULL, sizeof(STU) * 5, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    //p = (STU*)mmap(NULL, sizeof(STU) * 10, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);//内存映射区实际上是超过80字节的
    if (p == NULL)
    {
        ERR_EXIT("mmap");
    }

    //对映射的内存区进行写入操作
    char ch = 'a';
    for (int i = 0; i < 6; ++i)//for (int i = 0; i < 10; ++i)
    {
        memcpy((p+i)->name, &ch, 1);//对内存的操作就是对文件的操作，因为文件已经映射到这块内存区了
        (p+i)->age = 20 + i;

        ++ch;
    }

    printf("initialize over\n");

    //sleep(10);
    //删除映射
    munmap(p, sizeof(STU)*5);//（地址，字节）

    //munmap(p, sizeof(STU)*10);//（地址，字节）
    printf("exit...\n");

    return 0;
}