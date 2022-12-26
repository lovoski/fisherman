#include<sys/select.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <iostream>

int main(){
    int port_in  = 12321;
    int port_out = 9958;
    int sockfd;

    // 创建socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(-1==sockfd){
        return false;
        puts("Failed to create socket");
    }

    // 设置地址与端口
    struct sockaddr_in addr;
    socklen_t  addr_len=sizeof(addr);

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;       // Use IPV4
    addr.sin_port = htons(port_in);    //
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Time out
    struct timeval tv;
    tv.tv_sec  = 0;
    tv.tv_usec = 200000;  // 200 ms
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(struct timeval));
    
    // 绑定获取数据的端口，作为发送方，不绑定也行
    if (bind(sockfd, (struct sockaddr*)&addr, addr_len) == -1){
        printf("Failed to bind socket on port %d\n", port_in);
        close(sockfd);
        return false;
    }

    int counter = 0;
    while(1)
    {    
        addr.sin_family = AF_INET;
        addr.sin_port   = htons(port_out);
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        char message[1024];
        std::cin>>message;
        //从port.in发送到port.out
        sendto(sockfd,message,1024, 0, (sockaddr*)&addr, addr_len);   
        printf("Sended %d\n", ++counter);
        struct sockaddr_in server_addr;
        socklen_t server_len = sizeof(server_addr);
        char buffer[1024];
        int sz = recvfrom(sockfd, buffer, 1024, 0, (sockaddr*)&server_addr, &server_len);
        if(sz>0)
        {
            buffer[sz] = 0;
            printf("Get Message %d: %s\n", counter++, buffer);
        }
        sleep(1);
    }

    close(sockfd);
    return 0;
}
