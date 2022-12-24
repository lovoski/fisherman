cc=g++
flags=-Wall -g -I.
std=-std=c++17
exe=fisherman
libs=-lmysqlclient
obj=main.o fisherman.o threadpool.o

target : $(obj)
	$(cc) $(flags) $(std) -o $(exe) $(obj) $(libs)

main.o : main.cpp
fisherman.o : fisherman.cpp
threadpool.o : threadpool.cpp

clean :
	rm -rf *.o $(exe)