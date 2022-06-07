# Uses preferred declarative form (see 'Modern Cmake' talks)
cmake_minimum_required(VERSION 3.6)                # Highest version supported by Android (via gradle)

project(FaceGen CXX)

# Give cmake a more sensible default than no optimization and no debug info:
# Check must be done inside project since 'CMAKE_CONFIGURATION_TYPES' not defined until then.
if(NOT DEFINED CMAKE_BUILD_TYPE AND NOT DEFINED CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Defaulting to RELEASE configuration")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "(Debug | Release)")
endif()

# Global compile options:
set(CMAKE_CXX_STANDARD 14)                          # C++14 is required for this package
# Using 'add_compile_options' adds to both CPP and C flags (as appropriate) using annoying 'generators':
add_compile_options($<$<CONFIG:Debug>:-D_DEBUG>)    # add_definitions does not accept generators
if (MSVC)
    add_definitions(
        -DWIN32
        -D_UNICODE)
    add_compile_options(
        $<$<CONFIG:Debug>:/MTd>                     # Use static C runtime libraries to avoid DLL hell
        $<$<NOT:$<CONFIG:Debug>>:/MT>
        /EHsc                                       # Enable excpetions (required for this package)
        /W4                                         # Enhanced warnings
        /fp:fast                                    # Fast floating point optimizations (allow imprecise)
        $<$<CONFIG:Debug>:/Od>                      # Disable optimizations for Debug build (not cmake default)
        $<$<NOT:$<CONFIG:Debug>>:-DNDEBUG>)        
elseif(CMAKE_CXX_COMPILER_ID MATCHES GNU)
    add_compile_options(
        -Wextra
        -m64
        -fPIC
        $<$<CONFIG:Debug>:-g>
        $<$<CONFIG:Debug>:-O0>
        $<$<NOT:$<CONFIG:Debug>>:-ffast-math>)
elseif(CMAKE_CXX_COMPILER_ID MATCHES Clang)
    add_compile_options(
        -Wextra
        -fno-common
        -ftemplate-depth=1024
        -m64
        -fPIC
        $<$<CONFIG:Debug>:-g>
        $<$<CONFIG:Debug>:-O0>
        $<$<NOT:$<CONFIG:Debug>>:-Ofast>)
elseif(CMAKE_CXX_COMPILER_ID MATCHES Intel)
    add_compile_options(
        -Wextra
        -diag-disable=11074,11076
        $<$<CONFIG:Debug>:-g>
        $<$<NOT:$<CONFIG:Debug>>:-Ofast>)
endif()

# Best practice is to use 'add_subdirectory' for each target. This does 2 things:
# 1. Creates a corresponding subdirectory in the build.
# 2. Creates a new (inheriting) variable scope for the target.
# Unfortunately this also means we have many instances of the same filename (CMakeLists.txt).
add_subdirectory(LibTpBoost)
add_subdirectory(LibTpStb)
add_subdirectory(LibTpEigen)
add_subdirectory(LibFgBase)
if(WIN32)
    add_subdirectory(LibFgWin)
endif()
add_subdirectory(fgbl)
