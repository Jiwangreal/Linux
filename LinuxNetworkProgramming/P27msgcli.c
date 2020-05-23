//
// Created by wangji on 19-8-12.
//

// p25 system v消息队列(三)回射客户端

#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#define ERR_EXIT(m) \
        do \
        { \
             perror(m); \
             exit(EXIT_FAILURE);    \
        } while (0);

#define MSGMAX 8192

#define MSGMAX 8192

struct msgbuf 
{
    long mtype;       /* message type, must be > 0 */
    char mtext[MSGMAX];    /* message data */
};


int echocli(int msgid)
{
    struct msgbuf msg;
    memset(&msg, 0, sizeof(msgbuf));
    int pid = getpid();
    msg.mtype = 1;//客户端给服务器段发送的消息类型总是=1
    *((int*)msg.mtext) = pid;//msg.mtext前4个字节是pid
    int n;
    //msg.mtext + 4,表示前4个字节是pid
    while (fgets(msg.mtext + 4, MSGMAX, stdin) != NULL)//不停的从键盘中获取1行数据
    {
        //msgsnd(msgid, &msg,4 + strlen(4+msg.mtext), 0)也对，客户端发送的是：pid+回射行line
        if (msgsnd(msgid, &msg, sizeof(long) + strlen(msg.mtext), 0) < 0)
        {
            ERR_EXIT("msgsnd");
        }
        
        //前4个字节不需要清空，用以保存pid
        memset(msg.mtext+4, 0, sizeof(msg.mtext + 4));

        if ((n = msgrcv(msgid, &msg, MSGMAX, pid, 0)) < 0)//接收类型是pid的消息
        {
            ERR_EXIT("msgsnd");
        }
        fputs(msg.mtext + 4, stdout);
        memset(msg.mtext + 4, 0, sizeof(msg.mtext + 4));
    }


}

int main(int argc, char** argv)
{
    int msgid;
    msgid = msgget(1234, 0);
    if (msgid == -1)
    {
        ERR_EXIT("msgget");
    }

    echocli(msgid);



    return 0;
}
