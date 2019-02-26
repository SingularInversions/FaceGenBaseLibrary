//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Andrew Beatty
// Created: 19.01.21
//

#include "stdafx.h"

#include "FgImageIo.hpp"
#include "FgFileSystem.hpp"
#include "FgScopeGuard.hpp"
#include "FgStdio.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

using namespace std;

struct  StbiFree
{
    uchar       *data;
    StbiFree(uchar * d) : data(d) {}
    ~StbiFree() {stbi_image_free(data); }
};

void
fgLoadImgAnyFormat(const FgString & fname,FgImgRgbaUb & img)
{
    FILE *              fPtr = fgOpen(fname,false);     // Throws if unable to open
    int                 width,height,channels;
    // Request 4 channels if available:
    uchar *             data = stbi_load_from_file(fPtr,&width,&height,&channels,4);
    fclose(fPtr);
    if (data == nullptr) {
        FgException     e;
        e.pushMsg("Unable to decode image",stbi__g_failure_reason);
        e.pushMsg("Unable to load image",fname.m_str);
        throw e;
    }
    StbiFree            sf(data);   // Can't use FgScopeGuard since 'stbi_image_free' is extern C
    if (width*height <= 0)
        fgThrow("Invalid image dimensions",FgVect2I(width,height));
    img = FgImgRgbaUb(FgVect2UI(width,height),reinterpret_cast<FgRgbaUB*>(data));
}

static
void
writeToFile(void *context,void * data,int size)
{
    fwrite(data,1,size,reinterpret_cast<FILE*>(context));
}

void
fgSaveImgAnyFormat(const FgString & fname,const FgImgRgbaUb & img)
{
    if (img.numPixels() == 0)
        fgThrow("Cannot save empty image to file",fname);
    FgPath              path(fname);
    FgString            ext = fgToLower(path.ext);
    if (ext.empty())
        fgThrow("No image file extension specified",fname);
    if (!fgContains(fgSvec<string>("jpg","jpeg","png","bmp","tga"),ext.m_str))
        fgThrow("File extension is not a supported image output format",fname);
    uint                wid = img.width(),
                        hgt = img.height();
    const uchar         *data = &img.m_data[0].m_c[0];
    FILE                *fPtr = fgOpen(fname,true);
    int                 ret;
    if ((ext == "jpg" || ext == "jpeg"))
        ret = stbi_write_jpg_to_func(writeToFile,fPtr,wid,hgt,4,data,75);
    else if (ext == "png")
        ret = stbi_write_png_to_func(writeToFile,fPtr,wid,hgt,4,data,wid*4);
    else if (ext == "bmp")
        ret = stbi_write_bmp_to_func(writeToFile,fPtr,wid,hgt,4,data);
    else // (ext == "tga")
        ret = stbi_write_tga_to_func(writeToFile,fPtr,wid,hgt,4,data);
    fclose(fPtr);
    if (ret == 0)
        fgThrow("Unable to write image, check drive free space.",fname);
}
