cc=g++
flags=-Wall -g -I.
std=-std=c++14
exe=fisherman
libs=-lmysqlclient
obj=main.o threadpool.o sql.o socket.o fisherman.o

target : $(obj)
	$(cc) $(flags) $(std) -o $(exe) $(obj) $(libs)

main.o : main.cpp
sql.o : sql.cpp
threadpool.o : threadpool.cpp
socket.o : socket.cpp
fisherman.o : fisherman.cpp

clean :
	rm -rf *.o $(exe)