cmake_minimum_required(VERSION 3.15)
project(libusb C)

set(CMAKE_C_STANDARD 11)

project(libusb VERSION 0.0.1 DESCRIPTION "...")
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_library(libusb STATIC ${CMAKE_CURRENT_SOURCE_DIR}/lib/gcc/libusb.a)
