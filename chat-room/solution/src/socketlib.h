#ifndef _SOCKETLIB_H
#define _SOCKETLIB_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>

#include <string>

typedef struct {
  time_t timestamp;
  std::string message;
} message_t;

/*
 * Socket initialization procedure
 *          Server            Client
 *          ------            ------
 *          create            create
 *            |                 |
 *            V                 |
 *          bind                |
 *            |                 |
 *            V                 |
 *          listen              |
 *            |                 |
 *            V                 V
 *          accept  <------- connect
 *            |                 |
 *            V                 V
 *          read/write <----> read/write
 *            |                 |
 *            V                 V
 *          close             close
 */

int _checked(int ret, std::string calling_function);

// The macro allows us to retrieve the name of the calling function
#define checked(call) _checked(call, #call)

void init_address(struct sockaddr_in* address, const char* ip_address, int port);
int create_socket();
int connect_socket(int sock, const char* ip_address, int port);
void bind_socket(int socket, int port);
void listen_socket(int socket);
int accept_socket(int socket, struct sockaddr* remote_host);
void to_ip_host(struct sockaddr* s, char** ip, uint16_t* port);
int safe_read(int socket, void* buffer, size_t nbytes);
int safe_write(int socket, const void* buffer, size_t nbytes);
/**
 * @brief Receive data under the form <size_t len><data...>.
 */
size_t receive(int sock, message_t* dest);
/**
 * @brief Send data under the form <size_t len><...data>
 * Function name is 'ssend' instead of 'send' because the latter already exists.
 */
int ssend(int sock, message_t* msg);

#endif  //_SOCKETLIB_H