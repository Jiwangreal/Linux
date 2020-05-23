//
// Created by wangji on 19-8-14.
//

// p35 poxis共享内存

#include <errno.h>
#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include <string.h>

#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <mqueue.h>
#include <sys/mman.h>

using namespace std;

#define ERR_EXIT(m) \
        do  \
        {   \
            perror(m);  \
            exit(EXIT_FAILURE); \
        } while (0);

struct student
{
    char name[32];
    int age;
};

int main(int argc, char** argv)
{

    if (shm_unlink("/xyz") == -1)
    {
        ERR_EXIT("shmunlink");
    }
    printf("shm_unlink succ\n");


    return 0;
}