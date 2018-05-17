cmake_minimum_required(VERSION 3.5)

project(LibFgNix)

file(GLOB SOURCES "*.cpp")

add_library(LibFgNix STATIC ${SOURCES})

target_link_libraries(LibFgNix PUBLIC LibFgBase)
if(NOT ANDROID)     # gradle takes care of appropriate Bionic libc linkage in this case
    target_link_libraries(LibFgNix
        PRIVATE pthread
        PRIVATE rt)
endif()
