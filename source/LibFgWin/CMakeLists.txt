cmake_minimum_required(VERSION 3.6)

project(LibFgWin)

file(GLOB SOURCES *.cpp *.hpp *.c *.h)

add_library(LibFgWin STATIC ${SOURCES})

target_link_libraries(LibFgWin
	PUBLIC LibFgBase
    PRIVATE comctl32.lib)
