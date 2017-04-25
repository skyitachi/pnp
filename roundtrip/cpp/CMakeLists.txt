cmake_minimum_required(VERSION 3.7)
project(roundtrip)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES roundtrip.cc)
add_executable(roundtrip ${SOURCE_FILES})