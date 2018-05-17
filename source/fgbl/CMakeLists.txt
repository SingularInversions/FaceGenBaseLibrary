cmake_minimum_required(VERSION 3.5)

project(fgbl)

add_executable(fgbl fgbl.cpp)

if(WIN32)
    target_compile_options(fgbl PRIVATE /D_CONSOLE)
    target_link_libraries(fgbl PRIVATE LibFgWin)
else()
    # ld is a single-pass linker and functions in LibFgBase reference functions in LibFgNix so we need to link LibFgBase
    # before LibFgNix even though it's also implicitly linked after LibFgNix:
    target_link_libraries(fgbl
        PRIVATE LibFgBase
        PRIVATE LibFgNix)
endif()
