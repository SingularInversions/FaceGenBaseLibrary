//
// Coypright (c) 2022 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgImageIo.hpp"
#include "FgFileSystem.hpp"
#include "FgStdio.hpp"
#include "FgCommand.hpp"
#include "FgImgDisplay.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

using namespace std;

namespace Fg {

struct      StbiFree
{
    uchar       *data;
    StbiFree(uchar * d) : data(d) {}
    ~StbiFree() {stbi_image_free(data); }
};

void                loadImage_(String8 const & fname,ImgRgba8 & img)
{
    int                 width,height,channels;
    uchar *             data = nullptr;
    FILE *              fPtr = openFile(fname,false);     // Throws if unable to open, never return nullptr
    FGASSERT(fPtr != nullptr);
    data = stbi_load_from_file(fPtr,&width,&height,&channels,4);    // request 4 channels. Can't throw.
    fclose(fPtr);
    if (data == nullptr) {
        string          reason(stbi__g_failure_reason);
        fgThrow("Unable to decode image",reason,fname.m_str);
    }
    StbiFree            sf(data);           // Can't use ScopeGuard since 'stbi_image_free' is extern C
    if (width*height <= 0)
        fgThrow("Invalid image dimensions",Vec2I(width,height));
    img = ImgRgba8{Vec2UI(width,height),reinterpret_cast<Rgba8*>(data)};
}

ImgRgba8            loadImage(String8 const & fname)
{
    ImgRgba8         ret;
    loadImage_(fname,ret);
    return ret;
}

static void         writeToFile(void *context,void * data,int size)
{
    Ofstream &          ofs = *reinterpret_cast<Ofstream*>(context);
    ofs.write(reinterpret_cast<char*>(data),size);
}

void                saveImage(String8 const & fname,ImgRgba8 const & img)
{
    if (img.numPixels() == 0)
        fgThrow("Cannot save empty image to file",fname);
    Path                path {fname};
    String8             ext = toLower(path.ext);
    if (ext.empty())
        fgThrow("No image file extension specified",fname);
    if (!contains(getImgExts(),ext.m_str))
        fgThrow("File extension is not a supported image output format",fname);
    uint                wid = img.width(),
                        hgt = img.height();
    uchar const *       data = &img.m_data[0][0];
    Ofstream            ofs {fname};
    int                 ret = 1;
    if ((ext == "jpg" || ext == "jpeg"))
        // Quality level 90 is high quality and visually comparable to the ImageMagick defaults previously used,
        // however STB encoding is about 25% larger:
        ret = stbi_write_jpg_to_func(writeToFile,&ofs,wid,hgt,4,data,90);
    else if (ext == "png")
        ret = stbi_write_png_to_func(writeToFile,&ofs,wid,hgt,4,data,wid*4);
    else if (ext == "bmp")
        ret = stbi_write_bmp_to_func(writeToFile,&ofs,wid,hgt,4,data);
    else if (ext == "tga")
        ret = stbi_write_tga_to_func(writeToFile,&ofs,wid,hgt,4,data);
    else
        FGASSERT_FALSE1(ext.m_str);
    if (ret == 0)
        fgThrow("STB image write error",fname);
}

void                saveJfif(ImgRgba8 const & img,String8 const & fname,uint quality)
{
    if (fname.empty())
        fgThrow("Cannot save image to empty filename");
    if (img.numPixels() == 0)
        fgThrow("Cannot save empty image to file",fname);
    if (quality > 100)
        fgThrow("Invalid JPEG quality value",quality);
    uint                wid = img.width(),
                        hgt = img.height();
    uchar const *       data = &img.m_data[0][0];
    Ofstream            ofs {fname};
    int                 ret;
    ret = stbi_write_jpg_to_func(writeToFile,&ofs,wid,hgt,4,data,int(quality));
    if (ret == 0)
        fgThrow("STB JFIF image write error",fname);
}

static void         writeToMem(void * context,void * data,int size)
{
    Uchars *            buffPtr = reinterpret_cast<Uchars*>(context);
    Uchars &            buff = *buffPtr;
    size_t              off = buff.size();
    buff.resize(off+size);
    memcpy(&buff[off],data,size);
};

Uchars              encodeJpeg(uint w,uint h,uchar const * data,int quality)
{
    int                 wid = w,
                        hgt = h;
    Uchars              ret;
    int                 stbRet = stbi_write_jpg_to_func(writeToMem,&ret,wid,hgt,4,data,quality);
    if (stbRet == 0)
        fgThrow("STB jpeg encoding error");
    return ret;
}

Uchars              encodeJpeg(ImgRgba8 const & img,int quality)
{
    return encodeJpeg(img.width(),img.height(),&img.m_data[0].m_c[0],quality);
}

ImgRgba8            decodeJpeg(Uchars const & jfifBlob)
{
    int                 width,height,channels;
    uchar *             data = nullptr;
    // require 4 channels:
    data = stbi_load_from_memory(&jfifBlob[0],scast<int>(jfifBlob.size()),&width,&height,&channels,4);
    if (data == nullptr) {
        string          reason(stbi__g_failure_reason);
        fgThrow("STB unable to decode JFIF blob",reason);
    }
    StbiFree            sf {data};
    FGASSERT((width > 0) && (height > 0));
    Vec2UI              dims {scast<uint>(width),scast<uint>(height)};
    return ImgRgba8 {dims,reinterpret_cast<Rgba8*>(data)};
}

String              zlibInflate(String const & compressed,size_t sz)
{
    String          ret (sz,' ');
    int             rc = stbi_zlib_decode_buffer(
        &ret[0],int(sz),compressed.data(),int(compressed.size()));
    FGASSERT(rc == int(sz));
    return ret;
}

void                testDecodeJfif(CLArgs const &)
{
    String8             pathBase = dataDir() + "base/Mandrill512";
    String              blob = loadRaw(pathBase+".jpg");
    Uchars              ub; ub.reserve(blob.size());
    for (char ch : blob)
        ub.push_back(scast<uchar>(ch));
    ImgRgba8            tst = decodeJpeg(ub),
                        ref = loadImage(pathBase+"-jpeg.png");  // baseline jpg encoding using STB on windows
    FGASSERT(isApproxEqual(tst,ref,3U));
}

}
