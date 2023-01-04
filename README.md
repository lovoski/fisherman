# Introduction

server of an online chatting room based on udp

## client repo

- https://github.com/lovoski/anemonefish

## environment

- debian 10.2
- mysql 5.7
- libmysqlclient-dev 8.0

## compile & deploy
- how to compile
  ```bash
  cd src && make
  ```
- how to deploy<br>
  the project now uses `nlohmann json` as conf reader, to run the server, simply `./fisherman`, but the file `conf.json` should be completed first, the `"mysql_password"` is compulsory for now, only a machine that is ready for mysql root user can execute the binary successfully. the `"localip"` should be the ip address of the server in it's local domain, in ubuntu, the ip can be obtained by `ifconfig`, for example, if you need to access the server deployed in wsl2 from windows, the `localip` should be the exact ip of wsl2 rather than `127.0.0.1` or `localhost`.

## dependencies

- libmysqlclient:
  ```bash
  sudo apt install libmysqlclient-dev
  ```

## interfaces (developing)

### interface map

| interface name | interface number (inno) | usage |
| :------------: | :---------------------: | :-----: |
|test_connect|0|test if the client is still online|
|login|1|log in and sign up, create user if username is not found, check password and online status if username is found. all new users will be invited into a default conversation named "default lobby"|
|quit|2|let server knows the client is offline|
|broadcast|3|broadcast client's message to specified conversation|
|file_upload|4|  |
|file_list|5|  |
|delete_file|6|  |
|file_download|7|  |
|conversation_list|8|  |
|create_conversation|9|  |
|modify_conversation|10|  |

### detail of interfaces

- test_connect
  ```
  msg -> {4(uid), 4(inno), 1016(...)}
  ret -> {4(uid), 4(inno), 1016("server online")}
  ```
- login
  ```
  msg -> {4(uid), 4(inno), 1016(200(username), ...(password))}
  ret -> {4(uid), 4(inno), 1016(4(status_code), ...(username))}
  ```
- quit
  ```
  msg -> {4(uid), 4(inno), 1016(...(username))}
  ```
- broadcast
  ```
  msg -> {4(uid), 4(inno), 1016(4(cid), ...(message))}
  ```
- file_upload
  ```
  msg -> {4(uid), 4(inno), 1016(4(cid), 4(timestamp), 8(segementnum), ...(filename))}
  ret -> {4(uid), 4(inno), 1016(4(cid), ...)}
  msg -> {4(uid), 4(inno), 1016(4(cid), ...(file))} * segementnum
  ret -> {4(uid), 4(inno), 1016(4(cid), 8(segementnum), ...(filename))}
  ```
- file_list
  ```
  msg -> {4(uid), 4(inno), 1016(4(cid), ...)}
  ret -> {4(uid), 4(inno), 1016(4(cid), 4(fid), 4(timestamp), ...(filename))} * filenum
  ```
- delete_file
  ```
  msg -> {4(uid), 4(inno), 1016(4(cid), 4(fid), ...)}
  ret -> {4(uid), 4(inno), 1016(4(status_code))}
  ```
- file_download
  ```
  msg -> {4(uid), 4(inno), 1016(4(cid), 4(fid), ...)}
  ret -> {4(uid), 4(inno), 1016(4(cid), 4(timestamp), 8(segementnum), ...(filename))}
  ret -> {4(uid), 4(inno), 1016(4(cid), ...(file))} * segementnum
  ```
- conversation_list
  ```
  msg -> {4(uid), 4(inno), 1016(4(cid), ...)}
  ret -> {4(uid), 4(inno), 1016(8(conversationnum), ...)}
  ret -> {4(uid), 4(inno), 1016(4(cid), ...(conversationname))} * conversationnum
  ```
- create_conversation
  ```
  msg -> {4(uid), 4(inno), 1016(8(membernum), ...(conversationname))}
  ret -> {4(uid), 4(inno), 1016(4(cid), ...)}
  ```
- modify_conversation
  ```
  msg -> {4(uid), 4(inno), 1016(4(cid), 1(modoption), 8(modmembernum), ...(conversationname))}
  ret -> {4(uid), 4(inno), 1016(4(cid), ...)}
  msg -> {4(uid), 4(inno), 1016(4(cid),4(uid_member) * ...)} * ...
  ret -> {4(uid), 4(inno), 1016(4(status_code), ...)}
  ```