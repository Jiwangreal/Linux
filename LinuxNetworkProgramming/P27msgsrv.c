//
// Created by wangji on 19-8-12.
//

// p25 system v消息队列(三)回射服务器端

#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>//memset函数的头文件

#define ERR_EXIT(m) \
        do \
        { \
             perror(m); \
             exit(EXIT_FAILURE);    \
        } while (0);


#define MSGMAX 8192

struct msgbuf 
{
    long mtype;       /* message type, must be > 0 */
    char mtext[msgbuf];    /* message data */
};

void echo_srv(int msgid)
{
    struct msgbuf msg;
    int n;
    int type;
    memset(&msg, 0, sizeof(msgbuf));
    while (1)
    {
        if ((n = msgrcv(msgid, &msg, MSGMAX, 1, 0)) < 0)//不停的接收类型=1的消息，0表示以阻塞的方式接收
        {
            ERR_EXIT("msgsnd");
        }

        //解析的来自客户端的pid+回射行line
        type = *((int*)msg.mtext);//取出前4个字节
        msg.mtype = type;

        //前4个字节是pid
        fputs(msg.mtext+4, stdout);

        if (msgsnd(msgid, &msg, n, 0) < 0)
        {
            ERR_EXIT("msgsnd");
        }
        //memset(&msg, 0, sizeof(msgbuf));
    }

}

int main(int argc, char** argv)
{
    int msgid;
    msgid = msgget(1234, IPC_CREAT|0666);
    if (msgid == -1)
    {
        ERR_EXIT("msgget");
    }

    echo_srv(msgid);

    return 0;
}
