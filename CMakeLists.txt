cmake_minimum_required(VERSION 3.13)
project(Papyrus)

set(CMAKE_CXX_STANDARD 11)

include_directories(include)
# XXX: Use a better structure here.
include_directories(src)

add_subdirectory(src)
