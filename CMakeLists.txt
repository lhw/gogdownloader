#Configure to your liking
set(CMAKE_C_COMPILER clang)
set(CMAKE_BUILD_TYPE DEBUG)

cmake_minimum_required(VERSION 2.6)
project(gogdownloader)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake/)

add_subdirectory(src)
