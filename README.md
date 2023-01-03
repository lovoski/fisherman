# Introduction

server of an online chatting room based on udp

## environment

- Ubuntu 22.04
- g++ 11.3.0
- libmysqlclient-dev 8.0.31

## compile & deploy
- how to compile
  ```bash
  cd src && make
  ```
- how to deploy<br>
  the usage should be `fisherman <localhost> <max requests concurrently>`, the `localhost` must be the ip address of the server in it's local network, so that the service can be accessed from other virtual machines of actual clients. For example, if you plan on starting the server on wsl2 and open a client on windows, the `localhost` must be the ip address of wsl2 rather than `127.0.0.1` or `localhost`, the client in windows should also access the service through the actual ip.

## dependencies

- libmysqlclient:
  ```bash
  sudo apt install libmysqlclient-dev
  ```

## interfaces (developing)

- test_connect
  ```
  msg -> {4(uid), 4(inno), 1016(...)}
  ret -> {4(uid), 4(inno), 1016("server online")}
  ```
- login
  ```
  msg -> {4(uid), 4(inno), 1016(200(username), ...(password))}
  ret -> {4(uid), 4(inno), 1016(4(status_code), ...)}
  ```
- quit
  ```
  msg -> {4(uid), 4(inno), 1016(...)}
  ```
- broadcast
  ```
  msg -> {4(uid), 4(inno), 1016(4(cid), 4(timestamp), ...(message))}
  ret -> {4(uid), 4(inno), 1016(4(cid), 4(timestamp), ...("message sent"))}
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