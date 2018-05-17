cmake_minimum_required(VERSION 3.5)

project(LibJpegIjg6b)

file(GLOB SOURCES "*.c")

add_library(LibJpegIjg6b STATIC ${SOURCES})

target_include_directories(LibJpegIjg6b PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if (MSVC)
    target_compile_options(LibJpegIjg6b PRIVATE /W0)
else()
    target_compile_options(LibJpegIjg6b PRIVATE -w)
endif()
