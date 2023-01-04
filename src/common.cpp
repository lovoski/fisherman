#include "common.h"

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