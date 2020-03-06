//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgImageIo.hpp"
#include "FgFileSystem.hpp"
#include "FgStdio.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

using namespace std;

namespace Fg {

struct  StbiFree
{
    uchar       *data;
    StbiFree(uchar * d) : data(d) {}
    ~StbiFree() {stbi_image_free(data); }
};

void
loadImage(Ustring const & fname,ImgC4UC & img)
{
    FILE *              fPtr = fgOpen(fname,false);     // Throws if unable to open
    int                 width,height,channels;
    // Request 4 channels if available:
    uchar *             data = stbi_load_from_file(fPtr,&width,&height,&channels,4);
    fclose(fPtr);
    if (data == nullptr) {
        FgException     e;
        e.pushMsg("Unable to decode image",stbi__g_failure_reason);
        e.pushMsg("Unable to load image from file",fname.m_str);
        throw e;
    }
    StbiFree            sf(data);   // Can't use ScopeGuard since 'stbi_image_free' is extern C
    if (width*height <= 0)
        fgThrow("Invalid image dimensions",Vec2I(width,height));
    img = ImgC4UC(Vec2UI(width,height),reinterpret_cast<RgbaUC*>(data));
}

ImgC4UC
loadImage(Ustring const & fname)
{
    ImgC4UC         ret;
    loadImage(fname,ret);
    return ret;
}

static
void
writeToFile(void *context,void * data,int size)
{
    fwrite(data,1,size,reinterpret_cast<FILE*>(context));
}

void
saveImage(Ustring const & fname,const ImgC4UC & img)
{
    if (img.numPixels() == 0)
        fgThrow("Cannot save empty image to file",fname);
    Path              path(fname);
    Ustring            ext = toLower(path.ext);
    if (ext.empty())
        fgThrow("No image file extension specified",fname);
    if (!contains(imgFileExtensions(),ext.m_str))
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

void
saveJfif(ImgC4UC const & img,Ustring const & fname,uint quality)
{
    if (fname.empty())
        fgThrow("Cannot save image to empty filename");
    if (img.numPixels() == 0)
        fgThrow("Cannot save empty image to file",fname);
    if (quality > 100)
        fgThrow("Invalid JPEG quality value",quality);
    uint                wid = img.width(),
                        hgt = img.height();
    const uchar         *data = &img.m_data[0].m_c[0];
    FILE                *fPtr = fgOpen(fname,true);
    int                 ret;
    ret = stbi_write_jpg_to_func(writeToFile,fPtr,wid,hgt,4,data,int(quality));
    fclose(fPtr);
    if (ret == 0)
        fgThrow("Unable to save JFIF image, check drive free space.",fname);
}

}
