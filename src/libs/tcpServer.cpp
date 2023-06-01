#include <iostream>

#include "tcpServer.hpp"
#include "utils.hpp"
#include <openssl/err.h>
#include <openssl/ssl.h>

#define BUFFER_SIZE 1024

using namespace std;

void createServer(tcpServer *server, const int port, const string &cert,
                  const string &key) {
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
  if (cert.empty() || key.empty())
    return;
  OpenSSL_add_all_algorithms();
  SSL_load_error_strings();
  server->sslContext = SSL_CTX_new(TLS_server_method());
  if (!server->sslContext) {
    cout << "Error creating SSL context" << endl;
    exit(1);
  }
  if (SSL_CTX_use_certificate_file(server->sslContext, cert.c_str(),
                                   SSL_FILETYPE_PEM) <= 0) {
    cout << "Error loading certificate file" << endl;
    exit(1);
  }

  if (SSL_CTX_use_PrivateKey_file(server->sslContext, key.c_str(),
                                  SSL_FILETYPE_PEM) <= 0) {
    cout << "Error loading private key file" << endl;
    exit(1);
  }
  if (!SSL_CTX_check_private_key(server->sslContext)) {
    cout << "Error checking private key" << endl;
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

void handleRequest(int clientSocket, SSL_CTX *sslContext) {
  bool isSecure = sslContext != NULL;
  SSL *ssl;
  if (sslContext) {
    ssl = SSL_new(sslContext);
    SSL_set_fd(ssl, clientSocket);
    if (SSL_accept(ssl) <= 0) {
      cout << "Error accepting SSL connection" << endl;
      return;
    }
  }
  char buffer[BUFFER_SIZE];
  int clientRequest = isSecure ? SSL_read(ssl, buffer, BUFFER_SIZE - 1)
                               : read(clientSocket, buffer, BUFFER_SIZE - 1);
  if (clientRequest < 0) {
    cout << "A problem occurred while processing the client's request." << endl;
    if (!isSecure)
      return;
    SSL_shutdown(ssl);
    SSL_free(ssl);
    return;
  }
  string request = string(buffer);
  vector<string> requestLines;
  stringstream ss(request);
  string line;
  while (getline(ss, line, '\n')) {
    line = regex_replace(line, regex("\r"), "");
    line = regex_replace(line, regex("\n"), "");
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
    isSecure ? SSL_write(ssl, response.c_str(), response.length())
             : write(clientSocket, response.c_str(), response.length());
    if (!isSecure)
      return;
    SSL_shutdown(ssl);
    SSL_free(ssl);
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
    isSecure ? SSL_write(ssl, response.c_str(), response.length())
             : write(clientSocket, response.c_str(), response.length());
    cout << requestHeaders["Method"] << " " << requestHeaders["Path"]
         << " - 505" << (isSecure ? " - HTTPS" : "") << endl;
    if (!isSecure)
      return;
    SSL_shutdown(ssl);
    SSL_free(ssl);
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
    isSecure ? SSL_write(ssl, response.c_str(), response.length())
             : write(clientSocket, response.c_str(), response.length());
    cout << requestHeaders["Method"] << " " << requestHeaders["Path"]
         << " - 418" << (isSecure ? " - HTTPS" : "") << endl;
    if (!isSecure)
      return;
    SSL_shutdown(ssl);
    SSL_free(ssl);
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
    isSecure ? SSL_write(ssl, response.c_str(), response.length())
             : write(clientSocket, response.c_str(), response.length());
    cout << requestHeaders["Method"] << " " << requestHeaders["Path"]
         << " - 405" << (isSecure ? " - HTTPS" : "") << endl;
    if (!isSecure)
      return;
    SSL_shutdown(ssl);
    SSL_free(ssl);
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
    isSecure ? SSL_write(ssl, response.c_str(), response.length())
             : write(clientSocket, response.c_str(), response.length());
    cout << requestHeaders["Method"] << " " << requestHeaders["Path"]
         << " - 400" << (isSecure ? " - HTTPS" : "") << endl;
    if (!isSecure)
      return;
    SSL_shutdown(ssl);
    SSL_free(ssl);
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
      isSecure ? SSL_write(ssl, response.c_str(), response.length())
               : write(clientSocket, response.c_str(), response.length());
      cout << requestHeaders["Method"] << " " << requestHeaders["Path"]
           << " - 301" << (isSecure ? " - HTTPS" : "") << endl;
      if (!isSecure)
        return;
      SSL_shutdown(ssl);
      SSL_free(ssl);
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
    isSecure ? SSL_write(ssl, response.c_str(), response.length())
             : write(clientSocket, response.c_str(), response.length());

    cout << requestHeaders["Method"] << " " << requestHeaders["Path"]
         << " - 404" << (isSecure ? " - HTTPS" : "") << endl;
    if (!isSecure)
      return;
    SSL_shutdown(ssl);
    SSL_free(ssl);
    return;
  }
  sendFile(clientSocket, ssl, path, isSecure);
  cout << requestHeaders["Method"] << " " << requestHeaders["Path"] << " - 200"
       << (isSecure ? " - HTTPS" : "") << endl;
  if (!isSecure)
    return;
  SSL_shutdown(ssl);
  SSL_free(ssl);
}

void sendFile(int clientSocket, SSL *ssl, string &path, bool isSecure) {
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
    isSecure ? SSL_write(ssl, response.c_str(), response.length())
             : write(clientSocket, response.c_str(), response.length());
    cout << "Failed to send file: " << path << " - 500"
         << (isSecure ? " - HTTPS" : "") << endl;
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
  isSecure ? SSL_write(ssl, response.c_str(), response.length())
           : write(clientSocket, response.c_str(), response.length());
  char buffer[BUFFER_SIZE];
  int bytesRead;
  while ((bytesRead = fread(buffer, 1, BUFFER_SIZE, file)) > 0)
    isSecure ? SSL_write(ssl, buffer, bytesRead)
             : write(clientSocket, buffer, bytesRead);
  fclose(file);
}