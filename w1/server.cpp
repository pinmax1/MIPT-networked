#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <map>
#include<string>
#include "socket_tools.h"

void send_message_to_all(int sfd, std::string message, std::map<std::string, sockaddr_in> userdata_to_socket, std::string sender_address = "") {
  for(const auto& [address, socket]: userdata_to_socket) {
    if(address != sender_address || sender_address.empty()) {
      ssize_t res = sendto(sfd, message.c_str(), message.size(), 0, (sockaddr*)&socket, sizeof(socket));
      if (res == -1)
        std::cout << strerror(errno) << std::endl;
    }
  }
}



int main(int argc, const char **argv)
{
  srand(time(NULL));
  const char *port = "2025";

  int sfd = create_dgram_socket(nullptr, port, nullptr);

  if (sfd == -1)
  {
    printf("cannot create socket\n");
    return 1;
  }
  printf("listening!\n");

  std::map<std::string, std::string> userdata_to_username;
  std::map<std::string, sockaddr_in> userdata_to_socket;

  bool mathduel_active = false;;
  std::string first_mathduel_address;
  std::string second_mathduel_address;
  int correct_answer;

  while (true)
  {
    fd_set readSet;
    FD_ZERO(&readSet);
    FD_SET(sfd, &readSet);

    timeval timeout = { 0, 100000 }; // 100 ms
    select(sfd + 1, &readSet, NULL, NULL, &timeout);


    if (FD_ISSET(sfd, &readSet))
    {
      constexpr size_t buf_size = 1000;
      static char buffer[buf_size];
      memset(buffer, 0, buf_size);

      sockaddr_in sin;
      socklen_t slen = sizeof(sockaddr_in);
      ssize_t numBytes = recvfrom(sfd, buffer, buf_size - 1, 0, (sockaddr*)&sin, &slen);

      if (numBytes > 0)
      {
        std::string client_data = std::string(inet_ntoa(sin.sin_addr)) + ':' + std::to_string(sin.sin_port);
        std::string message(buffer);
        if(userdata_to_username.find(client_data) == userdata_to_username.end()) {
          userdata_to_username[client_data] = buffer;
          userdata_to_socket[client_data] = sin;
          printf("New user %s entered!\n", buffer);
          continue;
        }

        printf("(%s:%d %s) %s\n", inet_ntoa(sin.sin_addr), sin.sin_port, userdata_to_username[client_data].c_str(), buffer); // assume that buffer is a string

        if(message.substr(0,3) == "/c ") {
          std::string message_to_send = message.substr(3);
          send_message_to_all(sfd, message_to_send, userdata_to_socket, client_data);
        }

        if(message == "/mathduel") {
          if(!mathduel_active) {
            if(first_mathduel_address.empty()) {
              first_mathduel_address = client_data;
              send_message_to_all(sfd,"Client " + userdata_to_username[client_data] + " wants to start mathduel!", userdata_to_socket, client_data);
            } else if(second_mathduel_address.empty()) {
              second_mathduel_address = client_data;
              mathduel_active = true;
              std::string task;
              int a = rand()%30 + 1;
              int b = rand()%20 + 1;
              int c = rand()%100 + 1;
              bool is_plus = rand() & 1;
              if(is_plus) {
                task = std::to_string(a) + '*' + std::to_string(b) + '+' + std::to_string(c);
                correct_answer = a*b+c;
              } else {
                task = std::to_string(a) + '*' + std::to_string(b) + '-' + std::to_string(c);
                correct_answer = a*b-c;
              }

              sockaddr_in first_socket = userdata_to_socket[first_mathduel_address];
              sockaddr_in second_socket = userdata_to_socket[second_mathduel_address];

              sendto(sfd, task.c_str(), message.size(), 0, (sockaddr*)&first_socket, sizeof(first_socket));
              sendto(sfd, task.c_str(), message.size(), 0, (sockaddr*)&second_socket, sizeof(second_socket));
            }
          } else {
            std::string busy = "Mathduel has already started";
            sendto(sfd, busy.c_str(), busy.size(), 0, (sockaddr*)&sin, sizeof(sin));
          }
        }
        
        if(message.substr(0,5) == "/ans " && (client_data == first_mathduel_address || client_data == second_mathduel_address)) {
          if(message.substr(5) == std::to_string(correct_answer)) {
            std::string win_message = "User " + userdata_to_username[client_data] + " has won mathduel!";
            send_message_to_all(sfd, win_message, userdata_to_socket);
            mathduel_active = false;
            first_mathduel_address = "";
            second_mathduel_address = "";
          } else {
            std::string incorect_answer = "Incorrect answer";
            sendto(sfd, incorect_answer.c_str(), incorect_answer.size(), 0, (sockaddr*)&sin, sizeof(sin));
          }
        }
        
      }
    }
  }
  return 0;
}