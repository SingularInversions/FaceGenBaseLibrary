cmake_minimum_required(VERSION 3.5)

set(ANDROID True)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../source/LibUTF-8 ${CMAKE_CURRENT_BINARY_DIR}/LibUTF-8)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../source/LibTntJama ${CMAKE_CURRENT_BINARY_DIR}/LibTntJama)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../source/LibJpegIjg6b ${CMAKE_CURRENT_BINARY_DIR}/LibJpegIjg6b)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../source/LibTpBoost ${CMAKE_CURRENT_BINARY_DIR}/LibTpBoost)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../source/LibImageMagickCore ${CMAKE_CURRENT_BINARY_DIR}/LibImageMagickCore)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../source/LibFgBase ${CMAKE_CURRENT_BINARY_DIR}/LibFgBase)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../source/LibFgNix ${CMAKE_CURRENT_BINARY_DIR}/LibFgNix)

# Android also supports STATIC libs as well:
add_library(native-lib SHARED src/main/cpp/native-lib.cpp )

# Searches for prebuilt library 'log' and stores the path in variable 'log-lib'.
# Because CMake includes system libraries in the search path by default,
# you only need to specify the name of the public NDK library you want to add.
# CMake verifies that the library exists before completing its build:
find_library(log-lib log )

target_link_libraries(native-lib LibFgBase LibFgNix ${log-lib} )
