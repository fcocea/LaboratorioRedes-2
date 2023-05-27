#include <iostream>

#include "tcpServer.hpp"
#include "utils.hpp"

using namespace std;

int main(int argc, char *argv[]) {
  vector<string> args(argv, argv + argc);

  const string PORT = readParam(argc, args, 'p');
  if (PORT.empty()) {
    cout << "Missing required parameter -p PORT" << endl;
    return 1;
  }
  if (!isNumber(PORT)) {
    cerr << "Invalid port number: " << PORT << endl;
    return 1;
  }
  struct tcpServer server;
  createServer(&server, stoi(PORT));
  cout << "» Server listening on port: " << PORT << endl;
  while (true) {
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int connectionSocket =
        serverAccept(&server, &client_addr, &client_addr_len);
    handleRequest(connectionSocket);
    close(connectionSocket);
  }
  close(server.serverSocket);
  return 0;
}