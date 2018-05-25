cmake_minimum_required(VERSION 3.5)

project(LibFgWin)

file(GLOB SOURCES *.cpp *.hpp *.c *.h)

add_library(LibFgWin STATIC ${SOURCES})

target_link_libraries(LibFgWin
	PUBLIC LibFgBase
    PRIVATE opengl32.lib
    PRIVATE comctl32.lib)
