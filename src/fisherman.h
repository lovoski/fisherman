#ifndef FISHERMAN_H
#define FISHERMAN_H

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h> 
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <map>
#include "sql.h"
#include "threadpool.h"

class fisherman;
// 1024 bytes
const int message_max_len = 1024;
struct message {
  char uid[4];
  char inno[4];
  char content[message_max_len-8];
};
struct user {
  int uid;
  char *username;
  char *password;
  sockaddr addr;
};
struct conversation {
  int cid;
  pthread_mutex_t mtx;
  pthread_cond_t cond;
  std::vector<int> members;
  std::queue<message> messages;
  conversation() {
    pthread_cond_init(&cond, NULL);
    pthread_mutex_init(&mtx, NULL);
  }
  ~conversation() {
    pthread_mutex_lock(&mtx);
    while (!messages.empty())
      messages.pop();
    pthread_mutex_unlock(&mtx);
    pthread_mutex_destroy(&mtx);
  }
};
struct _args {
  message msg;
  fisherman *server;
  void reset(_args *args) {
    msg = args->msg;
    server = args->server;
  }
};

typedef void *(*interface_func)(void *);
void *test_connect(void *args);
void *login(void *args);
void *public_chat(void *args);
void *private_chat(void *args);
void *file_upload(void *args);
void *broadcast_message(void *args);
/**
 * @param args a _args pointer
 * @brief listen to msg from clients and append tasks to request_handler
*/
void *client_listening(void *args);

class fisherman {
public:
  fisherman(
    const char *host, 
    const int port, 
    const int max_requests);
  ~fisherman();
  void start(const int max_listeners);

  int server_sockfd;
  std::map<int, user> user_map;
  std::map<int, conversation> conv_map;
  std::map<int, interface_func> interface_map;
  threadpool *client_listeners;
  threadpool *requests_handler;
};

#endif