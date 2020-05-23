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

int main(int argc, char** argv)
{
    int msgid;
    msgid = msgget(1234, 0);
    if (msgid == -1)
    {
        ERR_EXIT("msgget");
    }

    printf("msgget succ\n");
    printf("msgid = %d\n", msgid);

    //首先获取消息队列的权限到buf中
    struct msqid_ds buf;
    msgctl(msgid, IPC_STAT, &buf);

    //sscanf将字符串，以%o的形式保存在buf.msg_perm.mode中
    //scanf将标准输入，以%o的形式保存在buf.msg_perm.mode中
    sscanf("600", "%o", (unsigned int*)&buf.msg_perm.mode);

    //然后在用IPC_SET设置
    msgctl(msgid, IPC_SET, &buf);

    return 0;
}