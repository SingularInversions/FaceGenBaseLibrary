cmake_minimum_required(VERSION 3.5)

project(LibFgBase)

file(GLOB SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)

add_library(LibFgBase STATIC ${SOURCES})

target_include_directories(LibFgBase PUBLIC src)
target_link_libraries(LibFgBase
	PUBLIC LibTpBoost
	PRIVATE LibImageMagickCore
	PRIVATE LibTntJama
	PRIVATE LibJpegIjg6b
	PRIVATE LibUTF-8)
