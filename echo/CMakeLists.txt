cmake_minimum_required(VERSION 3.7)
project(echo)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cc echo.cc)

add_executable(echo ${SOURCE_FILES})

TARGET_LINK_LIBRARIES(echo muduo_base muduo_net pthread)
