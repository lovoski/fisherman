#ifndef COMMON_H
#define COMMON_H

#include "threadpool.h"
#include "socket.h"

// 1024 bytes
const int message_max_len = 1024;
struct message {
  char uid[4];
  // interface_func number
  char inno[4];
  char content[message_max_len-8];
};
struct user {
  int uid;
  char username[200];
  char password[200];
  // true when login approved
  // set to false when quit
  bool approved_online;
  // 0 -> root
  // 1 -> admin
  // 2 -> visitor
  int privillege;
  // conversation list of each user
  std::vector<int> conv_list;
  sockaddr_in client_addr;
};
struct file {
  int fid;
  char *filename;
  // file shouldn't be store 
  // in database directly
  char *filepath;
  long long filesize;
};
struct conversation {
  int cid;
  char convname[200];
  std::vector<int> members;
  std::vector<int> files;
  char historypath[200];
  int capacity;
  pthread_mutex_t *mtx;
  conversation() {
    mtx = new pthread_mutex_t;
    pthread_mutex_init(mtx, NULL);
  }
  ~conversation() {
    pthread_mutex_destroy(mtx);
  }
};

template<typename T>
class tarray {
public:
  tarray(const int size = 50);
  ~tarray();
  int size();
  void resize(const int nsize);
  void insert(const T &ele);
  void insert(const int index, const T &ele);
  T &operator[](const int index);
private:
  int m_size, m_limit, m_cur_index;
  T *m_data;
};

template<typename T>
tarray<T>::tarray(const int size) {
  m_size = size;
  m_limit = 2 * size;
  m_cur_index = 0;
  m_data = (T *)malloc(m_limit * sizeof(T));
}
template<typename T>
tarray<T>::~tarray() {
  delete[] m_data;
}
template<typename T>
void tarray<T>::resize(const int nsize) {
  if (nsize < m_limit) return;
  m_limit = nsize;
  m_data = (T *)realloc(m_data, nsize * sizeof(T));
}
template<typename T>
void tarray<T>::insert(const T &ele) {
  if (m_cur_index >= m_limit) {
    m_limit *= 2;
    m_data = (T *)realloc(m_data, m_limit * sizeof(T));
  }
  m_data[m_cur_index] = ele;
  m_cur_index++;
  m_size++;
}
template<typename T>
void tarray<T>::insert(const int index, const T &ele) {
  if (index >= m_limit) {
    m_limit = 2 * (index+1);
    m_data = (T *)realloc(m_data, m_limit * sizeof(T));
  }
  memcpy(&m_data[index], &ele, sizeof(T));
  m_cur_index = index;
  m_size++;
}
template<typename T>
T &tarray<T>::operator[](const int index) {
  return m_data[index];
}
template<typename T>
int tarray<T>::size() {return m_cur_index+1;}


#endif