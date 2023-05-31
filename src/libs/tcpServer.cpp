#include <iostream>

#include "tcpServer.hpp"
#include "utils.hpp"

#define BUFFER_SIZE 1024

using namespace std;

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
    string body = joinVector(statusHTML.at(400), "\n");
    vector<string> responseHeaders = {
        "HTTP/1.1 " + STATUS.at(400),
        "Content-Type: text/html",
        "Content-Length: " + to_string(body.length()),
    };
    string response = joinVector(responseHeaders, "\r\n") + "\r\n\r\n" + body;
    write(clientSocket, response.c_str(), response.length());
    return;
  }
  if (requestHeaders["Version"] != "HTTP/1.1") {
    string body = joinVector(statusHTML.at(505), "\n");
    vector<string> responseHeaders = {
        "HTTP/1.1 " + STATUS.at(505),
        "Content-Type: text/html",
        "Content-Length: " + to_string(body.length()),
    };
    string response = joinVector(responseHeaders, "\r\n") + "\r\n\r\n" + body;
    write(clientSocket, response.c_str(), response.length());
    cout << requestHeaders["Method"] << " " << requestHeaders["Path"]
         << " - 505" << endl;
    return;
  }
  if (requestHeaders["Method"] == "BREW") {
    string body = joinVector(statusHTML.at(418), "\n");
    vector<string> responseHeaders = {
        "HTTP/1.1 " + STATUS.at(418),
        "Content-Type: text/html",
        "Content-Length: " + to_string(body.length()),
    };
    string response = joinVector(responseHeaders, "\r\n") + "\r\n\r\n" + body;
    write(clientSocket, response.c_str(), response.length());
    cout << requestHeaders["Method"] << " " << requestHeaders["Path"]
         << " - 418" << endl;
    return;
  }
  if (requestHeaders["Method"] != "GET") {
    string body = joinVector(statusHTML.at(405), "\n");
    vector<string> responseHeaders = {
        "HTTP/1.1 " + STATUS.at(405),
        "Content-Type: text/html",
        "Content-Length: " + to_string(body.length()),
    };
    string response = joinVector(responseHeaders, "\r\n") + "\r\n\r\n" + body;
    write(clientSocket, response.c_str(), response.length());
    cout << requestHeaders["Method"] << " " << requestHeaders["Path"]
         << " - 405" << endl;
    return;
  }
  string path = requestHeaders["Path"];
  path = regex_replace(path, regex("/+"), "/");
  requestHeaders["Path"] = path;
  if (path[0] != '/') {
    string body = joinVector(statusHTML.at(400), "\n");
    vector<string> responseHeaders = {
        "HTTP/1.1 " + STATUS.at(400),
        "Content-Type: text/html",
        "Content-Length: " + to_string(body.length()),
    };
    string response = joinVector(responseHeaders, "\r\n") + "\r\n\r\n" + body;
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
    if (path[path.length() - 1] != '/') { // /folder -> /folder/
      string body = joinVector(statusHTML.at(301), "\n");
      body = regex_replace(body, regex("\\$\\{URL\\}"), requestHeaders["Path"]);
      body = regex_replace(body, regex("\\$\\{NEW_URL\\}"),
                           requestHeaders["Path"] + "/");
      vector<string> responseHeaders = {
          "HTTP/1.1 " + STATUS.at(301), "Content-Type: text/html",
          "Content-Length: " + to_string(body.length()),
          "Location: " + requestHeaders["Path"] + "/"};
      string response = joinVector(responseHeaders, "\r\n") + "\r\n\r\n" + body;
      write(clientSocket, response.c_str(), response.length());
      cout << requestHeaders["Method"] << " " << requestHeaders["Path"]
           << " - 301" << endl;
      return;
    }
    path += "/index.html";
  }
  path = "www" + path;
  if (!verifyFile(path)) {
    string body = joinVector(statusHTML.at(404), "\n");
    body = regex_replace(body, regex("\\$\\{URL\\}"), requestHeaders["Path"]);
    vector<string> responseHeaders = {
        "HTTP/1.1 " + STATUS.at(404),
        "Content-Type: text/html",
        "Content-Length: " + to_string(body.length()),
    };
    string response = joinVector(responseHeaders, "\r\n") + "\r\n\r\n" + body;
    write(clientSocket, response.c_str(), response.length());

    cout << requestHeaders["Method"] << " " << requestHeaders["Path"]
         << " - 404" << endl;
    return;
  }
  sendFile(clientSocket, path);
  cout << requestHeaders["Method"] << " " << requestHeaders["Path"] << " - 200"
       << endl;
}

void sendFile(int clientSocket, string &path) {
  path = regex_replace(path, regex("/+"), "/");
  string extension = path.substr(path.find_last_of('.'));
  string contentType =
      MIME.count(extension) ? MIME.at(extension) : "application/octet-stream";

  FILE *file = fopen(path.c_str(), "rb");
  if (!file) {
    string body = joinVector(statusHTML.at(500), "\n");
    vector<string> responseHeaders = {
        "HTTP/1.1 " + STATUS.at(500),
        "Content-Type: text/html",
        "Content-Length: " + to_string(body.length()),
    };
    string response = joinVector(responseHeaders, "\r\n") + "\r\n\r\n" + body;
    write(clientSocket, response.c_str(), response.length());
    cout << "Failed to send file: " << path << " - 500" << endl;
    return;
  }
  fseek(file, 0, SEEK_END);
  int fileSize = ftell(file);
  rewind(file);
  vector<string> responseHeaders = {
      "HTTP/1.1 " + STATUS.at(200),
      "Content-Type: " + contentType,
      "Content-Length: " + to_string(fileSize),
  };
  string response = joinVector(responseHeaders, "\r\n") + "\r\n\r\n";
  write(clientSocket, response.c_str(), response.length());
  char buffer[BUFFER_SIZE];
  int bytesRead;
  while ((bytesRead = fread(buffer, 1, BUFFER_SIZE, file)) > 0)
    write(clientSocket, buffer, bytesRead);
  fclose(file);
}