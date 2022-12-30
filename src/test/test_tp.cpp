#include "../threadpool.h"
#include <cstdio>
#include <unistd.h>
#include <cmath>
#include <map>

typedef void *(*interface)(void *);
std::map<int, interface> m;

void *f1(void *args) {
  int *nums = (int *)args;
  printf("f1: %d\n", (nums[0] + nums[1]));
  return NULL;
}
void *f2(void *args) {
  int *max = (int *)args, cur = 0;
  for (int i = 0; i < *max; ++i) {
    cur += i;
  }
  printf("f2: %d\n", cur);
  return NULL;
}
void *f3(void *args) {
  printf("f3: fast f3\n");
  return NULL;
}

int main() {
  threadpool pool(20);
  int nums[4] = {1,2,3,4};
  m[0] = f1;
  m[1] = f2;
  m[2] = f3;
  pool.append_task({m[0], nums});
  pool.append_task({m[2], NULL});
  pool.append_task({m[0], nums+2});
  pool.append_task({m[1], nums+1});
  pool.append_task({m[0], nums+1});

  sleep(3);
  return 0;
}