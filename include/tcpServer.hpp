#ifndef TCPSERVER_HPP
#define TCPSERVER_HPPP

#include <arpa/inet.h>
#include <unistd.h>

struct tcpServer {
  struct sockaddr_in server_addr;
  int serverSocket;
};

void createServer(tcpServer *server, const int port);
int serverAccept(tcpServer *server, sockaddr_in *client_addr,
                 socklen_t *client_addr_len);
void handleRequest(int clientSocket);
void sendFile(int clientSocket, std::string &path);

#endif