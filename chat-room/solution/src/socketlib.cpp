#include "socketlib.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#include <string>

void init_address(struct sockaddr_in* address, const char* ip_address, int port) {
  address->sin_family = AF_INET;
  address->sin_addr.s_addr = INADDR_ANY;
  address->sin_port = htons(port);
}

int create_socket() {
  int file_descriptor;
  if ((file_descriptor = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  // We add the option "SO_REUSE_ADDR" to prevent error such as: “address already in use”.
  const int t = 1;
  if (setsockopt(file_descriptor, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &t, sizeof(t))) {
    perror("setsockopt failed");
    exit(EXIT_FAILURE);
  }
  return file_descriptor;
}

int connect_socket(int sock, const char* ip_address, int port) {
  struct sockaddr_in serv_addr;
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);
  // Convert IPv4 and IPv6 addresses from text to binary form
  if (inet_pton(AF_INET, ip_address, &serv_addr.sin_addr) < 0) {
    perror("Invalid address/address not supported");
    return -1;
  }

  if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
    perror("Socket connection failed \n");
    return -1;
  }
  return 0;
}

void bind_socket(int socket, int port) {
  struct sockaddr_in address;
  init_address(&address, "", port);

  if (bind(socket, (struct sockaddr*)&address, sizeof(address)) < 0) {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }
}

void listen_socket(int socket) {
  if (listen(socket, 3) < 0) {
    perror("listen");
    exit(EXIT_FAILURE);
  }
}

int accept_socket(int socket, struct sockaddr* remote_host) {
  unsigned addr_len = sizeof(remote_host);
  int new_socket;
  if ((new_socket = accept(socket, remote_host, &addr_len)) < 0) {
    perror("accept");
    exit(EXIT_FAILURE);
  }
  return new_socket;
}

void to_ip_host(struct sockaddr* s, char** ip, uint16_t* port) {
  struct sockaddr_in* sin = (struct sockaddr_in*)s;
  char* addr = inet_ntoa(sin->sin_addr);
  *ip = addr;
  *port = htons(sin->sin_port);
}

int safe_read(int socket, void* buffer, size_t nbytes) {
  int num_read = read(socket, buffer, nbytes);
  if (num_read == 0) {
    printf("Connection closed by remote host\n");
  } else if (num_read < 0) {
    perror("Error while reading from socket");
    exit(1);
  }
  return num_read;
}

int safe_write(int socket, const void* buffer, size_t nbytes) {
  unsigned num_written = 0;
  while (num_written < nbytes) {
    int written = write(socket, buffer, nbytes);
    if (written < 0) {
      perror("Error while writing to socket");
      return written;
    }
    num_written += written;
  }
  return num_written;
}

/**
 * @brief Receive data under the form <size_t len><data...>.
 */
size_t receive(int sock, message_t* dest) {
  size_t nbytes_to_receive;
  if (checked(read(sock, &nbytes_to_receive, sizeof(nbytes_to_receive))) == 0) {
    // Connection closed
    return 0;
  };
  char* buffer = (char*)malloc(nbytes_to_receive);
  if (buffer == NULL) {
    fprintf(stderr, "malloc could not allocate %zd bytes", nbytes_to_receive);
    perror("");
    exit(1);
  }
  // Receive timestamp (ignored)
  checked(read(sock, &dest->timestamp, sizeof(time_t)));
  size_t total_received = 0;
  while (nbytes_to_receive > 0) {
    size_t received = checked(read(sock, &buffer[total_received], nbytes_to_receive));
    if (received < 0) {
      return total_received;
    }
    total_received += received;
    nbytes_to_receive -= received;
  }
  buffer[total_received] = '\0';
  dest->message = std::string(buffer);
  free(buffer);
  return total_received;
}

/**
 * @brief Send data under the form <size_t len><...data>
 * Function name is 'ssend' instead of 'send' because the latter already exists.
 */
int ssend(int sock, message_t* msg) {
  size_t len = msg->message.length();
  checked(write(sock, &len, sizeof(len)));
  checked(write(sock, &msg->timestamp, sizeof(msg->timestamp)));
  return checked(write(sock, msg->message.c_str(), len));
}

int _checked(int ret, std::string calling_function) {
  if (ret < 0) {
    perror(calling_function.c_str());
    exit(EXIT_FAILURE);
  }
  return ret;
}