#ifndef CHARUTILS_H
#define CHARUTILS_H

#include <cstring>
#include <cstdio>
#include <cstdlib>

inline int atoi_assigned_len(const char *str, const int len) {
  char *t_str = (char *)str;
  t_str[len] = 0;
  return atoi(t_str);
}

inline char *get_default_history_path(const int cid) {
  
}

#endif