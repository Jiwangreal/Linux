//
// Created by wangji on 19-8-12.
//

// p25 system v消息队列(一)

#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#define ERR_EXIT(m) \
        do \
        { \
             perror(m); \
             exit(EXIT_FAILURE);    \
        } while (0);

int main(void)
{
    int msgid;
 

    // 当只有IPC_CREAT选项打开时,若已存在，则都返回该消息队列的ID，若不存在则创建消息队列，打开原有的消息队列
    //666：拥有者，主用户，其他用户具有读写权限
    msgid = msgget(1234, 0666 | IPC_CREAT);
    
    // 当IPC_CREAT | IPC_EXCL时, 如果没有该块消息队列，则创建，并返回消息队列ID。若已有该消息队列，则返回-1，输出：msgget:File exists
    //msgid = msgget(1234, 0666 | IPC_CREAT | IPC_EXCL);

    //IPC_PRIVATE所创建的消息队列，不能被其它进程所共享，可以用于本进程，以及父子进程
    //IPC_PRIVATE：执行一次本文件，就会创建一个消息队列，这些消息的队列key值为0，但是msqid不同
     //msgid = msgget(IPC_PRIVATE, 0666 | IPC_CREAT | IPC_EXCL);
    /*msgid = msgget(IPC_PRIVATE, 0666);
    msgid = msgget(1234, 0666 | IPC_CREAT);*/
    //msgid = msgget(1234, 0);    // flags = 0 表示按原来权限打开

    if (msgid == -1)
    {
        ERR_EXIT("msgget");
    }

    printf("msgget successful\n");
    printf("msgid = %d\n", msgid);

    msgctl(msgid, IPC_RMID,NULL);

    return 0;
}