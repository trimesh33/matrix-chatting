#include "network.h"

sockaddr_in MakeAddrIP4(const std::string& address, uint16_t port) {
  sockaddr_in res = {0};
  res.sin_family = AF_INET;
  res.sin_port = htons(port);
  memset(&res.sin_zero, 0, sizeof(res.sin_zero));
  inet_pton(AF_INET, address.c_str(), &res.sin_addr);
  return res;
}

class Client {
 public:
  Client(uint16_t port, const std::string& address) {
    socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_ == -1) {
      std::cout << "Failed to create socket. errno: " << errno << std::endl;
      exit(EXIT_FAILURE);
    }
    sockaddr_in addr = MakeAddrIP4(address, port);
    int success = connect(socket_, (sockaddr *)&addr, sizeof(addr));
    if (success == -1) {
      std::cout << "Fail to connect to server. errno: " << errno << std::endl;
      exit(EXIT_FAILURE);
    }
  }

  void Run() {
    Interface();
  }

  void Interface(){
    while (true) {
      std::cout << "Choose command (enter number):\n \
    [1] - view all messages\n \
    [2] - reply\n \
    [3] - refresh\n \
    [0] - close client\n";
      int query = 0;
      std::cin >> query;
      if (query < 0 || query > 3) {
        std::cout << "Error: Invalid command.\n";
      } else {
        char msg[MSG_SEND_MAX];
        std::optional<std::string> rec_msg;
        switch(query) {
          case 1:
            break;
          case 2:
            std::cout << "Write your message to Morpheus:\n";
            fgets(msg, MSG_SEND_MAX, stdin);
            Send(socket_, msg);
            break;
          case 3:
            rec_msg = Receive(socket_);
            Send(socket_, MSG_RECEIVED);
            std::cout << *rec_msg << "\n";
            break;
          case 0:
            // send message to server to remove me
            return;
        }
      }
    }
  }

 private:
  int socket_;
};

int main() {
  uint16_t port = 3376;
  std::string address = "127.0.0.1";
  Client client(port, address);
  client.Run();
}