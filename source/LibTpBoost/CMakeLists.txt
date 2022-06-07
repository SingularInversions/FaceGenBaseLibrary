cmake_minimum_required(VERSION 3.6)

project(LibTpBoost)

set(LIBS_ROOT "boost_1_69_0/libs")

file(GLOB_RECURSE SOURCES
    "${LIBS_ROOT}/date_time/src/*.cpp"
    "${LIBS_ROOT}/filesystem/src/*.cpp"
    "${LIBS_ROOT}/serialization/src/*.cpp"
    "${LIBS_ROOT}/system/src/*.cpp"
)

add_library(LibTpBoost STATIC ${SOURCES})

target_compile_options(LibTpBoost PUBLIC -DBOOST_ALL_NO_LIB)
target_compile_options(LibTpBoost PUBLIC -DBOOST_CONFIG_SUPPRESS_OUTDATED_MESSAGE)
if (MSVC)
    target_compile_options(LibTpBoost
        PRIVATE /W2)
else()
    target_compile_options(LibTpBoost
        PRIVATE -w)
endif()

target_include_directories(LibTpBoost PUBLIC boost_1_69_0/)
