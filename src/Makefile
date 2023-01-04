cc=g++
flags=-Wall -g -I.
std=-std=c++14
exe=fisherman
libs=-lmysqlclient
objs=main.o common.o threadpool.o sql.o socket.o fisherman.o

target : $(objs)
	$(cc) $(flags) $(std) -o $(exe) $(objs) $(libs)

main.o : main.cpp
common.o : common.cpp
sql.o : sql.cpp
threadpool.o : threadpool.cpp
socket.o : socket.cpp
fisherman.o : fisherman.cpp

clean :
	rm -rf *.o $(exe)