#ifndef SQL_H
#define SQL_H

#include <mysql/mysql.h>
#include <map>
#include <string>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include "common.h"

class sql {
public:
  sql(const char *host,
      const char *user,
      const char *password);
  ~sql();
  // add user to table users, update members of default_lobby
  void append_new_user(user &nu);
  void append_new_conversation(conversation &conv);
  void load_all_users(tarray<user> &user_map, std::map<std::string, int> &username_map);
  void load_all_files(tarray<file> &file_map);
  void load_all_conversation(tarray<conversation> &conv_map);
  void close();
private:
  MYSQL *conn;
};

#endif