#ifndef TCPSERVER_HPP
#define TCPSERVER_HPPP

#include <arpa/inet.h>
#include <map>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <string>
#include <unistd.h>
#include <vector>

struct tcpServer {
  struct sockaddr_in server_addr;
  int serverSocket;
  SSL_CTX *sslContext;
};

const std::string HOST = "127.0.0.1";
const std::map<std::string, std::string> MIME = {
    {".html", "text/html"},  {".htm", "text/html"},
    {".css", "text/css"},    {".js", "application/javascript"},
    {".png", "image/png"},   {".jpg", "image/jpeg"},
    {".jpeg", "image/jpeg"}, {".ico", "image/x-icon"}};
const std::map<int, std::string> STATUS = {
    {200, "200 OK"},
    {301, "301 Moved Permanently"},
    {400, "400 Bad Request"},
    {404, "404 Not Found"},
    {405, "405 Method Not Allowed"},
    {418, "418 I'm a teapot"},
    {500, "500 Internal Server Error"},
    {505, "505 HTTP Version Not Supported"}};
const std::map<int, std::vector<std::string>> statusHTML = {
    {301,
     {{"<!DOCTYPE html>"},
      {"<html>"},
      {"  <head>"},
      {"    <title>301 Moved Permanently</title>"},
      {"  </head>"},
      {"  <body>"},
      {"    <h1>Moved Permanently</h1>"},
      {"    <p>The requested URL ${URL} has been moved to ${NEW_URL}.</p>"},
      {"  </body>"},
      {"</html>\n"}}},
    {400,
     {{"<!DOCTYPE html>"},
      {"<html>"},
      {"  <head>"},
      {"    <title>400 Bad Request</title>"},
      {"  </head>"},
      {"  <body>"},
      {"    <h1>Bad Request</h1>"},
      {"    <p>The server cannot process the request due to a client "
       "error.</p>"},
      {"    </body>"},
      {"</html>\n"}}},
    {404,
     {{"<!DOCTYPE html>"},
      {"<html>"},
      {"  <head>"},
      {"    <title>404 Not Found</title>"},
      {"  </head>"},
      {"  <body>"},
      {"    <h1>Not Found</h1>"},
      {"    <p>The requested URL ${URL} was not found on this "
       "server.</p>"},
      {"  </body>"},
      {"</html>\n"}}},
    {405,
     {{"<!DOCTYPE html>"},
      {"<html>"},
      {"  <head>"},
      {"    <title>405 Method Not Allowed</title>"},
      {"  </head>"},
      {"  <body>"},
      {"    <h1>Method Not Allowed</h1>"},
      {"    <p>The requested method is not allowed for the URL.</p>"},
      {"  </body>"},
      {"</html>\n"}}},
    {418,
     {{"<!DOCTYPE html>"},
      {"<html>"},
      {"  <head>"},
      {"    <title>418 I'm a teapot</title>"},
      {"  </head>"},
      {"  <body>"},
      {"    <h1>I'm a teapot</h1>"},
      {"    <p>The server refuses to brew coffee because it is, permanently, a "
       "teapot.</p>"},
      {"  </body>"},
      {"</html>\n"}}},
    {500,
     {{"<!DOCTYPE html>"},
      {"<html>"},
      {"  <head>"},
      {"    <title>500 Internal Server Error</title>"},
      {"  </head>"},
      {"  <body>"},
      {"    <h1>Internal Server Error</h1>"},
      {"    <p>The server encountered an internal error or "
       "misconfiguration and was unable to complete your "
       "request.</p>"},
      {"  </body>"},
      {"</html>\n"}}},
    {505,
     {{"<!DOCTYPE html>"},
      {"<html>"},
      {"  <head>"},
      {"    <title>505 HTTP Version Not Supported</title>"},
      {"  </head>"},
      {"  <body>"},
      {"    <h1>HTTP Version Not Supported</h1>"},
      {"    <p>The requested HTTP version is not supported by the server.</p>"},
      {"  </body>"},
      {"</html>\n"}}}};

void createServer(tcpServer *server, const int port);
int serverAccept(tcpServer *server, sockaddr_in *client_addr,
                 socklen_t *client_addr_len);
void handleRequest(int clientSocket, SSL_CTX *sslContext);
void sendFile(int clientSocket, SSL *ssl, std::string &path, bool isSecure);

#endif