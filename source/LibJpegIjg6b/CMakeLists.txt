cmake_minimum_required(VERSION 3.6)

project(LibJpegIjg6b)

file(GLOB SOURCES *.c *.h)

add_library(LibJpegIjg6b STATIC ${SOURCES})

target_include_directories(LibJpegIjg6b PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})     # Required for dependent includes

if (MSVC)
    target_compile_options(LibJpegIjg6b PRIVATE /W0)
else()
    target_compile_options(LibJpegIjg6b PRIVATE -w)
endif()
