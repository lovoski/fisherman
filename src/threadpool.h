#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <pthread.h>
#include <cstdio>
#include <queue>
#include <vector>

struct task {
  void *(*call_func)(void *);
  void *args;
};

void *worker_func(void *args);
class threadpool {
  friend void *worker_func(void *args);
public:
  threadpool(const size_t size);
  ~threadpool();
  void append_task(const task &t);
private:
  std::vector<pthread_t> workers;
  // shared variables
  std::queue<task> tasks;
  pthread_mutex_t mtx;
  pthread_cond_t cond;
  bool destroy = false;
  int available;
};

#endif