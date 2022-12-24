//UDP网络编程之服务器

#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h> 
#include <string.h>
#include <unistd.h>

#define N 128
#define ERRLOG(errmsg) do{\
                            perror(errmsg);\
                            printf("%s - %s - %d\n", __FILE__, __func__, __LINE__);\
                            exit(1);\
                            }while(0)
typedef struct{
    int code; //操作码
    char name[32];  //用户名
    char text[32];  //消息
}MSG;

//链表结点结构体
typedef struct node{
    struct sockaddr_in addr; //数据域，用于保存每一个在线用户的信息
    struct node *next;
}linklist;

linklist *linklistcreate();
void do_login(int sockfd, MSG msg, linklist *h, struct sockaddr_in clientaddr);
void do_chat(int sockfd, MSG msg, linklist *h, struct sockaddr_in clientaddr);
void do_quit(int sockfd, MSG msg, linklist *h, struct sockaddr_in clientaddr);
int main(int argc, char const *argv[])
{
    if(argc < 3)
    {
        fprintf(stderr, "Usage: %s <ip> <port>\n", argv[0]);
        exit(1);
    }

    int sockfd;
    struct sockaddr_in serveraddr, clientaddr;
    socklen_t addrlen = sizeof(serveraddr);
    char buf[N] = {0};

    //第一步：创建套接字
    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        ERRLOG("socket error");
    }

    //第二步：填充服务器网络信息结构体
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(argv[1]);
    serveraddr.sin_port = htons(atoi(argv[2]));

    //第三步：将套接字与服务器网络信息结构体绑定
    if(bind(sockfd, (struct sockaddr *)&serveraddr, addrlen) == -1)
    {
        ERRLOG("bind error");
    }

    //创建子进程，实现一边发送系统信息，一边接收数据并处理
    pid_t pid;
    if((pid = fork()) == -1)
    {
        ERRLOG("fork error");
    }
    else if(pid > 0) //父进程负责发送系统信息
    {
        //将父进程看做是一个客户端，按照群聊的形式将数据发送给子进程
        MSG msg;
        msg.code = 2;
        strcpy(msg.name, "server");
        while(1)
        {
            fgets(msg.text, 32, stdin);
            msg.text[strlen(msg.text) - 1] = '\0';

            if(sendto(sockfd, &msg, sizeof(msg), 0, (struct sockaddr *)&serveraddr, addrlen) == -1)
            {
                ERRLOG("sendto error");
            }
        }
    }
    else //子进程负责接收数据并处理
    {
        MSG msg;
        linklist *h = linklistcreate();
        while(1)
        {
            if(recvfrom(sockfd, &msg, sizeof(msg), 0, (struct sockaddr *)&clientaddr, &addrlen) == -1)
            {
                ERRLOG("recvfrom error");
            }  

            printf("code:%d, name:%s, text:%s\n", msg.code, msg.name, msg.text);

            //根据接收到的数据做出相应的处理
            //登录：1   群聊：2   退出：3
            switch(msg.code)
            {
            case 1:
                //登录操作
                do_login(sockfd, msg, h, clientaddr);
                break;   
            case 2:
                //群聊操作
                do_chat(sockfd, msg, h, clientaddr);
                break;
            case 3:
                //退出操作
                do_quit(sockfd, msg, h, clientaddr);
                break;
            }
        }
    }

    return 0;
}

linklist *linklistcreate()
{
    linklist *h = (linklist *)malloc(sizeof(linklist));
    h->next = NULL;

    return h;
}

void do_login(int sockfd, MSG msg, linklist *h, struct sockaddr_in clientaddr)
{
    char buf[N] = {0};
    //组包将新用户登录的信息发送给其他在线的用户
    sprintf(buf, "----------------------%s上线了----------------------", msg.name);

    linklist *p = h;

    //遍历链表，给每一个数据域发送数据
    while(p->next != NULL)
    {
        p = p->next;

        if(sendto(sockfd, buf, N, 0, (struct sockaddr *)&p->addr, sizeof(struct sockaddr_in)) == -1)
        {
            ERRLOG("sendto error");
        }
    }

    //将新登录用户的信息保存在链表中
    linklist *temp = (linklist *)malloc(sizeof(linklist));
    temp->addr = clientaddr;
    temp->next = NULL;

    temp->next = h->next;
    h->next = temp;
}

void do_chat(int sockfd, MSG msg, linklist *h, struct sockaddr_in clientaddr)
{
    //组包将用户的群聊信息发送给其他用户
    //组包
    char buf[N] = {0};
    sprintf(buf, "%s: %s", msg.name, msg.text);

    //遍历链表将信息发送给其他用户
    linklist *p = h;
    while(p->next != NULL)
    {
        p = p->next;

        //发送者不接收自己发送的数据
        if(memcmp(&p->addr, &clientaddr, sizeof(clientaddr)) != 0)
        {
            if(sendto(sockfd, buf, N, 0, (struct sockaddr *)&p->addr, sizeof(struct sockaddr_in)) == -1)
            {
                ERRLOG("sendto error");
            }
        }
    }

}

void do_quit(int sockfd, MSG msg, linklist *h, struct sockaddr_in clientaddr)
{
    char buf[N] = {0};
    //组包将用户退出的信息发送给其他在线的用户
    sprintf(buf, "----------------------%s下线了----------------------", msg.name);

    //遍历链表将信息发送给其他用户
    linklist *p = h;
    linklist *temp;
    while(p->next != NULL)
    {
        //从链表中删除退出用户的信息
        if(memcmp(&p->next->addr, &clientaddr, sizeof(clientaddr)) == 0)
        {
            temp = p->next;
            p->next = temp->next;

            free(temp);
            temp = NULL;
        }
        else 
        {
            if(sendto(sockfd, buf, N, 0, (struct sockaddr *)&p->next->addr, sizeof(struct sockaddr_in)) == -1)
            {
                ERRLOG("sendto error");
            }
            p = p->next;
        }
    }
}