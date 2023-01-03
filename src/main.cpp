#include "fisherman.h"

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("usage: fisherman <localhost> <max requests concurrently>\n");
    exit(1);
  } else {
    fisherman server(argv[1], 9958, atoi(argv[2]));
    server.start();
    return 0;
  }
}