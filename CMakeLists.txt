cmake_minimum_required(VERSION 3.10)
project(Laboratorio2)

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})

set(SERVER_SRC
    src/server.cpp
    src/libs/tcpServer.cpp
    src/libs/utils.cpp
)

add_executable(server ${SERVER_SRC})
target_link_libraries(server ${OPENSSL_LIBRARIES})
include_directories(include)

