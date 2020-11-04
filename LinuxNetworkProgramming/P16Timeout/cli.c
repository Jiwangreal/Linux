#include <sysutil.h>

int main(void)
{
    int sock;
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        ERR_EXIT("socket");
    
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sinport = htons(5188);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    int ret = connect_timout(sock, &servaddr, 5);
    if (ret == -1 && errno == ETIMEDOUT)
    {
        printf("timeout...\n");
        return 1;
    }
    else if (ret == -1)
        ERR_EXIT("connect_timeout");

    struct sockaddr_in localaddr;
    socklen_t addrlen = sizeof(localaddr);

    struct sockaddr_in localaddr;
    socklen_t addrlen = sizeof(localaddr);
    if (getsockname(sock, (struct sockaddr*)localaddr, &addrlen) < 0)
        ERR_EXIT("getsockname");

    printf("ip=%s port =%d\n", inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port));//打印本地端口和ip

    return 0;
}