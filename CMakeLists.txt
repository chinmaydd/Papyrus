cmake_minimum_required(VERSION 3.13)
project(Papyrus)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE debug)

include_directories(include)
# XXX: Use a better structure here.
include_directories(src)

add_subdirectory(src)
