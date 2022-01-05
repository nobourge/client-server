#include <pthread.h>

#include <queue>
#include <string>
#include <vector>

#include "socketlib.h"

using namespace std;

typedef struct {
  string name;
  int socket;
  unsigned version;

} user_t;

class Server {
 private:
  int max_fd;
  int master_socket;
  vector<user_t*> users;

 private:
  void prepateFDSet(fd_set* read_set);
  void handleSocketReadActivity(fd_set* in_set, int& nactivities);
  void handleNewConnection();
  void disconnectUser(unsigned user_num);
  void forward(message_t* msg);

 public:
  Server();
  void run(int port = 8080);
  void shutDown();
};
