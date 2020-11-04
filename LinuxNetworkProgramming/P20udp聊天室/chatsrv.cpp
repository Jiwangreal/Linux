#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/wait.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "pub.h"


#define ERR_EXIT(m) \
        do \
        { \
             perror(m); \
             exit(EXIT_FAILURE);    \
        } while (0);

//聊天室成员列表
USER_LIST client_list;

void do_login(MESSAGE& msg, int sock, struct sockaddr_in *cliaddr);
void do_logout(MESSAGE& msg, int cosk, struct sockaddr_in *cliaddr);
void do_sendlist(int sock, struct sockaddr_in *cliaddr);

void do_login(MESSAGE& msg, int sock, struct sockaddr_in *cliaddr)
{
    USER_INFO user;
    strcpy(user.username, msg.body);//取出用户名
    user.ip = cliaddr->sin_addr.s_addr;//网络字节序的32bit整数
    user.port = cliaddr->sin_port;//网络字节序的16bit整数

    //查找用户
    USER_LIST::iterator it;//迭代器
    for (it=client_list.begin(); it !=client_list.end(); ++it)
    {
        if (strcmp(it->username, msg.body) == 0)
        {
            break;
        }
    }

    //没找到用户
    if (it == client_list.end())
    {
        //没找到该用户，说明该用户已经登录了
        printf("has a user login: %s <-> %s:%d\n", msg.body, inet_ntoa(cliaddr->sin_addr.s_addr), ntohs(cliaddr->sin_port));
        client_list.push_back(user);

        //登录成功应答
        MESSAGE reply_msg;
        memset(&reply_msg, 0, sizeof(reply_msg));
        reply_msg.cmd = htonl(S2C_LOGIN_OK);
        sendto(sock, &reply_msg, sizeof(msg), 0, (struct sockaddr*)cliaddr, sizeof(cliaddr));

        int count = htonl((int)client_list.size());
        //发送在线人数
        sendto(sock, &count, sizeof(msg), 0, (struct sockaddr*)cliaddr, sizeof(cliaddr));
        printf("sending user list information to: %s <->%s:%d\n", msg.body, it->username, it->port);

        //发送在线列表,给当前登录的客户端
        for (it = client_list.begin(); it!=client_list.end(); ++it)
        {
            //it是迭代器类型，it是一个指针，保存的是USER_INFO的指针，*it就是USER_INFO
            sendto(sock, &*it, sizeof(USER_INFO), (struct sockaddr*)cliaddr, sizeof(cliaddr));
        }

        //向其它用户通知有新用户登录
        for (it = client_list.begin(); it != client_list.end(); ++it)
        {
            //如果当前的用户是自己的话，不通知自己
            if (strcmp(it->username, msg.body) == 0)
                continue;

            struct sockaddr_in peeraddr;
            memset(&peeraddr, 0, sizeof(peeraddr));
            peeraddr.sin_family = AF_INET;
            peeraddr.sin_port = it->port;
            peeraddr.sin_addr.s_addr = it->ip;

            msg.cmd = htonl(S2C_SOMEONE_LOGIN);
            memcpy(msg.body, &user, sizeof(user));//user表示当前用户的登录信息

            //peeraddr来自迭代器it的遍历
            if (sendto(sock, &msg, sizeof(msg), 0, (struct sockaddr *)&peeraddr, sizeof(peeraddr)) == -1)
                ERR_EXIT("sendto");
        }
    }
    else//没有遍历到结尾，就找到用户
    {
        printf("user %s has already logined\n", msg.body);

        MESSAGE reply_msg;
        memset(&reply_msg, 0, sizeof(reply_msg));
        reply_msg.cmd = htonl(S2C_ALREADY_LOGINED);
        sendto(sock, &reply_msg, sizeof(reply_msg), 0, (struct sockaddr *)cliaddr, sizeof(cliaddr));
    }
    
}

void do_logout(MESSAGE& msg, int cosk, struct sockaddr_in *cliaddr)
{
    printf("has a user logout : %s <-> %s:%d\n", msg.body, inet_ntoa(cliaddr->sin_addr.s_addr), ntohs(cliaddr->sin_port));

    USER_LIST::iterator it;
    for (it = client_list.begin(); it != client_list.end(); ++it)
    {
        if (strcmp(it->username, msg.body) == 0)
            break;
    }
    if (it != client_list.end())
        client_list.erase(it);
    
    //向其它用户通知有用户登出
    for (it = client_list.begin(); it != client_list.end(); ++it)
    {
        if (strcmp(it->username, msg.body) == 0)
            continue;
        
        struct sockaddr_in peeraddr;
        memset(&peeraddr, 0, sizeof(peeraddr));
        peeraddr.sin_family = AF_INET;
        peeraddr.sin_port = it->port;
        peeraddr.sin_addr.s_addr = it->ip;

        msg.cmd = htonl(S2C_SOMEONE_LOGOUT);
        if (sendto(sock, &msg, sizeof(msg), 0, (struct sockaddr *)&peeraddr, sizeof(peeraddr)) == -1)
            ERR_EXIT("sendto"); 
    }
}

void do_sendlist(int sock, struct sockaddr_in *cliaddr)
{
    MESSAGE msg;
    msg.cmd = htonl(S2C_ONLINE_USER);
    sendto(sock, (const char *)&msg, sizeof(msg), 0, (struct sockaddr *)cliaddr, sizeof(cliaddr));

    int count = htonl((int)client_list.size());
    
    //发送在线用户数
    sendto(sock, (const char *)&count, sizeof(int), 0, (struct sockaddr *)cliaddr, sizeof(cliaddr));
    //发送在线用户列表
    for (USER_LIST::iterator, it=client_list.begin(); it !=client_list.end(); ++it)
    {
        sendto(sock, &*it, sizeof(USER_INFO), 0, (struct sockaddr *)cliaddr, sizeof(cliaddr));
    }
}


void chat_srv(int sock)
{
    struct sockaddr_in cliaddr;
    socklen_t clilen;
    int m;
    MEEAGE msg;
    while (1)
    {
        memset(&msg, 0, sizeof(msg));
        clilen = sizeof(cliaddr);
        n = recvfrom(sock, &msg, sizeof(msg), 0, (struct sockaddr *)&cliaddr, &clilen);
        if (n < 0)
        {
            if (errno == EINTR)
                continue;
            ERR_EXIT("recvfrom");
        }

        int cmd = ntohl(msg.cmd);//解析消息，将命令转换为主机字节序
        switch (cmd)
        {
            case C2S_LOGIN:
                do_login(msg, sock, &cliaddr);
                break;
            case C2S_LOGOUT:
                do_logout(msg, sock, &cliaddr);
                break;
            case C2S_ONLINE_USER:
                do_sendlist(sock, &cliaddr);
                break;
            default:
                break;
        }

    }
}

int main(void)
{
    int sock;
    if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0 )
        ERR_EXIT("socket");
    
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_Family = AF_INET;
    servaddr.sin_port = htons(5188);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sock, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
        ERR_EXIT("bind");
    
    chat_srv(sock);
    return 0;
}