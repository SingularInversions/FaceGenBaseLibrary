//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:Sohail Somani
//

#include "stdafx.h"

#include "jinclude.h"
#include "jpeglib.h"
#include "jerror.h"

#include "FgStdLibs.hpp"

struct my_destination_manager
{
    jpeg_destination_mgr pub;
    JOCTET * buffer;
    std::vector<JOCTET> * target;
};

typedef my_destination_manager * my_dest_ptr_t;

static const size_t OUTPUT_BUF_SIZE=4096;

METHODDEF(void)
init_destination(j_compress_ptr cinfo)
{
    my_dest_ptr_t dest = (my_dest_ptr_t) cinfo->dest;
    dest->buffer = (JOCTET *)
        (*cinfo->mem->alloc_small)((j_common_ptr) cinfo, JPOOL_IMAGE,
                                   OUTPUT_BUF_SIZE * sizeof(JOCTET));
    dest->pub.next_output_byte = dest->buffer;
    dest->pub.free_in_buffer = OUTPUT_BUF_SIZE;
}

METHODDEF(boolean)
empty_output_buffer (j_compress_ptr cinfo)
{
    my_dest_ptr_t dest = (my_dest_ptr_t) cinfo->dest;
    try
    {
        dest->target->insert(dest->target->end(),
                             dest->buffer,
                             dest->buffer + OUTPUT_BUF_SIZE);
        dest->pub.next_output_byte = dest->buffer;
        dest->pub.free_in_buffer = OUTPUT_BUF_SIZE;
    }
    catch(std::exception const &)
    {
        ERREXIT(cinfo,JERR_FILE_WRITE);
    }
    return TRUE;
}

METHODDEF(void)
term_destination(j_compress_ptr cinfo)
{
    my_dest_ptr_t dest = (my_dest_ptr_t) cinfo->dest;
    size_t datacount = OUTPUT_BUF_SIZE - dest->pub.free_in_buffer;

        // Write remaining data
    if(datacount > 0)
    {
        try
        {
            dest->target->insert(dest->target->end(),
                                 dest->buffer,
                                 dest->buffer + datacount);
        }
        catch(std::exception const &)
        {
            ERREXIT(cinfo,JERR_FILE_WRITE);
        }
    }
}

// The target buffer can be sized to prevent unnecessary resizing.
// This is optional. The target buffer must stay in scope until
// encoding is complete!
GLOBAL(void)
jpeg_mem_dest(j_compress_ptr cinfo,
              std::vector<JOCTET> & target)
{
    if(cinfo->dest == NULL)
    {
        cinfo->dest = (struct jpeg_destination_mgr *)
            (*cinfo->mem->alloc_small)((j_common_ptr) cinfo, JPOOL_IMAGE,
                                       SIZEOF(my_destination_manager));
    }
    my_dest_ptr_t dest = (my_dest_ptr_t) cinfo->dest;
    dest->pub.init_destination = init_destination;
    dest->pub.empty_output_buffer = empty_output_buffer;
    dest->pub.term_destination = term_destination;
    dest->target = &target;
}
