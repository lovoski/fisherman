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
#include <fstream>
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
  // conversation list of each user
  std::vector<int> conv_list;
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
  std::vector<int> members;
  std::vector<int> files;
  const char *historypath;
  pthread_mutex_t mtx;
  std::fstream history;
  conversation() {
    pthread_mutex_init(&mtx, NULL);
  }
  ~conversation() {
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
/**
 * status_code:
 * 0 -> {account exists, password correct}
 * 1 -> {account exists, wrong password}
 * 2 -> {account don't exist, new account created}
*/
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
  bool keep_serving = true;
  user *user_map;
  file *file_map;
  conversation *conv_map;
  std::map<std::string, int> username_map;
  std::vector<interface_func> interface_map;
  threadpool *client_listeners;
  threadpool *requests_handler;
};

#endif