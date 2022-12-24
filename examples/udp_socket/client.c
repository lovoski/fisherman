//UDP网络编程之客户端

#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h> 
#include <string.h>
#include <unistd.h>
#include <signal.h>

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

int main(int argc, char const *argv[])
{
    if(argc < 3)
    {
        fprintf(stderr, "Usage: %s <ip> <port>\n", argv[0]);
        exit(1);
    }

    int sockfd;
    struct sockaddr_in serveraddr;
    socklen_t addrlen = sizeof(serveraddr);

    //第一步：创建套接字
    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        ERRLOG("socket error");
    }

    //第二步：填充服务器网络信息结构体
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(argv[1]);
    serveraddr.sin_port = htons(atoi(argv[2]));

    MSG msg;

    //登录操作
    //设置操作码并输入用户名发送给服务器实现登录操作
    msg.code = 1;
    //输入用户名实现登录操作
    printf("您输入用户名：");
    fgets(msg.name, 32, stdin);
    msg.name[strlen(msg.name) - 1] = '\0';

    if(sendto(sockfd, &msg, sizeof(msg), 0, (struct sockaddr *)&serveraddr, addrlen) == -1)
    {
        ERRLOG("sendto error");
    }

    //创建子进程，实现一边发送数据，一边接收数据
    pid_t pid;
    if((pid = fork()) == -1)
    {
        ERRLOG("fork error");
    }
    else if(pid > 0) //父进程负责发送数据
    {
        while(1)
        {
            //输入数据，判断是群聊还是退出操作
            fgets(msg.text, 32, stdin);
            msg.text[strlen(msg.text) - 1] = '\0';

            //退出操作
            if(strcmp(msg.text, "quit") == 0)
            {
                //设置操作码
                msg.code = 3;

                //将退出消息发送给服务器
                if(sendto(sockfd, &msg, sizeof(msg), 0, (struct sockaddr *)&serveraddr, addrlen) == -1)
                {
                    ERRLOG("sendto error");
                }

                //客户端退出
                kill(pid, SIGKILL);
                exit(0);
            }
            //群聊操作
            else 
            {
                //设置操作码
                msg.code = 2;

                //将群聊数据发送给服务器
                if(sendto(sockfd, &msg, sizeof(msg), 0, (struct sockaddr *)&serveraddr, addrlen) == -1)
                {
                    ERRLOG("sendto error");
                }
            }
        }
    }
    else //子进程负责接收数据
    {
        char buf[N] = {0};
        while(1)
        {
            if(recvfrom(sockfd, buf, N, 0, NULL, NULL) == -1)
            {
                ERRLOG("recvfrom error");
            }

            printf("%s\n", buf);
        }
    }

    return 0;
}