cmake_minimum_required(VERSION 3.5)

project(LibTntJama)

add_library(LibTntJama INTERFACE)

target_include_directories(LibTntJama INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
