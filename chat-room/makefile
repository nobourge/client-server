all: client server

client: client.c common.h
	gcc -Wall -o client client.c -lpthread

server: server.c common.h
	gcc -Wall -o server server.c