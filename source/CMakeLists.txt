cmake_minimum_required(VERSION 3.5)     # Use guidelines from 'Modern Cmake' talks

project(cmake CXX)

# Global compile options:
set(CMAKE_CXX_STANDARD 11)		                    # C++11 is required for this package
# Using 'add_compile_options' adds to both CPP and C flags (as appropriate) but requires using annoying 'generators':
add_compile_options($<$<CONFIG:Debug>:-D_DEBUG>)    # add_definitions does not accept generators
if (MSVC)
	add_definitions(
        -DWIN32
        -D_UNICODE
        -D_CRT_SECURE_NO_WARNINGS)
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
        -Wno-unused-result
        -m64
        -fPIC
        $<$<CONFIG:Debug>:-g>
        $<$<CONFIG:Debug>:-O0>
        $<$<NOT:$<CONFIG:Debug>>:-ffast-math>)
elseif(CMAKE_CXX_COMPILER_ID MATCHES Clang)
    add_compile_options(
        -Wextra
        -Wno-unused-local-typedef
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

add_subdirectory(LibTpBoost)
add_subdirectory(LibJpegIjg6b)
add_subdirectory(LibImageMagickCore)
add_subdirectory(LibUTF-8)
add_subdirectory(LibTntJama)
add_subdirectory(LibFgBase)
if(WIN32)
    add_subdirectory(LibFgWin)
endif()
add_subdirectory(fgbl)
