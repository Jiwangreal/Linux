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

//当前用户名
char username[16];

//聊天室成员列表
USER_LIST client_list;

void do_someone_login(MESSAGE& msg);
void do_someone_logout(MESSAGE& msg);
void do_getlist(int sock);
void do_chat(const MESSAGE& msg);

void parse_cmd(char* cmdline, int sock, struct sockaddr_in *servaddr);
bool sendmsgto(int sock, char* name, char* msg);

void parse_cmd(char* cmdline, int sock, struct sockaddr_in *servaddr)
{
    char cmd[10] = {0};
    char *p;
    p = strchr(cmdline, ' ');//寻找空格，将指针定位到空格的位置
    if (p != NULL)
        *p = '\0';
    strcpy(cmd, cmdline);

    //如果是exit命令
    if (strcmp(cmd, "exit") == 0)
    {
        MESSAGE msg；
        memset(&msg, 0, sizeof(msg));
        msg.cmd = htonl(C2S_LOGOUT);
        strcpy(msg.body, username);

        if (sendto(sock, &msg, sizeof(msg), 0, (struct sockaddr *)servaddr, sizeof(servaddr)) == -1)
            ERR_EXIT("sendto");
        
        printf("user %s has logout server\n", username);
        exit(EXIT_SUCESS);
    }
    else if (strcmp(cmd, "send") == 0)//如果是send命令
    {
        char peername[16]={0};
        char msg[MSG_LEN]={0};

        /*
        send user msg
              p   p2
        */
       while(*p++ == ' ');
       char *p2;
       p2 = strchr(p, ' ');
       if (p2 == NULL)
       {
           printf("bad command\n");
           printf("\nCommands are:\n");
           printf("send username msg\n");
           printf("list\n");
           printf("exit\n");
           return;
       }
       *p2 == '\0';
       strcpy(peername, p);
       while (*p2++ == ' ');
       strcpy(msg, p2);
       sendmsgto(sock, peername, msg);
    }
    else if (strcmp(cmd, "list") == 0)
    {
        MESSAGE msg;
        memset(&msg, 0, sizeof(msg));
        msg.cmd = htonl(C2S_ONLINE_USER);

        if (sendto(sock, &msg, sizeof(msg), 0, (struct sockaddr *)servaddr, sizeof(servaddr)) == -1)
            ERR_EXIT("sendto");

    }
    else//如果是其他命令
    {
        printf("bad command\n");
           printf("\nCommands are:\n");
           printf("send username msg\n");
           printf("list\n");
           printf("exit\n");
    }
    
}

bool sendmsgto(int sock, char* name, char* msg)
{
    if (strcmp(name, username) == 0)
    {
        printf("can't send message to self\n");
        return false;
    }
    USER_LIST::iterator it;
    for (it = client_list.begin(); it !=client_list.end();++it)
    {
        if (strcmp(it->username, name) == 0)
            break;
    }
    if (it == client_list.end())
    {
        printf("user %s has not logined server\n", name);
        return false;
    }

    MESSAGE m;
    memset(&m, 0, sizeof(m));
    m.cmd = htonl(C2C_CHAT);

    CHAT_MSG cm;
    strcpy(cm.username, username)；
    strcpy(cm.msg, msg);

    memcpy(m.body, &cm, sizeof(cm));

    //得到对方的ip和端口
    strcut sockaddr_in peeraddr;
    memset(&peeraddr, 0, sizeof(peeraddr));
    peeraddr.sin_family = AF_INET;
    peeraddr.sin_addr.s_addr = it->ip;
    peeraddr.sin_port = it->port;

    printf("sending message [%s] to user [%s] <-> %s:%d\n", msg, name, inet_ntoa(it->ip),ntohs(it->port));
    sendto(sock, (const char*)&n, sizeof(m), 0, (struct sockaddr *)&peeraddr, sizeof(peeraddr));

    return true;
}

void do_getlist(int sock)
{
    int count;
    recvfrom(sock, &count, sizeof(int), 0, NULL, NULL);
    printf("has %d users logined server\n", ntohl(count));
    client_list.clear();//首先将当前的在线用户列表清空

    int n = ntohl(count);
    for (int i=0; i<n; i++)
    {
        USER_INFO user;
        recvfrom(sock, &user, sizeof(USER_INFO), 0, NULL, NULL);
        client_list.push_back(user);
        in_addr tmp;
        tmp.s_addr = user.ip;

        printf("%s <-> %s:%d\n", user.username, inet_ntoa(tmp), ntohs(user.port));
    }
}

void do_someone_login(MESSAGE& msg)
{
    USER_INFO *user = (USER_INFO*)msg.body;
    in_addr tmp;
    tmp.s_addr = user->ip;
    printf("%s <-> %s:%d has logined server\n", user->username, inet_ntoa(tmp),ntohs(user->port));
    client_list.push_back(*user);
}

void do_someone_logout(MESSAGE& msg)
{
    USER_LIST::iterator it;
    //需要遍历在线用户列表
    for (it=client_list.begin(); it !=client_list.end(); ++it)
    {
        if (strcmp(it->username, msg.body) == 0)
            break;
    }
    if (it != client_list.end())
        client_list.erase(it);
    printf("user %s has logout server\n", msg.body);
}

void do_chat(const MESSAGE& msg)
{
    CHAT_MSG *cm = (CHAT_MSG *)msg.body;
    printf("recv a msg [%s] from [%s]\n", cm->msg, cm->username);
}

void chat_cli(int sock)
{
    //初始化服务端地址
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(5188);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    struct sockaddr_in peeraddr;
    socklen_t peerlen;

    MESSAGE msg;
    while (1)
    {
        memset(username, 0, sizeof(username));
        printf("please input your name:");
        fflush(stdout);
        scanf("%s", username);

        //生成一个消息
        memset(&msg, 0, sizeof(msg));
        msg.cmd = htonl(C2S_LOGIN);//消息msg的命令是C2S_LOGIN
        strcpy(msg.body, username);

        sendto(sock, &msg, sizeof(msg), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));

        memset(&meg, 0, sizeof(msg));
        recvfrom(sock, &msg, sizeof(msg), 0, NULL, NULL);
        int cmd = ntohl(msg.cmd);
        if (cmd == S2C_ALREADY_LOGINED)
            printf("user %s already logined server, please user another username", username);
        else if (cmd == S2C_LOGIN_OK)
        {
            printf("user %s has logined server", username);
            break;
        }
    }
    //已经登录成功后，需要执行的代码
    int count;
    recvfrom(sock, &count, sizeof(int), 0, NULL, NULL);

    int n = ntohl(count);
    printf("has %d users logined server\n", n);

    for (int i = 0; i < n; i++)
    {
        USER_INFO user;
        recvfrom(sock, &user, sizeof(USER_INFO), 0 ,NULL, NULL);
        client_list.push_back(user);//客户端也维护了成员列表client_list
        in_addr tmp;
        tmp.s_addr = user.ip;

        printf("%d %s <->%s:%d\n", i, user.username, inet_ntoa(tmp), ntohs(user.port));//用户名，ip地址，端口号
    }

    //一个用户登录成功后，就输出下面的信息,表示当前可用的命令
    printf("\nCommands are :\n");
    printf("send username msg\n");
    printf("list\n");
    printf("exit\n");
    printf("\n");

    fd_set rset;
    FD_ZERO(&rset);
    int nready;
    while (1)
    {
        //IO多路复用，一个是对sock，还有一个是对标准输入IO进行操作，所以用select统一来管理
        //否则，若程序阻塞在标准输入，若此时有网络消息到来，则办法及时处理该消息
        FD_SET(STDIN_FILENO, &rset);
        FD_SET(sock, &rset);
        nready = select(sock+1, &rset, NULL, NULL, NULL);//监听读集合rset中的fd是否产生可读事件
        if (nready == -1)
            ERR_EXIT("select");
        if (nready == 0)
            continue;
        
        //若产生了可读事件
        if (FD_ISSET(sock, &rset))
        {
            peerlen = sizeof(peeraddr);
            memset(&msg, 0 ,sizeof(msg));
            recvfrom(sock, &msg, sizeof(msg), 0, (struct sockaddr *)&peeraddr, &peeraddr);
            int cmd = ntohl(msg.cmd);
            switch (cmd)
            {
                case S2C_SOMEONE_LOGIN:
                    do_someone_login(msg);
                    break;
                case S2C_SOME_LOGOUT:
                    do_someone_logout(msg);
                    break;
                case S2C_ONLINE_USER://在线用户列表的情况
                    do_getlist(sock);
                    break;
                case C2C_CHAT://客户端向客户端发送聊天信息的情况
                    do_chat(msg);
                    break;
                default:
                    break;

            }
        }
        //如果是标准输入产生了可读事件
        if (FD_ISSET(STDIN_FILENO, &rset))
        {
            char cmdline[100] = {0};
            if (fgets(cmdline, sizeof(cmdline), stdin) == NULL)
                break;
            if (cmdline[0] == '\n')
                continue;
            cmdline[strlen(cmdline) - 1] = '\0';
            parse_cmd(cmdline, sock, &servaddr);
        }

    }

    memset(&msg, 0 ,sizeof(msg));
    msg.cmd = htonl(C2S_LOGOUT);
    strcpy(msg.body, username);

    sendto(sock, (const char*)&msg, sizeof(msg), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
    close(sock);
}


int main(void)
{
    int sock;
    //创建数据包socket
    if ( (sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0 )
        ERR_EXIT("sock");

    chat_cli(sock);

    return 0;
}