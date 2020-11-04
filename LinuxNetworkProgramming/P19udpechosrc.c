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

    echo_srv(sock);
    return 0;
}