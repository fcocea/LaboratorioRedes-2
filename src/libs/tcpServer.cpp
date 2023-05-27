#include <iostream>

#include "tcpServer.hpp"
#include "utils.hpp"

#define BUFFER_SIZE 1024

using namespace std;

const string HOST = "127.0.0.1";
const map<string, string> MIME = {
    {".html", "text/html"},  {".htm", "text/html"},
    {".css", "text/css"},    {".js", "application/javascript"},
    {".png", "image/png"},   {".jpg", "image/jpeg"},
    {".jpeg", "image/jpeg"}, {".ico", "image/x-icon"}};

void createServer(tcpServer *server, const int port) {
  int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (serverSocket < 0) {
    cout << "Error creating socket" << endl;
    exit(1);
  }
  server->serverSocket = serverSocket;
  server->server_addr.sin_family = AF_INET;
  server->server_addr.sin_addr.s_addr = inet_addr(HOST.c_str());
  server->server_addr.sin_port = htons(port);

  if (bind(server->serverSocket, (struct sockaddr *)&(server->server_addr),
           sizeof(server->server_addr)) < 0) {
    cout << "Error binding socket" << endl;
    exit(1);
  }
  if (listen(server->serverSocket, 5) < 0) {
    cout << "Error listening socket" << endl;
    exit(1);
  }
}

int serverAccept(tcpServer *server, sockaddr_in *client_addr,
                 socklen_t *client_addr_len) {
  int connectionSocket = accept(
      server->serverSocket, (struct sockaddr *)client_addr, client_addr_len);
  if (connectionSocket < 0) {
    cout << "Error accepting connection" << endl;
    exit(1);
  }
  return connectionSocket;
}

void handleRequest(int clientSocket) {
  char buffer[BUFFER_SIZE];
  int clientRequest = read(clientSocket, buffer, BUFFER_SIZE - 1);
  if (clientRequest < 0) {
    cout << "A problem occurred while processing the client's request." << endl;
    return;
  }
  string request = string(buffer);
  vector<string> requestLines;
  stringstream ss(request);
  string line;
  while (getline(ss, line, '\n')) {
    line.erase(remove(line.begin(), line.end(), '\r'), line.end());
    line.erase(remove(line.begin(), line.end(), '\n'), line.end());
    requestLines.push_back(line);
  }
  map<string, string> requestHeaders = parseRequest(requestLines);
  if (requestHeaders.empty()) {
    string response = "HTTP/1.1 400 Bad Request\r\n\r\n";
    write(clientSocket, response.c_str(), response.length());
    return;
  }
  if (requestHeaders["Version"] != "HTTP/1.1") {
    string response = "HTTP/1.1 505 HTTP Version Not Supported\r\n\r\n";
    write(clientSocket, response.c_str(), response.length());
    cout << requestHeaders["Method"] << " " << requestHeaders["Path"]
         << " - 505" << endl;
    return;
  }
  if (requestHeaders["Method"] != "GET") {
    string response = "HTTP/1.1 405 Method Not Allowed\r\n\r\n";
    write(clientSocket, response.c_str(), response.length());
    cout << requestHeaders["Method"] << " " << requestHeaders["Path"]
         << " - 405" << endl;
    return;
  }
  string path = requestHeaders["Path"];
  path = regex_replace(path, regex("/+"), "/");
  if (path.length() > 1 && path[path.length() - 1] == '/') {
    path = path.substr(0, path.length() - 1);
  }
  requestHeaders["Path"] = path;
  if (path[0] != '/') {
    string response = "HTTP/1.1 400 Bad Request\r\n\r\n";
    write(clientSocket, response.c_str(), response.length());
    cout << requestHeaders["Method"] << " " << requestHeaders["Path"]
         << " - 400" << endl;
    return;
  }
  // regex folderPattern("(/.*/)|/"); /folder/ -> /folder/index.html ;
  // /file -> /file
  // if (regex_match(path, folderPattern)) {
  //   path += "index.html";
  // }
  regex extensionPattern(
      "\\.[a-zA-Z0-9]+$"); // /folder | /folder/ -> /folder/index.html
  if (!regex_search(path, extensionPattern)) {
    path += "/index.html";
  }
  path = "www" + path;
  if (!verifyFile(path)) {
    string body = "<!DOCTYPE html><html><head><title>404 Not "
                  "Found</title></head><body><h1>Not "
                  "Found</h1><p>The requested URL " +
                  requestHeaders["Path"] +
                  " was not found on this "
                  "server.</p></body></html>";
    string response = "HTTP/1.1 404 Not Found\r\n";
    response += "Content-Type: text/html\r\n";
    response += "Content-Length: " + to_string(body.length()) + "\r\n\r\n";

    write(clientSocket, response.c_str(), response.length());
    write(clientSocket, body.c_str(), body.length());

    cout << requestHeaders["Method"] << " " << requestHeaders["Path"]
         << " - 404" << endl;
    return;
  }
  sendFile(clientSocket, path);
  cout << requestHeaders["Method"] << " " << requestHeaders["Path"] << " - 200"
       << endl;
}

void sendFile(int clientSocket, const string &path) {
  string extension = path.substr(path.find_last_of('.'));
  string contentType =
      MIME.count(extension) ? MIME.at(extension) : "application/octet-stream";

  FILE *file = fopen(path.c_str(), "rb");
  if (!file) {
    string response = "HTTP/1.1 500 Internal Server Error\r\n\r\n";
    write(clientSocket, response.c_str(), response.length());
    cout << "500" << endl;
    return;
  }
  fseek(file, 0, SEEK_END);
  int fileSize = ftell(file);
  rewind(file);
  string response = "HTTP/1.1 200 OK\r\n";
  response += "Content-Type: " + contentType + "\r\n";
  response += "Content-Length: " + to_string(fileSize) + "\r\n\r\n";
  write(clientSocket, response.c_str(), response.length());
  char buffer[BUFFER_SIZE];
  int bytesRead;
  while ((bytesRead = fread(buffer, 1, BUFFER_SIZE, file)) > 0)
    write(clientSocket, buffer, bytesRead);
  fclose(file);
}