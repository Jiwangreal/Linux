#include <sysutil.h>

/*
read_timeout使用方法
int ret;
ret  = read_timeout(fd, 5);
if (ret == 0)//成功返回0，进行读操作
{
    read(fd,....);若wait_seconds=0，则阻塞在read
}
else if (ret == -1 && errno == ETIMEOUT)
{
    timeout...
}
else
{
    ERR_EXIT("read_timeout");
}
*/


/*
    * read timeout - 读超时检测函数，不含读操作
    * @fd：文件描述符
    * @wait_seconds:等待超时秒数，如果为0表示不检测超时
    * 成功（未超时）返回0，失败返回-1，超时返回-1并且errno = ETIMEDOUT 
*/
//若检测到超时，就不进行读操作，若返回成功未超时，将做读操作
//read_timeout作用：检测IO是否超时
int read_timeout(int fd, unsigned int wait_seconds)
{
    int ret = 0;
    if (wait_seconds > 0)
    {
        fd_set read_fdset;//读集合
        struct  timeval timeout;//超时时间

        FD_ZERO(&read_timeout);
        FD_SET(fd, &read_fdset);

        timeout.tv_sec = wait_seconds;
        timeout.tv_usec = 0;

        //超时时间到了，没有发生可读事件，那么返回值为0
        //在超时时间之内，发生可读事件，返回值为1
        //select调用失败返回小于0，若小于0是由信号中断的话(errno == EINTR)，则继续调用select
        do 
        {
            ret = select(fd +1, &read_fdset, NULL, NULL, &timeout);
        }while(ret < 0 && errno == EINTR);


        if (ret == 0 )
        {
            ret = -1;
            errno = ETIMEDOUT;
        }
        else if (ret == 1)//因为当前只将一个fd放入到读集合fdset中
            ret = 0;
        
        //若ret= -1,并且errno不等于EINTR，则不做任何处理，直接return
        //若wait_seconds=0，则不按照超时的方式进行处理
    }

    //wait_seconds=0，则直接返回ret=0
    return ret;
}

/*
    * read timeout - 读超时检测函数，不含写操作
    * @fd：文件描述符
    * @wait_seconds:等待超时秒数，如果为0表示不检测超时
    * 成功（未超时）返回0，失败返回-1，超时返回-1并且errno = ETIMEDOUT 
*/
//若检测到超时，就不进行写操作，若返回成功未超时，将做写操作
//write_timeout作用：检测IO是否超时
int write_timeout(int fd, unsigned int wait_seconds)
{
    int ret = 0;
    if (wait_seconds > 0)
    {
        fd_set write_fdset;//写集合
        struct  timeval timeout;//超时时间

        FD_ZERO(&write_timeout);
        FD_SET(fd, &write_fdset);

        timeout.tv_sec = wait_seconds;
        timeout.tv_usec = 0;

        do 
        {
            ret = select(fd +1, &write_timeout, NULL, NULL, &timeout);
        }while(ret < 0 && errno == EINTR);

        if (ret == 0 )
        {
            ret = -1;
            errno = ETIMEDOUT;
        }
        else if (ret == 1)//因为当前只将一个fd放入到读集合fdset中
            ret = 0;
    }

    return ret;
}

/*
 * accept_timeout 带超时的accept
 * fd：套接字,检测的IO
 * addr：输出参数，返回对方地址
 * wait_seconds：等待超时秒数，如果为0表示正常模式
 * 成功（未超时）返回已连接套接字，超时返回-1并且errno = ETIMEDOUT
*/
int accept_timeout(int fd, struct sockaddr_in *addr, unsigned int wait_seconds)
{
    int ret;
    socklen_t addrlen = sizeof(struct sockaddr_in);

    if (wait_seconds > 0)
    {
        fd_set accept_fdset;
        struct timeval timeout;
        FD_ZERO(&accept_fdset);
        FD_SET(fd, &accept_fdset);
        timeout.tv_sec = wait_seconds;
        timeout.tv_usec = 0;
        do
        {
            //三次握手已完成，已完成连接队列有一个条目，accept不阻塞，
            //就说明accept_fdset集合中产生了可读事件
            ret = select(fd+1, &accept_fdset, NULL, &timeout);
        } while (ret < 0 && errno == EINTR);

        if (ret == -1)//这里是ret=-1，errno不等于EINTR
            return -1;
        else if (ret == 0)//ret=0表示超时了
        {
            errno = ETIMEDOUT;
            return -1;
        }
    }

    //ret=1表示：表示检测到事件
    //wait_seconds=0,则直接调用accept，会阻塞
    if (addr != NULL)
        ret = accept(fd, (struct sockaddr*)addr, &addrlen);//返回已连接套接字
    else
        ret = accept(fd, NULL, NULL);

    if (ret == -1)
        ERR_EXIT("accept");    
    return ret;
}

/*
 * activate_noblock：设置I/O为非阻塞模式
 * @fd：文件描述符
*/
void activate_nonblock(int fd)
{
    int ret;
    int flags = fcntl(fd, F_GETFL);//获取fd的标记
    if (flags == -1)
        ERR_EXIT("fcntl");

    flags |= O_NONBLOCK;//添加非阻塞模式
    ret = fcntl(fd, F_SETTFL, flags);
    if (ret == -1)
        ERR_EXIT("fcntl");
}

/*
 * activate_noblock：设置I/O为阻塞模式
 * @fd：文件描述符
*/
void deactivate_nonblock(int fd)
{
    int ret;
    int flags = fcntl(fd, F_GETFL);
    if (flags == -1)
        ERR_EXIT("fcntl");

    flags &= ~O_NONBLOCK;//去掉非阻塞模式
    ret = fcntl(fd, F_SETTFL, flags);
    if (ret == -1)
        ERR_EXIT("fcntl");
}

/*
 * connect_timeout  connect
 * @fd：套接字
 * @addr：要连接的对方地址
 * @wait_seconds等待超时秒数，如果为0表示正常模式
 * 成功（未超时）返回0，失败返回-1，超时返回-1并且errno=ETIMEDOUT
*/
int connect_timeout(int fd, struct sockaddr_in *addr, unsigned int wait_seconds)
{
    int ret;
    socklen_t addrlen = sizeof(struct sockaddr_in);

    //不能直接调用connect，直接调用就会阻塞，希望以非阻塞的方式调用，所以fd
    //设置为非阻塞模式
    if (wait_seconds > 0)
        activate_nonblock(fd);

    ret = connect(fd, (struct sockaddr*)addr, addrlen);
    if (ret <0 && errno == EINPROGRESS)//非阻塞fd返回一定是EINPROGRESS，表示正在处理中
    {
        printf("AAAAA");
        fd_set connect_fdset;//已连接fd集合
        struct timeval timeout;//超时时间
        FD_ZERO(connect_fdset);
        FD_SET(fd, &connect_fdset);
        timeout.tv_sec = wait_seconds;
        timeout.tv_usec = 0;
        do
        {
            /* 一旦连接建立，套接字就可写 */
            ret = select(fd+1, NULL, &connect_fdset, NULL, &timeout);
        } while (ret <0 && errno == EINTER);
        
        if (ret == 0)//超时时间到了，还没有产生可写事件，意味着连接没成功
        {
            ret = -1;
            errno = ETIMEDOUT;
        }
        else if (ret < 0)
            return -1;
        else if (ret == 1)//检测到可写事件
        {
            printf("BBBBB");
            //ret返回1，可能有2种情况，一种是连接建立成功，一种是套接字产生错误
            //此时错误信息不会保存至errno变量中，因此，需要调用getsockopt来获取
            int err;
            socklen_t socklen = sizeof(err);
            int sockoptret = getsockopt(fd, SOL_SOCKET , SO_ERROR, &err, &socklen);//获取错误到err中
            if (sockoptret == -1)
            {
                return -1;
            }
            if (err == 0)//表示没有错误，连接建立成功
            {
                printf("DDDDD");
                ret = 0;
            }
            else
            {
                printf("CCCCC");
                errno =err;
                ret = -1;
            }
            
        }
        
    }

     if (wait_seconds >0)
     {
         deactivate_nonblock(fd);//重新设置为阻塞模式
     }
     return ret;
}