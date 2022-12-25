#include "fisherman.h"

template<typename T>
array<T>::array(const int size) {
  m_size = size;
  m_limit = 2 * size;
  m_cur_index = 0;
  m_data = (T *)malloc(m_limit * sizeof(T));
}
template<typename T>
array<T>::~array() {
  delete[] m_data;
}
template<typename T>
void array<T>::resize(const int nsize) {
  if (nsize < m_limit) return;
  m_limit = nsize;
  m_data = (T *)realloc(m_data, nsize * sizeof(T));
}
template<typename T>
void array<T>::insert(const T &ele) {
  if (m_cur_index >= m_limit) {
    m_limit *= 2;
    m_data = (T *)realloc(m_data, m_limit * sizeof(T));
  }
  m_data[m_cur_index] = ele;
  m_cur_index++;
  m_size++;
}
template<typename T>
T &array<T>::operator[](const int index) {
  return m_data[index];
}
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
inline int mstrlen(const char *s) {
  int ret = 0;
  while (s[ret] != 0) ret++;
  return ret;
}
bool mstrcmp(const char *s1, const char *s2) {
  int l1 = mstrlen(s1), l2 = mstrlen(s2);
  if (l1 != l2) return false;
  for (int i = 0; i < l1; ++i) {
    if (s1[i] != s2[i]) return false;
  }
  return true;
}
static void debug_mem(char *mem, int len, const char * info) {
  printf("%s", info);
  for (int i = 0; i < len; ++i)
    printf("%d,", mem[i]);
  printf("\n");
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
    printf("[error] error creating serverfd\n");
    exit(1);
  }
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = inet_addr(host);
  serveraddr.sin_port = htons(port);
  if (bind(server_sockfd, (struct sockaddr *)&serveraddr, addrlen) == -1) {
    printf("[error] error binding socket\n");
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
  conv_map.resize(200);
  conv_map[0].cid = 0;
  conv_map[0].members.push_back(0);
  conv_map[0].members.push_back(1);
  conv_map[0].members.push_back(2);
  // start client_listening
  _args args;
  args.server = this;
  for (int i = 0; i < max_listeners; ++i) {
    client_listeners->append_task({client_listening, &args});
  }
  while (keep_serving);
}

void *client_listening(void *args) {
  fisherman *param = ((_args *)args)->server;
  struct sockaddr_in client_sockaddr;
  socklen_t addrlen = sizeof(client_sockaddr);
  while (true) {
    message msg;
    memset(&msg, 0, message_max_len);
    printf("[wait for msg]\n");
    if (recvfrom(param->server_sockfd, &msg, message_max_len, 0, 
    (struct sockaddr *)&client_sockaddr, &addrlen) == -1) {
      printf("[error] recvfrom error\n");
    }
    _args inargs = {msg, param, client_sockaddr};
    printf("[receive request] interface number:%d\n", argv_to_int32(msg.inno, 4));
    param->requests_handler->append_task({param->interface_map[argv_to_int32(msg.inno, 4)], &inargs});
  }
}

void *test_connect(void *args) {
  _args *tc = (_args *)args;
  message tmp_msg;
  const char *reply = "server online";
  memcpy(tmp_msg.uid, tc->msg.uid, sizeof(tc->msg.uid));
  memcpy(tmp_msg.inno, tc->msg.inno, sizeof(tc->msg.inno));
  memcpy(tmp_msg.content, reply, sizeof(reply));
  if (sendto(tc->server->server_sockfd, &tmp_msg, message_max_len, 0, 
  (struct sockaddr *)&tc->client_addr, sizeof(sockaddr_in)) == -1) {
    printf("[error] sendto error\n");
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
  if (sv->username_map.find(std::string(ls->msg.content+200)) != sv->username_map.end()) {
    int uid = sv->username_map[ls->msg.content];
    // debug_mem(sv->user_map[uid].password, 10, "server:");
    // debug_mem(ls->msg.content, 10, "client:");
    if (mstrcmp(sv->user_map[uid].password, ls->msg.content)) {
      // approved login, update user state
      // reject duplicate login
      if (sv->user_map[uid].approved_online)
        goto __reject;
      sv->user_map[uid].approved_online = true;
      printf("[user login] username:%s\n", (ls->msg.content+200));
      // notify other users
      status_code == 0;
    } else {
      __reject:
      // reject login
      sv->user_map[uid].approved_online = false;
      printf("[rejcect user login] username:%s\n", (ls->msg.content+200));
      status_code = 1;
    }
  } else {
    // create account for user
    printf("[register new user] username:%s\n", (ls->msg.content+200));
    status_code = 2;
  }
  tmp_msg.content[3] = status_code;
  if (sendto(ls->server->server_sockfd, &tmp_msg, message_max_len, 0, 
  (struct sockaddr *)&ls->client_addr, sizeof(sockaddr_in)) == -1) {
    printf("[error] sendto error\n");
  }
  return NULL;
}

void *quit(void *args) {
  _args *q = (_args *)args;
  int uid = argv_to_int32(q->msg.uid, 4);
  q->server->user_map[uid].approved_online = false;
  printf("[user quit] username:%s\n", (q->msg.content+200));
  return NULL;
}

void *broadcast(void *args) {
  _args *bc = (_args *)args;
  int sender_uid = argv_to_int32(bc->msg.uid, 4);
  conversation *conv = &bc->server->conv_map[argv_to_int32(bc->msg.content, 4)];
  pthread_mutex_lock(&conv->mtx);
  for (auto i : conv->members) {
    if (i == sender_uid) {
      message tmp_msg;
      const char *reply = "message sent";
      memcpy(tmp_msg.uid, bc->msg.uid, sizeof(bc->msg.uid));
      memcpy(tmp_msg.inno, bc->msg.inno, sizeof(bc->msg.inno));
      memcpy(tmp_msg.content, reply, sizeof(reply));
      if (sendto(bc->server->server_sockfd, tmp_msg.uid, message_max_len, 0, 
      (struct sockaddr *)&bc->client_addr, sizeof(sockaddr_in)) == -1) {
        printf("[error] sendto error\n");
      }
    } else {
      if (sendto(bc->server->server_sockfd, bc->msg.uid, message_max_len, 0, 
      (struct sockaddr *)&bc->client_addr, sizeof(sockaddr_in)) == -1) {
        printf("[error] sendto error\n");
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

void *conversation_list(void *args) {return NULL;}

void *create_conversation(void *args) {return NULL;}

void *modify_conversation(void *args) {return NULL;}
