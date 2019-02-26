cmake_minimum_required(VERSION 3.6)

project(LibTpStb)

add_library(LibTpStb INTERFACE)

target_include_directories(LibTpStb INTERFACE stb)
