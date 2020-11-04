#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>//sockaddr_in
#include <string.h>

#define ERR_EXIT(m) \
        do  \
        {   \
            perror(m);  \
            exit(EXIT_FAILURE); \
        } while(0);



void echo_srv(int sock)
{
    char recvbuf[1024] = {0};
    struct sockaddr_in peeraddr;
    socklen_t peerlen;
    int n;
    while (1)
    {
        peerlen = sizeof(peeraddr);
        memset(recvbuf, 0, sizeof(recvbuf));
        //指定对方的地址信息:peeraddr,peerlen
        n = recvfrom(sock, recvbuf, sizeof(recvbuf), 0, (struct sockaddr*)&peeraddr, &peerlen);
        if (n == -1)
        {
            if (errno == EINTR)//被信号中断的话，继续接收
                continue;
            ERR_EXIT("recvfrom");
        }
        else if (n > 0)
        {
            fputs(recvbuf, stdout);
            sendto(sock, recvbuf, n, 0, (struct sockaddr*)&peeraddr, peerlen);
        }
    }
    close(sock);
}

int main(void)
{
    int sock;
    //PF_INET:IPv4的地址家族
    //SOCK_DGRAM：IPv4家族中的udp套接口，所以第三个参数可为0
    if ((sock = socket(PF_INET, SOCK_DGRAM, 0))< 0);
        ERR_EXIT("socket");
    
    //三个参数：地址族，端口，本机ip地址
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, servaddr);
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(5188);
    servaddr.sin_addr.s_addr = htons(INADDR_ANY);

    if (bind(sock, (struct sockaddr*)&servaddr, sizeof(servaddr)) <0)
        ERR_EXIT("bind");
    
    //服务端也是客户端
    //若sendto中的4写成0的话，不会发送任何字节数据，但是实际上若不算数据链路层，
    //发送的就是一个TCP的头部（20字节）和一个IP头部（20字节），共40字节，对方受到返回0，不代表连接的关闭
    //因为udp是无连接的
    sendto(sock, "ABCD", 4, 0, (struct sockaddr*)&servaddr, sizeof(servaddr));
    int n;
    char recvbuf[1];
    int i;
    for (i = 0; i < 4, i++)
    {
        //接收的长度sizeof(recvbuf)一定要大于发送方所发送的数据报的长度ABCD，才不会出现截断现象
        //这里是出现截断了
        n = recvfrom(sock, recvbuf, sizeof(recvbuf), 0, NULL, NULL);
        if (n == -1)
        {
            if (errno == EINTR)
                continue;
            ERR_EXIT("recvfrom");
        }
        else if (n >0)
            printf("n=%d %c\n",n, recvbuf[0]);
    }
    
    return 0;
}