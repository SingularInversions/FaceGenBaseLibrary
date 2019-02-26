cmake_minimum_required(VERSION 3.6)

project(fgbl)

file(GLOB SOURCES *.cpp *.hpp)

add_executable(fgbl ${SOURCES})

if(WIN32)
    target_compile_options(fgbl PRIVATE /D_CONSOLE)
    target_link_libraries(fgbl LibFgWin)
else()
    # Nix files must be included in base library, due to single-pass linker:
    target_link_libraries(fgbl LibFgBase)
endif()
