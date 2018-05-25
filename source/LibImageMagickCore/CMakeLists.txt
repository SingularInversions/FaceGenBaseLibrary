cmake_minimum_required(VERSION 3.5)

project(LibImageMagickCore)

set(PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/ImageMagick-6.6.2")
file(GLOB MAGICK_SOURCES
    "${PROJECT_ROOT}/bzlib/*.c"
    "${PROJECT_ROOT}/coders/*.c"
    "${PROJECT_ROOT}/jbig/libjbig/*.c"
    "${PROJECT_ROOT}/magick/*.c"
    "${PROJECT_ROOT}/png/*.c"
    "${PROJECT_ROOT}/zlib/*.c"
)
file(GLOB_RECURSE JASPER_SOURCES "${PROJECT_ROOT}/jp2/src/libjasper/*.c")
set(TIFF_ROOT ${PROJECT_ROOT}/tiff/libtiff/)
set(TIFF_SOURCES
    ${TIFF_ROOT}tif_aux.c
    ${TIFF_ROOT}tif_close.c
    ${TIFF_ROOT}tif_codec.c
    ${TIFF_ROOT}tif_color.c
    ${TIFF_ROOT}tif_compress.c
    ${TIFF_ROOT}tif_dir.c
    ${TIFF_ROOT}tif_dirinfo.c
    ${TIFF_ROOT}tif_dirread.c
    ${TIFF_ROOT}tif_dirwrite.c
    ${TIFF_ROOT}tif_dumpmode.c
    ${TIFF_ROOT}tif_error.c
    ${TIFF_ROOT}tif_extension.c
    ${TIFF_ROOT}tif_fax3.c
    ${TIFF_ROOT}tif_fax3sm.c
    ${TIFF_ROOT}tif_flush.c
    ${TIFF_ROOT}tif_getimage.c
    ${TIFF_ROOT}tif_jpeg.c
    ${TIFF_ROOT}tif_luv.c
    ${TIFF_ROOT}tif_lzw.c
    ${TIFF_ROOT}tif_next.c
    ${TIFF_ROOT}tif_ojpeg.c
    ${TIFF_ROOT}tif_open.c
    ${TIFF_ROOT}tif_packbits.c
    ${TIFF_ROOT}tif_pixarlog.c
    ${TIFF_ROOT}tif_predict.c
    ${TIFF_ROOT}tif_print.c
    ${TIFF_ROOT}tif_read.c
    ${TIFF_ROOT}tif_strip.c
    ${TIFF_ROOT}tif_swab.c
    ${TIFF_ROOT}tif_thunder.c
    ${TIFF_ROOT}tif_tile.c
    ${TIFF_ROOT}tif_version.c
    ${TIFF_ROOT}tif_warning.c
    ${TIFF_ROOT}tif_write.c
    ${TIFF_ROOT}tif_zip.c
)
if(WIN32)
    set(TIFF_SOURCES ${TIFF_SOURCES} ${TIFF_ROOT}tif_win32.c)
else()
    set(TIFF_SOURCES ${TIFF_SOURCES} ${TIFF_ROOT}tif_unix.c)
    file(GLOB XLIB_SOURCES "${PROJECT_ROOT}/xlib/*.c")
    set(MAGICK_SOURCES ${MAGICK_SOURCES} ${XLIB_SOURCES})
endif()

add_library(LibImageMagickCore STATIC
    ${MAGICK_SOURCES}
    ${TIFF_SOURCES}
    ${JASPER_SOURCES})

target_compile_options(LibImageMagickCore
    PRIVATE -D_MAGICKLIB
    PRIVATE -DTIFF_PLATFORM_CONSOLE)
if(MSVC)
    target_compile_options(LibImageMagickCore PRIVATE /W0)      # disables warnings for MSVC
else()
    target_compile_options(LibImageMagickCore PRIVATE -w)       # disables warnings for gcc/clang
endif()

target_include_directories(LibImageMagickCore
    PUBLIC  ${PROJECT_ROOT}
    PRIVATE ${PROJECT_ROOT}/bzlib/
    PRIVATE ${PROJECT_ROOT}/tiff/libtiff/
    PRIVATE ${PROJECT_ROOT}/zlib/
    PRIVATE ${PROJECT_ROOT}/png/
    PRIVATE ${PROJECT_ROOT}/xlib/
    PRIVATE ${PROJECT_ROOT}/jbig/libjbig/
    PRIVATE ${PROJECT_ROOT}/jp2/src/libjasper/include/
    PRIVATE ${PROJECT_ROOT}/jp2/src/libjasper/include/jasper/
)

target_link_libraries(LibImageMagickCore PRIVATE LibJpegIjg6b)
