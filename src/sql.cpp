#include "sql.h"

void error_close(MYSQL *sql) {
  fprintf(stderr, "%s\n", mysql_error(sql));
  mysql_close(sql);
  exit(1);
}

void mstrsplit_to_int32(const char *src, std::vector<int> &res) {
  res.clear();
  int len = strlen(src);
  int t = 1, tt = -1;
  for (int i = len-1; i>=0; --i) {
    if (src[i] == ';') {
      t = 1;
      if (tt != -1) res.push_back(tt);
      tt = 0;
    } else {
      tt += (src[i]-'0')*t;
      t *= 10;
    }
  }
}

sql::sql(
  const char *host,
  const char *user,
  const char *password) {
  conn = mysql_init(NULL);
  if (conn == NULL) {
    fprintf(stderr, "%s\n", mysql_error(conn));
    exit(1);
  }
  if (mysql_real_connect(conn, host, user, password, NULL, 0, NULL, 0) == NULL) {
    error_close(conn);
  }
  if (mysql_query(conn, "CREATE DATABASE IF NOT EXISTS fisherman")) error_close(conn);
  if (mysql_query(conn, "use fisherman")) error_close(conn);
  if (mysql_query(conn, "CREATE TABLE IF NOT EXISTS users(uid INT(8) NOT NULL PRIMARY KEY, username TEXT, password TEXT, conv_list TEXT, privillege INT)")) error_close(conn);
  if (mysql_query(conn, "CREATE TABLE IF NOT EXISTS convs(cid INT(8) NOT NULL PRIMARY KEY, convname TEXT, members TEXT, files TEXT, historypath TEXT, capacity INT)")) error_close(conn);
  if (mysql_query(conn, "SELECT * FROM users")) error_close(conn);
  MYSQL_RES *result = mysql_store_result(conn);
  MYSQL_ROW row;
  int rows_count = mysql_num_rows(result);
  if (rows_count == 0) {
    mysql_query(conn, "INSERT INTO users VALUES(0, 'sys', '20230103', ';0;', 0)");
    mysql_query(conn, "INSERT INTO convs VALUES(0, 'default lobby', ';0;', ';0;', '', 2000)");
  }
}
sql::~sql() {
  mysql_close(conn);
}
void sql::close() {
  mysql_close(conn);
}
void sql::append_new_user(user &nu) {
  char statement[600] = {0};
  sprintf(statement, "INSERT INTO users VALUES(%d, '%s', '%s', ';0;', %d)", nu.uid, nu.username, nu.password, nu.privillege);
  if (mysql_query(conn, statement)) error_close(conn);
  if (mysql_query(conn, "SELECT * FROM convs WHERE cid=0")) error_close(conn);
  MYSQL_RES *result = mysql_store_result(conn);
  MYSQL_ROW row;
  row = mysql_fetch_row(result);
  char nstatement[10000] = {0};
  sprintf(nstatement, "UPDATE convs SET members='%s%d;' WHERE cid=0", row[2], nu.uid);
  if (mysql_query(conn, nstatement)) error_close(conn);
}
void sql::append_new_conversation(conversation &conv) {
  char statement[600] = {0};
  char memberlist[200] = {0}, tmp[30] = {0};
  memberlist[0] = ';';
  for (auto i : conv.members) {
    sprintf(tmp, "%d;", i);
    strcat(memberlist, tmp);
  }
  sprintf(statement, "INSERT INTO convs VALUES(%d, '%s', '%s', ';0;', '', %d)", conv.cid, conv.convname, "", conv.capacity);
  if (mysql_query(conn, statement)) error_close(conn);
}

void sql::load_all_users(tarray<user> &user_map, std::map<std::string, int> &username_map) {
  if (mysql_query(conn, "SELECT * FROM users")) error_close(conn);
  MYSQL_RES *result = mysql_store_result(conn);
  MYSQL_ROW row;
  int fields = mysql_num_fields(result);
  while ((row = mysql_fetch_row(result))) {
    user nu;
    memset(&nu, 0, sizeof(user));
    nu.uid = atoi(row[0]);
    memcpy(nu.username, row[1], mstrlen(row[1]));
    memcpy(nu.password, row[2], mstrlen(row[2]));
    mstrsplit_to_int32(row[3], nu.conv_list);
    nu.privillege = atoi(row[4]);
    nu.approved_online = false;
    user_map.insert(nu.uid, nu);
    username_map.insert({user_map[nu.uid].username, nu.uid});
  }
}

void sql::load_all_files(tarray<file> &file_map) {}

void sql::load_all_conversation(tarray<conversation> &conv_map) {
  if (mysql_query(conn, "SELECT * FROM convs")) error_close(conn);
  MYSQL_RES *result = mysql_store_result(conn);
  MYSQL_ROW row;
  int fields = mysql_num_fields(result);
  while ((row = mysql_fetch_row(result))) {
    conversation nc;
    memset(nc.convname, 0, sizeof(nc.convname));
    memset(nc.historypath, 0, sizeof(nc.historypath));
    nc.cid = atoi(row[0]);
    memcpy(nc.convname, row[1], strlen(row[1]));
    mstrsplit_to_int32(row[2], nc.members);
    mstrsplit_to_int32(row[3], nc.files);
    memcpy(nc.historypath, row[4], strlen(row[4]));
    nc.capacity = atoi(row[5]);
    conv_map.insert(nc.cid, nc);
  }
}