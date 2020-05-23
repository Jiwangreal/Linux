//
// Created by wangji on 19-8-12.
//

// p25 system v消息队列(二)

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

struct msgbuf 
{
    long mtype;       /* message type, must be > 0 */
    char mtext[1];    /* message data */
};



int main(int argc, char** argv)
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <bytes> <type>\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    int len = atoi(argv[1]);//消息长度
    int type = atoi(argv[2]);//消息类型
    int msgid;
    msgid = msgget(1234, 0);
    if (msgid == -1)
    {
        ERR_EXIT("msgget");
    }

    //准备一个消息，分配内存
    struct msgbuf *ptr;

    //消息内存大小=消息类型long mtype+发送的消息的长度len
    ptr = (struct msgbuf*) malloc(sizeof(long) + len);
    ptr->mtype = type;
    if (msgsnd(msgid, ptr, len, 0) < 0)//0表示消息队列满的时候，以阻塞的方式发送
    {
        ERR_EXIT("msgsnd");
    }
    
    /*
    if (msgsnd(msgid, ptr, len, IPC_NOWAIT) < 0)//IPC_NOWAIT表示非阻塞，当发送的消息超过msgmnb时
    {
        ERR_EXIT("msgsnd");
    }
    //会返回EAGAIN 错误：Resource temporarily unavailable
    //发送的消息不会阻塞了
    */



    return 0;
}
