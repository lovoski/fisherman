# Introduction

server of an online chatting room based on udp

## environment

- Ubuntu 22.04
- g++ 11.3.0
- libmysqlclient-dev 8.0.31

## compile

```bash
cd src && make
```

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