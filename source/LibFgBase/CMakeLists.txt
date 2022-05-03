cmake_minimum_required(VERSION 3.6)

project(LibFgBase)

file(GLOB SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h)
if(UNIX)
    file(GLOB UNIXSOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/nix/*.cpp)
endif()

add_library(LibFgBase STATIC ${SOURCES} ${UNIXSOURCES})

target_link_libraries(LibFgBase
	PUBLIC LibTpBoost
	PRIVATE LibTpStb
	PRIVATE LibTpEigen)

target_include_directories(LibFgBase PUBLIC src)    # Required for dependents to know where to look for include files.

# gradle takes care of appropriate Bionic libc linkage for Android.

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    target_link_libraries(LibFgBase
        PUBLIC pthread)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")       # MacOS
    find_library(CARBON_LIBRARY Carbon)
    target_link_libraries(LibFgBase
        PUBLIC ${CARBON_LIBRARY})
endif()
