#include "server.h"

#include <string.h>
#include <sys/select.h>
#include <unistd.h>

#include <ctime>
#include <iostream>
#include <string>

#include "socketlib.h"

Server::Server() {}

void Server::run(int port) {
  this->master_socket = checked(create_socket());
  bind_socket(this->master_socket, port);
  listen_socket(this->master_socket);
  printf("Server is waiting for new connections on port %d...\n", port);
  this->max_fd = master_socket;

  fd_set read_set;
  while (true) {
    this->prepateFDSet(&read_set);
    int nactivities = checked(select(this->max_fd + 1, &read_set, nullptr, nullptr, nullptr));
    this->handleSocketReadActivity(&read_set, nactivities);
  }
}

void Server::forward(message_t* msg) {
  for (unsigned i = 0; i < this->users.size(); i++) {
    user_t* u = this->users[i];
    if (ssend(u->socket, msg) <= 0) {
      this->disconnectUser(i);
    }
  }
}

void Server::prepateFDSet(fd_set* read_set) {
  FD_ZERO(read_set);
  FD_SET(this->master_socket, read_set);
  for (unsigned i = 0; i < this->users.size(); i++) {
    user_t* user = this->users[i];
    FD_SET(user->socket, read_set);
  }
}

void Server::handleSocketReadActivity(fd_set* in_set, int& nactivities) {
  if (nactivities <= 0) return;
  if (FD_ISSET(this->master_socket, in_set)) {
    this->handleNewConnection();
    nactivities--;
  }
  unsigned i = this->users.size() - 1;
  while (nactivities > 0 and i >= 0) {
    int socket = this->users[i]->socket;
    message_t msg;
    if (FD_ISSET(socket, in_set)) {
      nactivities--;
      size_t nbytes = receive(socket, &msg);
      if (nbytes < 0) {
        exit(1);
      } else if (nbytes == 0) {
        this->disconnectUser(i);
      } else {
        // message_buffer[nbytes] = '\0';
        char date_buffer[32];
        struct tm* local_time = localtime(&msg.timestamp);
        strftime(date_buffer, sizeof(date_buffer), "%H:%M:%S", local_time);
        msg.message = "[" + string(date_buffer) + "] " + this->users[i]->name + ": " + msg.message;
        this->forward(&msg);
      }
    }
    i--;
  }
}

void Server::disconnectUser(unsigned user_num) {
  user_t* user = this->users[user_num];
  printf("User %s has disconnected\n", user->name.c_str());
  this->users.erase(this->users.begin() + user_num);
  this->max_fd = this->master_socket;
  for (auto user : this->users) {
    if (user->socket > this->max_fd) {
      this->max_fd = user->socket;
    }
  }
}

void Server::handleNewConnection() {
  struct sockaddr remote_host;
  char username[64];

  int socket = accept_socket(this->master_socket, &remote_host);
  int nbytes = safe_read(socket, username, 64);
  if (nbytes <= 0) {
    return;
  }
  username[nbytes] = '\0';
  const int ack = 0;
  nbytes = safe_write(socket, &ack, sizeof(int));
  if (nbytes <= 0) {
    return;
  }

  user_t* new_user = new user_t;
  new_user->socket = socket;
  new_user->version = 0;
  new_user->name = username;
  this->users.push_back(new_user);
  char* ip;
  uint16_t port;
  to_ip_host(&remote_host, &ip, &port);
  printf("New user %s connected (%s:%d)\n", username, ip, port);
  if (socket > this->max_fd) {
    this->max_fd = socket;
  }
}

int main(int argc, char const* argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Utilisation: ./server <port>\n");
    exit(0);
  }
  const int port = atoi(argv[1]);
  if (port < 1024) {
    fprintf(stderr, "Le port doit être supérieur à 1023.\n");
    exit(0);
  }
  Server server = Server();
  server.run(port);
  return 0;
}
