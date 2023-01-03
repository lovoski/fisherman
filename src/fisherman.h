#ifndef FISHERMAN_H
#define FISHERMAN_H

#include <map>
#include <vector>
#include <fstream>
#include <string>
#include <unistd.h>
#include <sys/time.h>
#include "sql.h"

class fisherman;

struct _args {
  message msg;
  fisherman *server;
  sockaddr_in client_addr;
  void reset(_args *args) {
    msg = args->msg;
    server = args->server;
    client_addr = args->client_addr;
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
    const char *host, 
    const int port, 
    const int max_requests,
    const char *sqlhost,
    const char *sqluser,
    const char *sqlpassword);
  ~fisherman();
  void start(const int max_listeners = 10);

  int server_sockfd;
  UDPSocket *udpserver;
  sql *db;
  bool keep_serving = true;
  tarray<user> user_map;
  tarray<file> file_map;
  tarray<conversation> conv_map;
  std::map<std::string, int> username_map;
  std::vector<interface_func> interface_map;
  threadpool *client_listeners;
  threadpool *requests_handler;
};

#endif