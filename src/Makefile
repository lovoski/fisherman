cc=g++
flags=-Wall -g -I.
std=-std=c++17
exe=fisherman
libs=-lmysqlclient
obj=main.o fisherman.o threadpool.o sql.o socket.o

target : $(obj)
	$(cc) $(flags) $(std) -o $(exe) $(obj) $(libs)

main.o : main.cpp
sql.o : sql.cpp
threadpool.o : threadpool.cpp
fisherman.o : fisherman.cpp
socket.o : socket.cpp

clean :
	rm -rf *.o $(exe)