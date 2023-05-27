cmake_minimum_required(VERSION 3.10)
project(Laboratorio2)

set(SERVER_SRC
    src/server.cpp
    src/libs/tcpServer.cpp
    src/libs/utils.cpp
)

add_executable(server ${SERVER_SRC})

include_directories(include)

