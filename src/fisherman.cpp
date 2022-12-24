#include "fisherman.h"

fisherman::fisherman(
  const char *host,
  const int port,
  const int max_requests
  ) {
  requests_handler = new threadpool(max_requests);

  struct sockaddr_in serveraddr;
  socklen_t addrlen = sizeof(serveraddr);
  if ((server_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
    perror("error creating serverfd\n");
    exit(1);
  }
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = inet_addr(host);
  serveraddr.sin_port = htons(port);
  if (bind(server_sockfd, (struct sockaddr *)&serveraddr, addrlen) == -1) {
    perror("error binding socket\n");
    exit(1);
  }
}
fisherman::~fisherman() {
  delete client_listeners;
  delete requests_handler;
}
void fisherman::start(const int max_listeners) {
  client_listeners = new threadpool(max_listeners);
  // build interface map
  interface_map.resize(50);
  interface_map[0] = test_connect;
  interface_map[1] = login;
  interface_map[2] = quit;
  interface_map[3] = broadcast;
  interface_map[4] = file_upload;
  interface_map[5] = file_list;
  interface_map[6] = delete_file;
  interface_map[7] = file_download;
  interface_map[8] = conversation_list;
  interface_map[9] = create_conversation;
  interface_map[10] = modify_conversation;
  // build user map and username map
  user_map.push_back({0, "a", "123", true});
  user_map.push_back({1, "b", "123", true});
  user_map.push_back({2, "c", "123", true});
  username_map.insert({"a", 0});
  username_map.insert({"b", 1});
  username_map.insert({"c", 2});
  // build file map
  // ...
  // build conversation map
  conversation default_lobby;
  default_lobby.cid = 0;
  default_lobby.members.resize(3);
  default_lobby.members.push_back(0);
  default_lobby.members.push_back(1);
  default_lobby.members.push_back(2);
  // start client_listening
  _args args;
  args.server = this;
  for (int i = 0; i < max_listeners; ++i) {
    client_listeners->append_task({client_listening, &args});
  }
}
void *client_listening(void *args) {
  fisherman *param = ((_args *)args)->server;
  struct sockaddr client_sockaddr;
  socklen_t addrlen = sizeof(client_sockaddr);
  message msg;
  while (true) {
    if (recvfrom(param->server_sockfd, &msg, message_max_len, 0, (struct sockaddr *)&client_sockaddr, &addrlen) == -1) {
      perror("recvfrom error\n");
      exit(1);
    }
    _args inargs = {msg, param};
    param->requests_handler->append_task({param->interface_map[atoi(msg.inno)], &inargs});
  }
};
void *test_connect(void *args) {return NULL;}
void *login(void *args) {return NULL;}
void *quit(void *args) {return NULL;}
void *broadcast(void *args) {return NULL;}
void *file_upload(void *args) {return NULL;}
void *file_list(void *args) {return NULL;}
void *delete_file(void *args) {return NULL;}
void *file_download(void *args) {return NULL;}
void *conversation_list(void *args) {return NULL;}
void *create_conversation(void *args) {return NULL;}
void *modify_conversation(void *args) {return NULL;}
