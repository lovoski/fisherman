#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h> 
#include <string.h>
#include <unistd.h>
#include <signal.h>

int argv2int(char *argv, int len) {
  int ret = 0, n = 1;
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
  login.inno[3] = 1;
  memcpy(login.content, password, sizeof(password));
  memcpy(login.content+200, username, sizeof(username));
  while (keep_serving) {
    if (sendto(sockfd, &login, max_msg_len, 0, (struct sockaddr *)&serveraddr, addrlen) == -1) {
      printf("error login\n");
    } else {
      message ret;
      __start:
      recvfrom(sockfd, &ret, max_msg_len, 0, NULL, NULL);
      if (argv2int(ret.inno, 4) != 1)
        goto __start;
      else {
        printf("[login] status_code=%d\n", argv2int(ret.content, 4));
        break;
      }
    }
  }
  pid_t pid;
  if ((pid = fork()) == -1) {
    perror("error forking\n");
    exit(1);
  } else if (pid > 0) {
    while (keep_serving) {
      // scanf("localhost:%s", input_text);
      // ... 
    }
  } else {
    message msg;
    while (keep_serving) {
      if (recvfrom(sockfd, msg.uid, max_msg_len, 0, NULL, NULL) == -1) {
        printf("error recvfrom\n");
      }
    }
  }
  return 0;
}