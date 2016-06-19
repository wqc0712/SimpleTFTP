cmake_minimum_required(VERSION 3.3)
project(SimpleTFTP)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SERVER_SOURCE TFTPServer.cpp ServerSocket.cpp Socket.cpp log.cpp)
set(CLIENT_SOURCE TFTPClient.cpp ClientSocket.cpp Socket.cpp log.cpp)
add_executable(server ${SERVER_SOURCE})
add_executable(client ${CLIENT_SOURCE})