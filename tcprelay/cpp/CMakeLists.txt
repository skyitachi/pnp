cmake_minimum_required(VERSION 3.6)
project(cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -lpthread")

file(GLOB SOURCE_FILES ${PROJECT_SOURCE_DIR}/*.cc)

MESSAGE(STATUS ${PROJECT_SOURCE_DIR})
MESSAGE(STATUS ${cpp_BINARY_DIR})

foreach (source ${SOURCE_FILES})
    string(REPLACE ".cc" "" name ${source})
    string(REPLACE ${PROJECT_SOURCE_DIR}/ "" basename ${name})
    add_executable(${basename} ${source})
endforeach (source ${SOURCE_FILES})
