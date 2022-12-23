#pragma once

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

threadpool::threadpool(const size_t size) {
  available = size;
  pthread_mutex_init(&mtx, NULL);
  pthread_cond_init(&cond, NULL);
  workers.resize(size);
  for (auto i : workers) {
    pthread_create(&i, NULL, worker_func, this);
  }
}
threadpool::~threadpool() {
  destroy = true;
  pthread_mutex_lock(&mtx);
  for (auto i : workers) {
    pthread_join(i, NULL);
  }
  pthread_mutex_unlock(&mtx);
  pthread_mutex_destroy(&mtx);
}
void threadpool::append_task(const task &t) {
  pthread_mutex_lock(&mtx);
  if (!destroy)
    tasks.emplace(t);
  if (available > 0) { // only signal when available
    pthread_mutex_unlock(&mtx);
    pthread_cond_signal(&cond);
  }
}
void *worker_func(void *args) {
  threadpool *pool = (threadpool *)args;
  pthread_mutex_lock(&pool->mtx);
  while (true) {
    if (pool->tasks.empty()) { // waiting for tasks if there's none
      ++pool->available;
      // unlock -> wait -> lock
      int status = pthread_cond_wait(&pool->cond, &pool->mtx);
      --pool->available;
      if (status == 0) continue;
      else { // error, exit
        pthread_mutex_unlock(&pool->mtx);
        break;
      }
    } else { // there's tasks queuing
      task current_task = pool->tasks.front();
      pool->tasks.pop();
      pthread_mutex_unlock(&pool->mtx); // allow other threads to get task
      current_task.call_func(current_task.args);
      pthread_mutex_lock(&pool->mtx);
    }
  }
  return NULL;
}
