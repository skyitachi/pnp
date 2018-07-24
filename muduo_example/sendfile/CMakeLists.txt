cmake_minimum_required(VERSION 3.7)
project(sendfile)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES server.cc)

add_executable(filetransfer ${SOURCE_FILES})

TARGET_LINK_LIBRARIES(filetransfer muduo_base muduo_net pthread)
