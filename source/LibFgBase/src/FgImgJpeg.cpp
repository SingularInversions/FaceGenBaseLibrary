//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Sohail Somani, Andrew Beatty

#include "stdafx.h"

#include <csetjmp>
#include "FgStdLibs.hpp"
#include "FgStdStream.hpp"
#include "FgException.hpp"
#include "FgImage.hpp"
#include "FgStdString.hpp"

#ifdef _MSC_VER
// _setjmp and C++ object destruction is non-portable.
// This means that the code below which uses setjmp CANNOT allocate ANY C++ objects while
// longjmp can be called. Relying on longjmp to deallocate stack objects is unportable.
#  pragma warning(disable:4611)
#endif

extern "C" {
#include "jpeglib.h"
}

// defined in jpeg_mem_src.cpp
void jpeg_mem_src(j_decompress_ptr cinfo,const JOCTET * buffer, size_t size);

// Defined in jpeg_mem_dest.cpp
void jpeg_mem_dest(j_compress_ptr cinfo,std::vector<JOCTET> & target);

struct IJGErrorManager
{
    struct jpeg_error_mgr pub;
    int dummy[2];               // Align for x64
    jmp_buf setjmp_buffer;
};

enum {IJG_OK = 0, IJG_FAILED = 1};

typedef IJGErrorManager * error_mgr_ptr_t;

METHODDEF(void)
fgIJGErrorExit(j_common_ptr cinfo)
{
    error_mgr_ptr_t p = reinterpret_cast<error_mgr_ptr_t>(cinfo->err);
    longjmp(p->setjmp_buffer,IJG_FAILED);
}

using namespace std;

namespace Fg {

static
bool
loadJpeg(
    const vector<uchar> &   jpgBuffer,
    ImgC4UC &           img)
{
    jpeg_decompress_struct cinfo;
    IJGErrorManager jerr;

    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = fgIJGErrorExit;

    bool                succeeded = false;
    bool                allocError = false;
    Vec2UI           allocErrorSz;

    vector<uchar>       buff;

    switch(setjmp(jerr.setjmp_buffer))
    {
        case 0:
        {
            // DO NOT ALLOCATE ANY C++ OBJECTS HERE OR ELSE THERE COULD BE A MEMORY LEAK
            jpeg_create_decompress(&cinfo);
            jpeg_mem_src(&cinfo,&jpgBuffer[0],jpgBuffer.size());
            jpeg_read_header(&cinfo,TRUE);

            // We need to do this because libjpeg does not do a very good job at guessing.
            // Unfortunately, I think that this means if we have a 3 component JPEG with RGB components,
            // that it will be severely misinterpreted.
            switch(cinfo.num_components)
            {
            case 1:
                cinfo.jpeg_color_space = JCS_GRAYSCALE;
                break;
            case 3:
                cinfo.jpeg_color_space = JCS_YCbCr;
                break;
            default:
                cinfo.jpeg_color_space = JCS_UNKNOWN;
                break;
            }
            // We always want RGB out
            cinfo.out_color_space = JCS_RGB;
            jpeg_start_decompress(&cinfo);
            // Must try-catch C++ allocations to avoid memory leaks here:
            try {
                img.resize(cinfo.output_width,cinfo.output_height);
                buff.resize(img.width()*3);
            }
            catch(...)
            {
                succeeded = false;
                allocError = true;
                allocErrorSz = Vec2UI(cinfo.output_width,cinfo.output_height);
                goto cleanup;
            }
            uchar*              buffer = &buff[0];
            uint                row = 0;
            // Here we use the library's state variable cinfo.output_scanline as the
            // loop counter, so that we don't have to keep track ourselves:
            while (cinfo.output_scanline < cinfo.output_height) {
                // jpeg_read_scanlines expects an array of pointers to scanlines.
                // Here the array is only one element long, but you could ask for
                // more than one scanline at a time if that's more convenient:
                jpeg_read_scanlines(&cinfo,&buffer,1);
                uchar         *ptr = buffer;
                for (uint col=0; col<img.width(); col++) {
                    img.xy(col,row).red() = *ptr++;
                    img.xy(col,row).green() = *ptr++;
                    img.xy(col,row).blue() = *ptr++;
                    img.xy(col,row).alpha() = 255;
                }
                row++;
            }
            jpeg_finish_decompress(&cinfo);
            goto ok;
        }
        default: // this should never ever happen
        {
        }
        case IJG_FAILED:
        {
            goto failure;
        }
    }
failure:
    succeeded = false;
    goto cleanup;
ok:
    succeeded = true;
    goto cleanup;
cleanup:
    jpeg_destroy_decompress(&cinfo);
    if (allocError)
        fgThrow("Allocation error in loadJpeg for size: "+toStr(allocErrorSz));
    return succeeded;
}

static
bool
saveJpeg(
    uint                    wid,
    uint                    hgt,
    const uchar *           srcImg,     // Must be RGBA of size wid*hgt*4
    vector<unsigned char> & jpgBuffer,
    int                     quality)
{
    jpeg_compress_struct cinfo;
    JSAMPROW row_pointer[1];

        // Resize buffer to 0 since the jpeg to-memory encoder
        // will append to the buffer
    jpgBuffer.clear();

    IJGErrorManager jerr;
    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = fgIJGErrorExit;

    bool succeeded = false;

    vector<uchar> img_buffer(wid*hgt*3);

    switch(setjmp(jerr.setjmp_buffer))
    {
        case 0:
        {
            // DO NOT ALLOCATE ANY C++ OBJECTS HERE ELSE
            // THERE COULD BE A MEMORY LEAK
            jpeg_create_compress(&cinfo);
            jpeg_mem_dest(&cinfo, jpgBuffer);

            cinfo.image_width = wid;
            cinfo.image_height = hgt;
            cinfo.input_components = 3;
            cinfo.in_color_space = JCS_RGB;

            jpeg_set_defaults(&cinfo);

            jpeg_set_quality(&cinfo,quality,TRUE /* limit to baseline range*/ );
            jpeg_set_colorspace(&cinfo,JCS_YCbCr);

            jpeg_start_compress(&cinfo, TRUE);

            for(uint yy=0; yy<hgt; yy++) {
                const uchar *   srcPtr = srcImg + yy*wid*4;
                size_t          dstOff = yy*wid*3;
                for(uint xx=0; xx<wid; xx++) {
                    img_buffer[dstOff+xx*3] = srcPtr[xx*4];
                    img_buffer[dstOff+xx*3+1] = srcPtr[xx*4+1];
                    img_buffer[dstOff+xx*3+2] = srcPtr[xx*4+2];
                }
            }

            uint row_stride = wid * 3;
            while(cinfo.next_scanline < cinfo.image_height)
            {
                row_pointer[0] = &img_buffer[cinfo.next_scanline * row_stride];
                jpeg_write_scanlines(&cinfo,row_pointer,1);
            }
            jpeg_finish_compress(&cinfo);
            goto ok;
        }
        default: // this should never ever happen
        {
        }
        case IJG_FAILED:
        {
            goto failure;
        }
    }
failure:
    succeeded = false;
    goto cleanup;
ok:
    succeeded=true;
    goto cleanup;
cleanup:
    jpeg_destroy_compress(&cinfo);
    return succeeded;
}

std::vector<uchar>
imgEncodeJpeg(uint wid,uint hgt,const uchar * data,int quality)
{
    vector<uchar>       ret;
    if(!saveJpeg(wid,hgt,data,ret,quality)) 
        fgThrow("Could not encode as JPEG/JFIF");
    return ret;
}

vector<uchar>
imgEncodeJpeg(const ImgC4UC & img,int quality)
{
    vector<uchar>       ret;
    if(!saveJpeg(img.width(),img.height(),&img.m_data[0].m_c[0],ret,quality)) 
        fgThrow("Could not encode as JPEG/JFIF");
    return ret;
}

ImgC4UC
imgDecodeJpeg(Uchars const & data)
{
    ImgC4UC         ret;
    if(!loadJpeg(data,ret))
        fgThrow("Could not decode as JPEG/JFIF");
    return ret;
}

}
