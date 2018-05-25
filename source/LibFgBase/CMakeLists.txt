cmake_minimum_required(VERSION 3.5)

project(LibFgBase)

file(GLOB SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h)
if(UNIX)
    file(GLOB UNIXSOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/nix/*.cpp)
endif()

add_library(LibFgBase STATIC ${SOURCES} ${UNIXSOURCES})

target_link_libraries(LibFgBase
	PUBLIC LibTpBoost
	PUBLIC LibJpegIjg6b
	PUBLIC LibTntJama
	PRIVATE LibImageMagickCore
	PRIVATE LibUTF-8)

target_include_directories(LibFgBase PUBLIC src)    # Required for dependents to know where to look for include files.

if(UNIX AND NOT ANDROID)                            # gradle takes care of appropriate Bionic libc linkage in this case
    target_link_libraries(LibFgBase
        PRIVATE pthread
        PRIVATE rt)
endif()
