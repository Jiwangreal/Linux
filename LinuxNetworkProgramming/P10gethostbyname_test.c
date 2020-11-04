//
// Created by wangji on 19-8-6.
//

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define ERR_EXIT(m) \
        do \
        { \
            perror(m); \
            exit(EXIT_FAILURE); \
        } while (0);

int getlocalip(char *ip)
{
    char host[100] = {0};
    if (gethostname(host, sizeof host) < 0)
    {
        ERR_EXIT("gethostname");
    }

    struct hostent *hp;
    if ((hp = gethostbyname(host)) == NULL)
    {
        ERR_EXIT("gethostbyname");
    }
    strcpy(ip, inet_ntoa(*(struct in_addr*)hp->h_addr_list[0]));//获取第一条iip
    //等价于trcpy(ip, inet_ntoa(*(struct in_addr*)hp->h_addr)); man gethostbyname可以看到#define h_addr h_addr_list[0]


    return 0;
}

int main(void)
{
   char host[100] = {0};
   if (gethostname(host, sizeof host) < 0)
   {
       ERR_EXIT("gethostname");
   }

   struct hostent *hp;
   if ((hp = gethostbyname(host)) == NULL)
   {
       ERR_EXIT("gethostbyname");
   }
   int i = 0;//若不初始化，会出现段错误
   while (hp->h_addr_list[i] != NULL)//hp->h_addr_list[i] 是char*的，是ip地址的一种结构，不是点分十进制的
   {
           // 先强转为struct in_addr*，解惑取个*，就可以得到struct in_addr地址结构
       printf("%s\n", inet_ntoa(*(struct in_addr*)hp->h_addr_list[i]));//这里转换成点分十进制
       ++i;
   }

    char ip[16] = {0};
    getlocalip(ip);
    printf("localip = %s\n", ip);

    return 0;
}
