#include "network.h"

class Server {
public:
  Server() {
    socket_acceptor_ = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_acceptor_ == -1) {
      std::cout << "Failed to create socket. errno: " << errno << std::endl;
      exit(EXIT_FAILURE);
    }
    sockaddr_in sockaddr;
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_addr.s_addr = INADDR_ANY;
    sockaddr.sin_port = htons(port_); // htons is necessary to convert a number to
    // network byte order
    if (bind(socket_acceptor_, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) < 0) {
      std::cout << "Failed to bind to port. errno: " << errno << std::endl;
      exit(EXIT_FAILURE);
    }

    // Start listening. Hold at most 10 connections in the queue
    if (listen(socket_acceptor_, max_queue_listen_) < 0) {
      std::cout << "Failed to listen on socket. errno: " << errno << std::endl;
      exit(EXIT_FAILURE);
    }
  }

  void AcceptThread() {
    while (true) {
      // Grab a connection from the queue
      auto addrlen = sizeof(sockaddr);
      sockaddr_in sockaddr;
      int connection = accept(socket_acceptor_, (struct sockaddr*)&sockaddr, (socklen_t*)&addrlen);
      if (!is_running_) {
        return;
      }
      if (connection < 0) {
        std::cout << "Failed to grab connection. errno: " << errno << std::endl;
        exit(EXIT_FAILURE);
      }
      clients_sockets_.push_back({connection, sockaddr});
    }
  }

  void Run() {
    is_running_ = true;
    accept_thread = std::async(&Server::AcceptThread, this);
    Interface();
    is_running_ = false;
    close(socket_acceptor_);
  }

  void Interface(){
    while (true) {
      std::cout << "Choose command (enter number):\n \
    [1] - view all clients\n \
    [2] - send broadcast message\n \
    [3] - send private messages\n \
    [4] - refresh all\n \
    [0] - close server\n";
      int query = 0;
      std::cin >> query;
      if (query < 0 || query > 4) {
        std::cout << "Error: Invalid command.\n";
      } else {
        char msg[MSG_SEND_MAX];
        std::optional<std::string> rec_msg;
        char str_addr[20];
        int id = 0;
        switch(query) {
          case 1:
            PrintAllClients();
            break;
          case 2:
            std::cout << "Enter message for all: ";
            fgets(msg, MSG_SEND_MAX, stdin);
            if (msg[0]=='\n') {
              fgets(msg, MSG_SEND_MAX, stdin);
            }
            for (auto client : clients_sockets_) {
              Send(client.socket, "M: " + std::string(msg));
            }
            break;
          case 3:
            PrintAllClients();
            std::cout << "Choose client id: ";
            std::cin >> id;
            if (id < 0 || id >= clients_sockets_.size()) {
              std::cout << "Error: Invalid client id.\n";
            } else {
              std::cout << "Enter message: ";
              std::cin >> msg;
              Send(clients_sockets_[id].socket, "M: " + std::string(msg));
              std::cout << "Message send\n";
            }
            break;
          case 4:
            for (auto & clients_socket : clients_sockets_) {
              rec_msg = Receive(clients_socket.socket);
              if (rec_msg) {
                inet_ntop(AF_INET, &(clients_socket.sockaddr), str_addr, INET_ADDRSTRLEN);
                std::cout << "Client " << str_addr << "replayed: "<< *rec_msg << "\n";
              }
            }
            break;
          case 0:
            return;
          default:
            break;
        }
      }
    }
  }

  void PrintAllClients() {
    std::cout << "Clients total: " << clients_sockets_.size() << '\n';
    for (int i = 0; i < clients_sockets_.size(); ++i) {
      char str[20];
      inet_ntop(AF_INET, &(clients_sockets_[i].sockaddr), str, INET_ADDRSTRLEN);
      std::cout << i << ' ' << str << ':' << ntohs(clients_sockets_[i].sockaddr.sin_port) << '\n';
    }
  }


private:
  const int max_queue_listen_ = 10;
  const int port_ = 3376;

  int socket_acceptor_;

  struct Client {
    int socket;
    sockaddr_in sockaddr;
  };

  std::vector<Client> clients_sockets_;
//  std::vector<std::string> clients_messages_;
  std::future<void> accept_thread;
  std::atomic_bool is_running_;
};



int main() {
  Server server;
  server.Run();
  return 0;
}
