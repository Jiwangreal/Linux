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

//一条消息的最大值不能超过8192
#define MSGMAX 8192


struct msgbuf 
{
    long mtype;       /* message type, must be > 0 */
    char mtext[1];    /* message data */
};


int main(int argc, char** argv)
{
    int flag = 0;
    int type = 0;
    int opt;

    while (1)
    {
        //man 3 getopt查询该函数用法
        //因为执行的时候:./P26msg_recv -n -t 1(等价于./P26msg_recv -n -t1)，可以用getopt函数来解析参数
        //-n表示消息以非阻塞方式接收(n)，,-t 1表示消息类型为1(t:)，可以用："nt:"来表示，:表示-t 参数后面的1
        opt = getopt(argc, argv, "nt:");
        if (opt == '?')//表示解析到不认识的参数
        {
            exit(EXIT_FAILURE);
        }
        if (opt == -1)//opt=-1，表示所有的参数都解析完毕
        {
            break;
        }

        //switch解析参数
        switch (opt)
        {
            case 'n':
                // printf("AAA\n");
                flag |= IPC_NOWAIT;
    
                break;
            case 't':
                // printf("BBB\n");
                type = atoi(optarg);//t后面的参数1保存在optarg
                //printf("n=%d\n",type);//type=0表示按顺序接收            
                break;
        }
    }


    int msgid;
    msgid = msgget(1234, 0);
    if (msgid == -1)
    {
        ERR_EXIT("msgget");
    }

    struct msgbuf *ptr;
    ptr = (struct msgbuf*) malloc(sizeof(long) + MSGMAX);//实际消息的最大值
    ptr->mtype = type;
    //msgrcv( msqid, &buf1, recvlength ,3,0 ) ;
    int n;//接收到的字节数
    if ((n = msgrcv(msgid, ptr, MSGMAX, type, flag)) < 0)
    {
        ERR_EXIT("msgsnd");
    }
    printf("type=%d, bytes=%d\n", ptr->mtype, (int)n) ;
    return 0;
}
