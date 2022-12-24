# Introduction

server of an online chatting room

## environment

- Ubuntu 22.04
- g++ 11.3.0
- libmysqlclient-dev 8.0.31

## dependencies

- libmysqlclient:
  ```bash
  sudo apt install libmysqlclient-dev
  ```

## interfaces (developing)

- test_connect
  ```

  ```
- login
  ```

  ```
- quit
  ```

  ```
- broadcast
  ```

  ```
- file_upload
  ```
  1: msg -> {4(uid), 4(inno), 1016(4(cid), 8(segementnum), ...(filename))}
  2: msg -> {4(uid), 4(inno), 1016(4(cid), ...(file))} * segementnum
  ```
- file_list
  ```

  ```
- file_download
  ```

  ```
- conversation_list
  ```

  ```