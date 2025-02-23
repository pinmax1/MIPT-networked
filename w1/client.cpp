#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <stdio.h>
#include "socket_tools.h"

int main(int argc, const char **argv)
{
  const char *port = "2025";

  addrinfo resAddrInfo;
  int sfd = create_dgram_socket("localhost", port, &resAddrInfo);

  if (sfd == -1)
  {
    printf("Cannot create a socket\n");
    return 1;
  }
  printf("Enter your username:");
  std::string input;
  std::getline(std::cin, input);
  ssize_t res = sendto(sfd, input.c_str(), input.size(), 0, resAddrInfo.ai_addr, resAddrInfo.ai_addrlen);
  printf(">");
  fflush(stdout);
  while (true)
  {
    fd_set readSet;
    FD_ZERO(&readSet);
    FD_SET(sfd, &readSet);
    FD_SET(fileno(stdin), &readSet);
    timeval timeout = { 0, 100000 }; // 100 ms
    select(sfd + 1, &readSet, NULL, NULL, &timeout);


    if(FD_ISSET(fileno(stdin), &readSet)) {
      printf(">");
      std::string input;
      std::getline(std::cin, input);
      ssize_t res = sendto(sfd, input.c_str(), input.size(), 0, resAddrInfo.ai_addr, resAddrInfo.ai_addrlen);
      if (res == -1)
        std::cout << strerror(errno) << std::endl;
    }


    if(FD_ISSET(sfd, &readSet)) {
      constexpr size_t buf_size = 1000;
      static char buffer[buf_size];
      memset(buffer, 0, buf_size);

      sockaddr_in sin;
      socklen_t slen = sizeof(sockaddr_in);
      ssize_t numBytes = recvfrom(sfd, buffer, buf_size - 1, 0, (sockaddr*)&sin, &slen);
      if (numBytes > 0) {
        printf("%s\n", buffer);
        printf(">");
        fflush(stdout);
      }
        
    }
  }
  return 0;
}