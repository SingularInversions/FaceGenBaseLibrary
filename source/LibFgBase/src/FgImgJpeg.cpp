//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"
#include <csetjmp>
#include "FgStdLibs.hpp"
#include "FgStdStream.hpp"
#include "FgException.hpp"
#include "FgImage.hpp"

#ifdef _MSC_VER
// _setjmp and C++ object destruction is non-portable.
// This means that the code below which uses setjmp CANNOT allocate ANY C++ objects while
// longjmp can be called. Relying on longjmp to deallocate stack objects is unportable.
#  pragma warning(disable:4611)
#endif

extern "C" {
#include "jpeglib.h"
    // defined in jpeg_mem_src.c
    void jpeg_mem_src(j_decompress_ptr cinfo,
                      const JOCTET * buffer, size_t size);
}

// Defined in jpeg_mem_dest.cpp
void jpeg_mem_dest(j_compress_ptr cinfo,
                   std::vector<JOCTET> & target);

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

static
bool
loadJpeg(
    const vector<uchar> &   jpgBuffer,
    FgImgRgbaUb &           img)
{
    jpeg_decompress_struct cinfo;
    IJGErrorManager jerr;

    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = fgIJGErrorExit;

    bool succeeded = false;

    vector<uchar>       buff;

    switch(setjmp(jerr.setjmp_buffer))
    {
        case 0:
        {
            // DO NOT ALLOCATE ANY C++ OBJECTS HERE ELSE
            // THERE COULD BE A MEMORY LEAK
            jpeg_create_decompress(&cinfo);
            jpeg_mem_src(&cinfo,&jpgBuffer[0],jpgBuffer.size());
            jpeg_read_header(&cinfo,TRUE);

                // We need to do this because libjpeg does not do
                // a very good job at guessing. Unfortunately, I think that
                // this means if we have a 3 component JPEG with RGB components,
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

                // Here we use the library's state variable cinfo.output_scanline as the
                // loop counter, so that we don't have to keep track ourselves.
                //
            img.resize(cinfo.output_width,cinfo.output_height);
            buff.resize(img.width()*3);
            uchar*              buffer = &buff[0];
            uint                row = 0;
            while (cinfo.output_scanline < cinfo.output_height)
            {
                    // jpeg_read_scanlines expects an array of pointers to scanlines.
                    // Here the array is only one element long, but you could ask for
                    // more than one scanline at a time if that's more convenient.
                jpeg_read_scanlines(&cinfo,&buffer,1);
                uchar         *ptr = buffer;
                for (uint col=0; col<img.width(); col++)
                {
                    img.elem(col,row).red() = *ptr++;
                    img.elem(col,row).green() = *ptr++;
                    img.elem(col,row).blue() = *ptr++;
                    img.elem(col,row).alpha() = 255;
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
    return succeeded;
}

static
bool
saveJpeg(
    const FgImgRgbaUb &     img,
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

    vector<uchar> img_buffer(img.height() * img.width() * 3);

    switch(setjmp(jerr.setjmp_buffer))
    {
        case 0:
        {
            // DO NOT ALLOCATE ANY C++ OBJECTS HERE ELSE
            // THERE COULD BE A MEMORY LEAK
            jpeg_create_compress(&cinfo);
            jpeg_mem_dest(&cinfo, jpgBuffer);

            cinfo.image_width = img.width();
            cinfo.image_height = img.height();
            cinfo.input_components = 3;
            cinfo.in_color_space = JCS_RGB;

            jpeg_set_defaults(&cinfo);

            jpeg_set_quality(&cinfo,quality,TRUE /* limit to baseline range*/ );
            jpeg_set_colorspace(&cinfo,JCS_YCbCr);

            jpeg_start_compress(&cinfo, TRUE);

            uchar * imgPtr = &img_buffer[0];
            for(uint yy = 0; yy < img.height(); yy++)
            {
                for(uint xx=0; xx<img.width(); xx++)
                {
                    *imgPtr++ = img.elem(xx,yy).red();
                    *imgPtr++ = img.elem(xx,yy).green();
                    *imgPtr++ = img.elem(xx,yy).blue();
                }
            }

            uint row_stride = img.width() * 3;
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

void
fgImgSaveJfif(
    const FgImgRgbaUb & img,
    const FgString &    fname,
    int                 quality)
{
    vector<uchar>       target;
    if(!saveJpeg(img,target,quality)) 
        fgThrow("Could not encode as JFIF",fname);
    FgOfstream  ofs(fname);
    ofs.setf(std::ios::skipws);
    std::ostream_iterator<unsigned char> output_file(ofs);
    std::copy(target.begin(),target.end(),
              output_file);
}

void
fgImgSaveJfif(
    const FgImgRgbaUb &     img,
    std::vector<uchar> &    buffer,
    int                     quality)
{
    if(!saveJpeg(img,buffer,quality)) 
        fgThrow("Could not encode as JFIF");
}

void
fgImgLoadJfif(
    const FgString &    fname,
    FgImgRgbaUb &       img)
{
    FgIfstream  ifs(fname);

    ifs.seekg(0,std::ios::end);
    std::streamsize size = ifs.tellg();
    ifs.seekg(0,std::ios::beg);
    
    vector<uchar>       source;
    source.resize(static_cast<std::size_t>(size));
    ifs.read(reinterpret_cast<char*>(&source[0]),size);
    
    if(!loadJpeg(source,img))
        fgThrow("Error processing JFIF data",fname);
}

void 
fgImgLoadJfif(
    const vector<uchar> &   data,
    FgImgRgbaUb &           img)
{
    if(!loadJpeg(data,img))
        fgThrow("Error processing JFIF data");
}
