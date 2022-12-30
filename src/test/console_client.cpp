#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h> 
#include <string.h>
#include <unistd.h>
#include <signal.h>

void int32_to_argv(__int32_t num, char *ret) {
  int n = 0xff000000;
  for (int i = 0; i < 4; ++i) {
    ret[i] = (num & n) >> (24-8*i);
    n >>= 8;
  }
}
void int64_to_argv(__int64_t num, char *ret) {
  __int64_t n = 0xff00000000000000;
  for (int i = 0; i < 8; ++i) {
    ret[i] = (num & n) >> (56-8*i);
    n >>= 8;
  }
}
__int32_t argv_to_int32(char *argv, int len) {
  int ret = 0, n = 1;
  for (int i = len-1; i >= 0; --i) {
    ret += argv[i] * n;
    n <<= 8;
  }
  return ret;
}
__int64_t argv_to_int64(char *argv, int len) {
  __int64_t ret = 0, n = 1;
  for (int i = len-1; i >= 0; --i) {
    ret += argv[i] * n;
    n <<= 8;
  }
  return ret;
}

const int max_msg_len = 1024;
struct message {
  char uid[4];
  char inno[4];
  char content[max_msg_len - 8];
};

int main() {
  int sockfd;
  struct sockaddr_in serveraddr;
  socklen_t addrlen = sizeof(serveraddr);
  char username[800], password[200];
  char input_text[1000];
  bool keep_serving = true;

  if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
    perror("error creating socket\n");
    exit(1);
  }
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
  serveraddr.sin_port = htons(atoi("9958"));

  printf("username:");
  scanf("%s", username);
  printf("password:");
  scanf("%s", password);
  message login;
  message ret;
  login.inno[3] = 1;
  memcpy(login.content, password, sizeof(password));
  memcpy(login.content+200, username, sizeof(username));
  while (keep_serving) {
    if (sendto(sockfd, &login, max_msg_len, 0, (struct sockaddr *)&serveraddr, addrlen) == -1) {
      printf("error login\n");
    } else {
      __start:
      recvfrom(sockfd, &ret, max_msg_len, 0, NULL, NULL);
      if (argv_to_int32(ret.inno, 4) != 1)
        goto __start;
      else {
        printf("uid:%d\n",argv_to_int32(ret.uid,4));
        printf("[login] status_code=%d\n", argv_to_int32(ret.content, 4));
        break;
      }
    }
  }
  pid_t pid = fork();
  if (pid == -1) {
    perror("error forking\n");
    exit(1);
  } else if (pid > 0) {
    setbuf(stdout,NULL);
    while (keep_serving) 
    {
      // scanf("localhost:%s", input_text);
      // ... 
      message tmp_msg;
      memcpy(tmp_msg.uid, ret.uid, sizeof(ret.uid));
      printf("inno:\n");
      int inno;
      scanf("%d",&inno);
      int32_to_argv(inno,tmp_msg.inno);
      //printf("\n%d\n",argv_to_int32(tmp_msg.inno,4));
      printf("content:\n");
      scanf("%s",&tmp_msg.content);
      if (sendto(sockfd, &tmp_msg, max_msg_len, 0, (struct sockaddr *)&serveraddr, addrlen) == -1) {
        printf("error send\n");
      }
    }
  } else {
    setbuf(stdout,NULL);
    message msg;
    while (keep_serving) 
    {
      if (recvfrom(sockfd, &msg, max_msg_len, 0, NULL, NULL) == -1) {//改动
        printf("error recvfrom\n");
      }else printf("%s\n",msg.content);
    }
  }
}