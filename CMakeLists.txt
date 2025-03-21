cmake_minimum_required(VERSION 3.31)

project(vtable-hook VERSION 0.3.1)

#Require C++23 without compiler extensions
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(vtable-hook INTERFACE)
target_include_directories(vtable-hook INTERFACE ${vtable-hook_SOURCE_DIR}/include)
