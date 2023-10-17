//
// Created by trimesh on 17.10.2023.
//

#include <iostream>
#include <string>
#include <netinet/in.h>
#include <cstdlib>
#include <vector>
#include <array>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <optional>

#include <future>

#ifndef MATRIX_CHATTING__NETWORK_H_
#define MATRIX_CHATTING__NETWORK_H_


#define MSG_SEND_MAX 1024
#define MSG_RECEIVED "received"

bool SocketHasData(int socket, int time_usec=10) {
  fd_set set;
  timeval time;

  FD_ZERO(&set);
  FD_SET(socket, &set);
  time.tv_usec = time_usec;
  select(socket + 1, &set, nullptr, nullptr, &time);
  return FD_ISSET(socket, &set);
}

void Send(int client_socket, const std::string& msg) {
  int msg_size = msg.size() * sizeof(char);
  send(client_socket, &msg_size, sizeof(msg_size), 0);
  send(client_socket, msg.c_str(), msg_size, 0);
}

std::optional<std::string> Receive(int client_socket) {
  if (SocketHasData(client_socket)) {
    int msg_size = 0;
    recv(client_socket, &msg_size, sizeof(msg_size), 0);
    char msg[msg_size];
    recv(client_socket, &msg, msg_size, 0);
    return std::string(msg);
  }
  return {};
}


#endif //MATRIX_CHATTING__NETWORK_H_
