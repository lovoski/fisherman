#include "fisherman.h"

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

fisherman::fisherman(
  const char *host,
  const int port,
  const int max_requests,
  const char *sqlhost,
  const char *sqluser,
  const char *sqlpassword
  ) {
  requests_handler = new threadpool(max_requests);
  udpserver = new UDPSocket(host, port);
  db = new sql(sqlhost, sqluser, sqlpassword);
}
fisherman::~fisherman() {
  delete client_listeners;
  delete requests_handler;
  delete udpserver;
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
  db->load_all_users(user_map, username_map);
  // build file map
  // ...
  // build conversation map
  // load history of each conversation, set up the file stream
  db->load_all_conversation(conv_map);
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
  struct sockaddr_in client_addr;
  socklen_t addrlen = sizeof(client_addr);
  while (true) {
    message msg;
    memset(&msg, 0, message_max_len);
    printf("[wait for msg]\n");
    param->udpserver->recvFrom(&msg, message_max_len, client_addr);
    _args inargs = {msg, param, client_addr};
    printf("[receive request] interface number:%d\n", argv_to_int32(msg.inno, 4));
    //printf("[receive request] user number:%d\n", argv_to_int32(msg.uid, 4));
    int inno = argv_to_int32(msg.inno, 4);
    if (inno < param->interface_map.size()) {
      param->requests_handler->append_task({param->interface_map[inno], &inargs});
    }
  }
}

void *test_connect(void *args) {//0000
  setbuf(stdout,NULL);
  _args *tc = (_args *)args;
  message tmp_msg;
  const char *reply = "server online";
  memcpy(tmp_msg.uid, tc->msg.uid, sizeof(tc->msg.uid));
  memcpy(tmp_msg.inno, tc->msg.inno, sizeof(tc->msg.inno));
  memcpy(tmp_msg.content, reply, strlen(reply));
  tc->server->udpserver->sendTo(&tmp_msg, message_max_len, tc->client_addr);
  return NULL;
}

void *login(void *args) {//0001
  _args *ls = (_args *)args;
  message tmp_msg;
  fisherman *sv = ls->server;
  int status_code;
  const char *reply = "server online";
  memcpy(tmp_msg.inno, ls->msg.inno, sizeof(ls->msg.inno));
  if (sv->username_map.find(std::string(ls->msg.content+200)) != sv->username_map.end()) {
    int uid = sv->username_map[ls->msg.content+200];
    if (uid == 0) // reject sys user to login
      goto __reject;
    int32_to_argv(uid, tmp_msg.uid); // copy uid to ret msg
    // debug_mem(sv->user_map[uid].password, 10, "server:");
    // debug_mem(ls->msg.content, 10, "client:");
    if (mstrcmp(sv->user_map[uid].password, ls->msg.content)) {
      // approved login, update user state
      // reject duplicate login
      if (sv->user_map[uid].approved_online)
        goto __reject;
      sv->user_map[uid].approved_online = true;
      sv->user_map[uid].client_addr = ls->client_addr;
      printf("[user login] username:%s\n", (ls->msg.content+200));
      status_code == 0;
    } else {
      __reject:
      // reject login
      // sv->user_map[uid].approved_online = false;
      printf("[rejcect user login] username:%s\n", (ls->msg.content+200));
      status_code = 1;
    }
  } else {
    // create account for user
    // assign new uid
    int n_uid = ls->server->user_map.size();
    user n_user;
    n_user.approved_online = true;
    n_user.client_addr = ls->client_addr;
    memcpy(n_user.password, ls->msg.content, mstrlen(ls->msg.content));
    memcpy(n_user.username, ls->msg.content+200, mstrlen(ls->msg.content+200));
    n_user.uid = n_uid;
    n_user.privillege = 2;
    n_user.conv_list.push_back(0);

    // append user to global
    ls->server->username_map.insert({std::string(ls->msg.content+200), n_uid});
    ls->server->user_map.insert(n_user);
    ls->server->db->append_new_user(n_user);

    // add to default lobby
    pthread_mutex_lock(ls->server->conv_map[0].mtx);
    ls->server->conv_map[0].members.push_back(n_uid);
    pthread_mutex_unlock(ls->server->conv_map[0].mtx);

    printf("[register new user] username:%s\n", (ls->msg.content+200));
    status_code = 2;
  }
  int32_to_argv(status_code, tmp_msg.content);
  // return username to user
  memcpy(tmp_msg.content+4, ls->msg.content+200, mstrlen((ls->msg.content+200)));
  ls->server->udpserver->sendTo(&tmp_msg, message_max_len, ls->client_addr);
  return NULL;
}

void *quit(void *args) {
  _args *q = (_args *)args;
  int uid = argv_to_int32(q->msg.uid, 4);
  q->server->user_map[uid].approved_online = false;
  printf("[user quit] username:%s\n", (q->msg.content));
  return NULL;
}

void *broadcast(void *args) {//0003
  _args *bc = (_args *)args;
  int sender_uid = argv_to_int32(bc->msg.uid, 4);
  int cid = argv_to_int32(bc->msg.content, 4); // find destinated conversation
  auto members = bc->server->conv_map[cid].members;
  pthread_mutex_lock(bc->server->conv_map[cid].mtx);
  for (auto i : bc->server->conv_map[cid].members) {
    if (i != sender_uid && bc->server->user_map[i].approved_online) { // don't reply to the sender
      bc->server->udpserver->sendTo(&bc->msg, message_max_len, bc->server->user_map[i].client_addr);
    }
  }
  pthread_mutex_unlock(bc->server->conv_map[cid].mtx);
  return NULL;
}

void *file_upload(void *args) {return NULL;}

void *file_list(void *args) {return NULL;}

void *delete_file(void *args) {return NULL;}

void *file_download(void *args) {return NULL;}

void *conversation_list(void *args) {return NULL;}

void *create_conversation(void *args){return NULL;}

void *modify_conversation(void *args) {return NULL;}