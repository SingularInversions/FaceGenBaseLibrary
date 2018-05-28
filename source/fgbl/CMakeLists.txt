cmake_minimum_required(VERSION 3.5)

project(fgbl)

file(GLOB SOURCES *.cpp *.hpp)

add_executable(fgbl ${SOURCES})

if(WIN32)
    target_compile_options(fgbl PRIVATE /D_CONSOLE)
    target_link_libraries(fgbl LibFgWin)
else()
    # In this case, nix files are included in base library, as they must be under a single-pass linker model:
    target_link_libraries(fgbl LibFgBase)
endif()
