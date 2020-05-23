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
    msgid = msgget(1234, 0);
    if (msgid == -1)
    {
        ERR_EXIT("msgget");
    }

    printf("msgget succ\n");
    printf("msgid = %d\n", msgid);

    struct msqid_ds buf;
    msgctl(msgid, IPC_STAT, &buf);
    printf("mode = %o\n", buf.msg_perm.mode);//消息队列的权限
    printf("bytes = %ld\n", buf.__msg_cbytes);//当前消息队列的字节数
    printf("number=%d\n", (int)buf.msg_qnum);//消息队列的消息个数
    printf("msgmnb=%d\n", (int)buf.msg_qbytes);//消息队列容纳的字节总数

    return 0;
}