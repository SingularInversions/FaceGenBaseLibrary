cmake_minimum_required(VERSION 3.6)

project(LibTpEigen)

add_library(LibTpEigen INTERFACE)

target_include_directories(LibTpEigen INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
