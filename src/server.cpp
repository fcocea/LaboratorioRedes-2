#include <iostream>

#include "tcpServer.hpp"
#include "utils.hpp"

using namespace std;

int main(int argc, char *argv[]) {
  vector<string> args(argv, argv + argc);

  const string PORT = readParam(argc, args, 'p');
  const string CERTIFICATE = readParam(argc, args, 'c');
  const string PRIVATE_KEY = readParam(argc, args, 'k');
  if (PORT.empty()) {
    cout << "Missing required parameter -p PORT" << endl;
    return 1;
  }
  if (!isNumber(PORT)) {
    cerr << "Invalid port number: " << PORT << endl;
    return 1;
  }
  if (!CERTIFICATE.empty() && PRIVATE_KEY.empty()) {
    cout << "Missing required parameter -k PRIVATE_KEY" << endl;
    return 1;
  }
  if (!CERTIFICATE.empty() && !verifyFile(CERTIFICATE)) {
    cout << "Invalid certificate file: " << CERTIFICATE << endl;
    return 1;
  }
  if (CERTIFICATE.empty() && !PRIVATE_KEY.empty()) {
    cout << "Missing required parameter -c CERTIFICATE" << endl;
    return 1;
  }
  if (!PRIVATE_KEY.empty() && !verifyFile(PRIVATE_KEY)) {
    cout << "Invalid private key file: " << PRIVATE_KEY << endl;
    return 1;
  }
  if (createFolder("www")) {
    cout << "» The \"www\" folder was created successfully!" << endl;
  }
  struct tcpServer server;
  createServer(&server, stoi(PORT), CERTIFICATE, PRIVATE_KEY);
  cout << "» Server listening on port: " << PORT << endl;
  if (CERTIFICATE.empty() && PRIVATE_KEY.empty()) {
    cout << "» Server runs only in HTTP mode" << endl;
  }
  while (true) {
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int connectionSocket =
        serverAccept(&server, &client_addr, &client_addr_len);
    unsigned char firstByte;
    bool SSLConnection = false;
    if (recv(connectionSocket, &firstByte, sizeof(firstByte), MSG_PEEK)) {
      SSLConnection = firstByte == 0x16;
      if (CERTIFICATE.empty() && PRIVATE_KEY.empty() && SSLConnection) {
        close(connectionSocket);
        continue;
      }
    }
    handleRequest(connectionSocket, SSLConnection ? server.sslContext : NULL);
    close(connectionSocket);
  }
  close(server.serverSocket);
  return 0;
}