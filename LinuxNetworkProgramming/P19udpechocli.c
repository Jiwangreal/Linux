#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <arpa/inet.h>
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



void echo_cli(int sock)
{
    //三个参数：地址族，端口，本机ip地址
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, servaddr);
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(5188);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");//INADDR_ANY:表示本机任意地址
    
    //解决异步错误，connect保证该套接字不饿能发送数据给其它地址，与TCP3次握手不同
    connect(sock, (struct sockaddr*)&servaddr, sizeof(servaddr));

    int ret;//测试异步错误
    char sendbuf[1024] = {0};
    char recvbuf[1024] = {0};

    /*
    对于已连接套接字而言：
    获取本地地址可以用getsockname
    获取远程地址可以用getpeername
    当前的套接字是未连接的，仅仅可以获取本地的地址，该套接字是第一次sendto的时候绑定的
    */
    while (fgets(sendbuf, sizeof(sendbud), sdtin) !=NULL)
    {
        //若只开启udp客户端，不开启udp服务端
        //会产生ICMP异步错误：sento只是将应用层的缓冲区拷贝套套接口对应的缓冲区，是不报错的，该错误会延迟到recvfrom才收到通知
        //而recvfrom也不不能通知，因为TCP/IP规定：这种异步错误是不能返还给未连接的套接字的，所以recvfrom会一直阻塞
        // sendto(sock, sendbuf, strlen(sendbuf), 0, (struct sockaddr*)&servaddr, sizeof(servaddr));

        //解决异步错误，因为这里的sock的对等方的地址已经由connect指定了
        sendto(sock, sendbuf, strlen(sendbuf), 0, NULL, 0);
        //等价于send(sock, sendbuf, strlen(sendbuf))
        ret = recvfrom(sock, recvbuf, sizeof(recvbuf), 0, NULL, NULL);
        if(ret == -1)
        {
            if (errno == EINTR)
                continue;
            ERR_EXIT("recvfrom");
        }

        fputs(recvbuf, stdout);
        memset(sendbuf, 0, sizeof(sendbuf));
        memset(recvbuf, 0, sizeof(recvbuf));
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
    
    echo_cli(sock);

    return 0;
}