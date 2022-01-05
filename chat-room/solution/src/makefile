FLAGS=-Wall -Werror

all: client server 

client: client.cpp socketlib.o client.h
	g++ -o client client.cpp socketlib.o socketlib.h client.h ${FLAGS} -pthread

server: server.cpp socketlib.o server.h
	g++ -g -o server server.cpp socketlib.o socketlib.h ${FLAGS}

socketlib.o: socketlib.cpp socketlib.h
	g++ -c socketlib.c ${FLAGS}

clean:
	rm -f client server *.o

clear: clean