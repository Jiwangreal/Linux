#include <stdio.h>

int main()
{
    unsigned int x=0x12345678;
    unsigned char *p= (unsigned char *)&x;
    //字节序的转换
    // 小端：低位放低位
    // 大端：低位放高位，高位放低位
    // P[3]是高地址
    printf("%0x_%0x_%0x_%0x_\n",p[0],p[1],p[2],p[3]);

    unsigned int y= htonl(x);
    p = (unsigned char *)&y;
    printf("%0x_%0x_%0x_%0x_\n",p[0],p[1],p[2],p[3]);
    
    return 0;
}






