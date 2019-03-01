cmake_minimum_required(VERSION 3.4.1)

add_library(FgBase STATIC IMPORTED)
set_target_properties(FgBase PROPERTIES
  IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../build_android/${ANDROID_ABI}/clang/release/libFgBase.a"
  INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../source/LibFgBase/src"
)

add_library(TpBoost STATIC IMPORTED)
set_target_properties(TpBoost PROPERTIES
  IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../build_android/${ANDROID_ABI}/clang/release/libTpBoost.a"
  INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../source/LibTpBoost/boost_1_67_0"
)

add_library(JpegIjg6b STATIC IMPORTED)
set_target_properties(JpegIjg6b PROPERTIES
  IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../build_android/${ANDROID_ABI}/clang/release/libJpegIjg6b.a"
  INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../../source/LibJpegIjg6b"
)

add_library(random SHARED
            random.cpp)

# Include libraries needed for random lib
target_link_libraries(random
                      android
                      FgBase
                      TpBoost
                      JpegIjg6b)
