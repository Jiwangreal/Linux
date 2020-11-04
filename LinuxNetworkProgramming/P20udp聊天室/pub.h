#idndef _PUB_H_
#define _PUB_H_

#include <list>
#include <algorithm>
using namespace std;

//C2S
#define C2S_LOGIN 0x01
#define C2S_LOGOUT 0x02
#define C2S_ONLINE_USER 0x03

#define MSG_LEN 512
//S2C
#define S2C_LOGIN_OK 0x01
#define S2C_ALREADY_LOGINED 0x02
#define S2C_SOMEONE_LOGIN 0x03
#define S2C_SOMEONE_LOGOUT 0x04
#define S2C_ONLINE_USER 0x05

//C2C
#define C2C_CHAT 0x06

//传递的消息结构
typedef struct message
{
    int cmd;//表示上面的消息
    char body[MSG_LEN];//消息的内容，存放用户名
}MESSAGE;

//用户信息的结构体
typedef struct user_info
{
    char username[16];//用户名
    unsigned int ip;//ip，用网络字节序的4个字节来表示
    unsigned short port;//端口，用2个字节的网络字节序来表示
}USER_INFO;

//客户端与客户端传输的消息
typedef struct chat_msg
{
    char username[16];//用户名
    char msg[100];//发送的消息
}CHAT_MSG;

//用C++好处：不用自己去实现链表
typedef list<USER_INFO> USER_LIST;//定义了一个列表list，链表类型是：USER_LIST

#endif /*_PUB_H_*/