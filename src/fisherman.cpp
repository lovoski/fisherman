#include "fisherman.h"

inline int atoi_assigned_len(
  const char *str, 
  const int len) {
  char *t_str = (char *)str;
  t_str[len] = 0;
  return atoi(t_str);
}

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
  user_map = new user[50];
  user_map[0] = {0, "a", "123", true};
  user_map[1] = {1, "b", "123", true};
  user_map[2] = {2, "c", "123", true};
  username_map.insert({"a", 0});
  username_map.insert({"b", 1});
  username_map.insert({"c", 2});
  // build file map
  // ...
  // build conversation map
  // load history of each conversation, set up the file stream
  conversation lobby;
  conv_map = new conversation[200];
  conv_map[0].cid = 0;
  conv_map[0].members.push_back(0);
  conv_map[0].members.push_back(1);
  conv_map[0].members.push_back(2);
  // start client_listening
  _args args;
  args.server = this;
  for (int i = 0; i < max_listeners; ++i) 
  {
//    printf("建立第%d个连接\n",i+1);
    client_listeners->append_task({client_listening, &args});
  }
  while (keep_serving);
}

void *client_listening(void *args) {
  fisherman *param = ((_args *)args)->server;
  struct sockaddr client_sockaddr;
  socklen_t addrlen = sizeof(client_sockaddr);
  message msg;
  while (true) {
    printf("[pending receiving message]\n");
    if (recvfrom(param->server_sockfd, &msg, message_max_len, 0, (struct sockaddr *)&client_sockaddr, &addrlen) == -1) {
      perror("recvfrom error\n");
      exit(1);
    }
    else
    {
      printf("成功接收！\n");
      // for(int i=0;i<4;i++)
      //   std::cout<<msg.uid[i];
       printf("%d %d\n",substr_int(msg.uid,0,4),substr_int(msg.inno,0,4));
    }
    param->user_map[substr_int(msg.uid,0,4)].addr = client_sockaddr;
    _args inargs = {msg, param};
    printf("[receive request] interface number:%d\n", substr_int(msg.inno,0,4));
    param->requests_handler->append_task({param->interface_map[substr_int(msg.inno,0,4)], &inargs});
  }
}
int substr_int(char *c,int l,int r){
  int num=0;
  for(int i=l;i<r;i++)
  {
    num=num*10+c[i]-'0';
  }
  return num;
}
void *test_connect(void *args) {
  _args *tc = (_args *)args;
  message tmp_msg;
  const char *reply = "server online";
  memcpy(tmp_msg.uid, tc->msg.uid, sizeof(tc->msg.uid));
  memcpy(tmp_msg.inno, tc->msg.inno, sizeof(tc->msg.inno));
  memcpy(tmp_msg.content, reply, sizeof(reply));
//  std::cout<<substr_int(tc->msg.uid,0,4)<<" "<<substr_int(tc->msg.inno,0,4)<<std::endl;
  if (sendto(tc->server->server_sockfd,reply, message_max_len, 0, 
  (struct sockaddr *)&tc->server->user_map[substr_int(tc->msg.uid,0,4)].addr, sizeof(sockaddr_in)) == -1) {
    perror("sendto error\n");
    exit(1);
  }
  return NULL;
}

void *login(void *args) {
  _args *ls = (_args *)args;
  message tmp_msg;
  const char *reply = "server online";
  memcpy(tmp_msg.uid, ls->msg.uid, sizeof(ls->msg.uid));
  memcpy(tmp_msg.inno, ls->msg.inno, sizeof(ls->msg.inno));
  fisherman *sv = ls->server;
  int status_code;
  if (sv->username_map.find(std::string(ls->msg.content)) != sv->username_map.end()) {
    if (sv->user_map[sv->username_map[ls->msg.content]].password == (ls->msg.content+200)) {
      // approved login, update user state
      printf("[user login] username:%s\n", (ls->msg.content+200));
      status_code == 0;
    } else {
      // reject login
      printf("[rejcect user login] username:%s\n", (ls->msg.content+200));
      status_code = 1;
    }
  } else {
    // create account for user
    printf("[register new user] username:%s\n", (ls->msg.content+200));
    status_code = 2;
  }
  memcpy(tmp_msg.content, &status_code, sizeof(status_code));
  if (sendto(ls->server->server_sockfd, tmp_msg.uid, message_max_len, 0, 
  (struct sockaddr *)&ls->server->user_map[substr_int(ls->msg.uid,0,4)].addr, sizeof(sockaddr_in)) == -1) {
    perror("sendto error\n");
    exit(1);
  }
  return NULL;
}

void *quit(void *args) {
  return NULL;
}

void *broadcast(void *args) {
  _args *bc = (_args *)args;
  int sender_uid = substr_int(bc->msg.uid,0,4);
  conversation *conv = &bc->server->conv_map[atoi_assigned_len(bc->msg.content, 4)];
  pthread_mutex_lock(&conv->mtx);
  for (auto i : conv->members) {
    if (i == sender_uid) {
      message tmp_msg;
      const char *reply = "message sent";
      memcpy(tmp_msg.uid, bc->msg.uid, sizeof(bc->msg.uid));
      memcpy(tmp_msg.inno, bc->msg.inno, sizeof(bc->msg.inno));
      memcpy(tmp_msg.content, reply, sizeof(reply));
      if (sendto(bc->server->server_sockfd, tmp_msg.uid, message_max_len, 0, 
      (struct sockaddr *)&bc->server->user_map[i].addr, sizeof(sockaddr_in)) == -1) {
        perror("sendto error\n");
        exit(1);
      }
    } else {
      if (sendto(bc->server->server_sockfd, bc->msg.uid, message_max_len, 0, 
      (struct sockaddr *)&bc->server->user_map[i].addr, sizeof(sockaddr_in)) == -1) {
        perror("sendto error\n");
        exit(1);
      }
    }
  }
  pthread_mutex_unlock(&conv->mtx);
  return NULL;
}

void *file_upload(void *args) {return NULL;}

void *file_list(void *args) {return NULL;}

void *delete_file(void *args) {return NULL;}

void *file_download(void *args) {return NULL;}

void *conversation_list(void *args){return NULL;}

void *create_conversation(void *args) {//0009
  _args *cc = (_args *)args;
  int cid = substr_int(cc->msg.content,0,4);
  conversation *conv = &cc->server->conv_map[cid];
  conv->cid = cid;
  conv->members.push_back(substr_int(cc->msg.uid,0,4));
  if (sendto(cc->server->server_sockfd,"Build group successfully!", message_max_len, 0, 
  (struct sockaddr *)&cc->server->user_map[substr_int(cc->msg.uid,0,4)].addr, sizeof(sockaddr_in)) == -1) {
    perror("sendto error\n");
    exit(1);
  }
  return NULL;
}

void *modify_conversation(void *args) {return NULL;}
