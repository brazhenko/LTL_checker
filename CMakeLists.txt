cmake_minimum_required(VERSION 3.27)
project(ltl)

set(CMAKE_CXX_STANDARD 20)

add_executable(ltl main.cpp ptl_graph.cpp)
#SET(CMAKE_CXX_FLAGS_DEBUG "-O0")
#SET(CMAKE_C_FLAGS_DEBUG "-O0")
#target_compile_options(ltl PRIVATE -pg)
#target_link_libraries(ltl PRIVATE -pg)


add_executable(graph graph.cpp)