/**
 * Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
 * Use, modification and distribution is subject to the MIT License,
 * see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
 *
 * Authors:Sohail Somani
 **/

#include "stdafx.h"

/* Visual C++ sal.h uses single line comments */
#if defined(_MSC_VER)
#  pragma warning(push)
#  pragma warning(disable:4001)
#endif

extern "C" {
#include "jinclude.h"
#include "jpeglib.h"
#include "jerror.h"
}

#if defined(_MSC_VER)
#  pragma warning(pop)
#endif

struct my_source_mgr
{
    struct jpeg_source_mgr pub; /* public fields/base class */
};

typedef struct my_source_mgr * my_src_ptr;

METHODDEF(void)
init_source(j_decompress_ptr cinfo)
{
    (void)cinfo; /* unused */
}

METHODDEF(boolean)
fill_input_buffer(j_decompress_ptr cinfo)
{
        /* This function should never get called! */
    ERREXIT(cinfo,JERR_INPUT_EOF);
    return TRUE; /* Never gets here anyway */
}

METHODDEF(void)
skip_input_data(j_decompress_ptr cinfo, long num_bytes)
{
    my_src_ptr src = (my_src_ptr) cinfo->src;
    if(src->pub.bytes_in_buffer < (size_t)num_bytes)
    {
        ERREXIT(cinfo,JERR_INPUT_EOF);
    }
    src->pub.next_input_byte += (size_t) num_bytes;
    src->pub.bytes_in_buffer -= (size_t) num_bytes;
}

METHODDEF(void)
term_source(j_decompress_ptr cinfo)
{
    (void)cinfo; /* unused */
    /* no-op */
}

GLOBAL(void)
jpeg_mem_src(j_decompress_ptr cinfo,
             const JOCTET * buffer, size_t size)
{
    my_src_ptr src;
    if(cinfo->src == NULL)
    {
        cinfo->src = (struct jpeg_source_mgr *)
            (*cinfo->mem->alloc_small)((j_common_ptr) cinfo, JPOOL_IMAGE,
                                       SIZEOF(struct my_source_mgr));
        src = (my_src_ptr) cinfo->src;
    }

    src = (my_src_ptr) cinfo->src;
    src->pub.init_source = init_source;
    src->pub.fill_input_buffer = fill_input_buffer;
    src->pub.skip_input_data = skip_input_data;
    src->pub.resync_to_restart = jpeg_resync_to_restart;
    src->pub.term_source = term_source;
    src->pub.bytes_in_buffer = size;
    src->pub.next_input_byte = buffer;
}
