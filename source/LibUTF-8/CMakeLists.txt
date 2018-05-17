cmake_minimum_required(VERSION 3.5)

project(LibUTF-8)

add_library(LibUTF-8 INTERFACE)

target_include_directories(LibUTF-8 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
