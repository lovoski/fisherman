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
#include <vector>
#include <string>
#include "sql.h"
#include "threadpool.h"

class fisherman;
// 1024 bytes
const int message_max_len = 1024;
struct message {
  char uid[4];
  // interface_func number
  char inno[4];
  char content[message_max_len-8];
};
struct user {
  int uid;
  char *username;
  char *password;
  // 0 -> root
  // 1 -> admin
  // 2 -> peasant
  int privillege;
  sockaddr addr;
  // true when login approved
  // set to false when quit
  bool approved_online;
};
struct file {
  int fid;
  char *filename;
  // file shouldn't be store 
  // in database directly
  char *filepath;
  long long filesize;
};
struct conversation {
  int cid;
  pthread_mutex_t mtx;
  pthread_cond_t cond;
  std::vector<int> members;
  std::vector<int> files;
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
void *quit(void *args);
void *broadcast(void *args);
void *file_upload(void *args);
void *file_list(void *args);
void *delete_file(void *args);
void *file_download(void *args);
void *conversation_list(void *args);

/**
 * @brief use for create conversation only
*/
void *create_conversation(void *args);

/**
 * @brief use for modify and delete conversation
*/
void *modify_conversation(void *args);
/**
 * @param args a _args pointer
 * @brief listen to msg from clients and append tasks to request_handler
*/
void *client_listening(void *args);

class fisherman {
public:
  fisherman(
    const char *host = "127.0.0.1", 
    const int port = 9958, 
    const int max_requests = 200);
  ~fisherman();
  void start(const int max_listeners = 10);

  int server_sockfd;
  std::vector<user> user_map;
  std::vector<file> file_map;
  std::map<std::string, int> username_map;
  std::vector<conversation> conv_map;
  std::vector<interface_func> interface_map;
  threadpool *client_listeners;
  threadpool *requests_handler;
};

#endif