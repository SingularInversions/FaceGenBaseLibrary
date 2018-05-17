cmake_minimum_required(VERSION 3.5)

project(LibTpBoost)

set(LIBS_ROOT "boost_1_66_0/libs")

file(GLOB_RECURSE SOURCES
    "${LIBS_ROOT}/date_time/src/*.cpp"
    "${LIBS_ROOT}/filesystem/src/*.cpp"
    "${LIBS_ROOT}/serialization/src/*.cpp"
    "${LIBS_ROOT}/system/src/*.cpp"
)
set(THREAD_ROOT ${LIBS_ROOT}/thread)
if(WIN32)
    file(GLOB WIN_THREADS ${THREAD_ROOT}/src/win32/*.cpp)
    set(THREAD_SOURCES ${THREAD_SOURCES} ${WIN_THREADS})
endif(WIN32)
if(UNIX)
    file(GLOB PTHREADS ${THREAD_ROOT}/src/pthread/*.cpp)
    set(THREAD_SOURCES ${THREAD_SOURCES} ${PTHREADS})
endif(UNIX)
set(SOURCES ${SOURCES} ${THREAD_SOURCES})

add_library(LibTpBoost STATIC ${SOURCES})

target_compile_options(LibTpBoost PUBLIC -DBOOST_ALL_NO_LIB)
target_compile_options(LibTpBoost PUBLIC -DBOOST_THREAD_USE_LIB=1)
target_compile_options(LibTpBoost PUBLIC -DBOOST_CONFIG_SUPPRESS_OUTDATED_MESSAGE)
if (MSVC)
    target_compile_options(LibTpBoost
        PRIVATE /W0
        PRIVATE -D_SCL_SECURE_NO_DEPRECATE=1    # not turned off by /W0
	    PRIVATE -D_CRT_SECURE_NO_DEPRECATE=1)
else()
    target_compile_options(LibTpBoost
        PUBLIC -DBOOST_THREAD_POSIX
        PRIVATE -w)
endif()
target_compile_options(LibTpBoost PRIVATE -DBOOST_THREAD_BUILD_LIB=1)

target_include_directories(LibTpBoost PUBLIC boost_1_66_0/)
