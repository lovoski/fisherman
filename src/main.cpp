#include "fisherman.h"
#include "3rd/json.hpp"

int main() {
  std::fstream conf_file;
  conf_file.open("conf.json", std::ios::in);
  nlohmann::json conf = nlohmann::json::parse(conf_file);
  std::string mysql_host = conf["mysql_host"].get<std::string>();
  std::string mysql_username = conf["mysql_username"].get<std::string>();
  std::string mysql_password = conf["mysql_password"].get<std::string>();
  std::string localip = conf["localip"].get<std::string>();
  int port = conf["port"].get<int>();
  int concurrency = conf["concurrency"].get<int>();
  if (!mysql_host.empty() && !mysql_username.empty() 
      && !mysql_password.empty() && !localip.empty()) {
    if (concurrency == 0) {
      printf("[error] can't set concurrency to 0\n");
      exit(1);
    } else {
      printf("[succeed] start server\n");
      fisherman server(localip.c_str(), 
                       port, 
                       concurrency, 
                       mysql_host.c_str(), 
                       mysql_username.c_str(), 
                       mysql_password.c_str());
      server.start();
    }
  } else {
    printf("[error] conf file incomplete\n");
    exit(1);
  }
  return 0;
}