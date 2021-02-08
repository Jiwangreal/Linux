#include <stdio.h>
#include <arpa/inet.h>

int main()
{
    unsigned long addr = inet_addr("192.168.0.100");

    printf("addr = %u \n", ntohl(addr));//u：无符号方式输出

    struct in_addr ipaddr;
    ipaddr.s_addr=addr;
    printf("%s\n",inet_ntoa(ipaddr));    
    return 0;
}


