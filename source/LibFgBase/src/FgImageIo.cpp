//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Sohail Somani
//

#include "stdafx.h"
#include "FgImageIo.hpp"
#include "FgThread.hpp"
#include "FgFileSystem.hpp"
#include "FgScopeGuard.hpp"
#include "FgSmartPtr.hpp"
#include "FgCommand.hpp"

// Don't let ImageMagick redeclare malloc:
#define HAVE_STDLIB_H
#include "magick/ImageMagick.h"

using namespace std;

static FgOnce fg_magick_init = FG_ONCE_INIT;

static
void
fgEnsureMagick()
{
    struct Helper
    {
        static void
        init()
        {
            MagickCoreGenesis(0,MagickTrue);
        }
    };
    fgRunOnce(fg_magick_init,Helper::init);
}

void
fgLoadImgAnyFormat(
    const FgString &    fname,
    FgImgRgbaUb &       img)
{
    if (!fgFileReadable(fname))
        fgThrow("Unable to read file",fname);
    fgEnsureMagick();
    FgScopePtr<ExceptionInfo>   exception(AcquireExceptionInfo(),DestroyExceptionInfo);
    FgScopePtr<ImageInfo>       image_info(CloneImageInfo(0),DestroyImageInfo);
    (void) strcpy(image_info->filename,fname.as_utf8_string().c_str());
    // TODO: Do we need to convert into a specific colourspace?
    Image *imgPtr = ReadImage(image_info.get(),exception.get());
    if (imgPtr == 0)
        // exception.description is NULL. exception->reason includes the filename:
        fgThrow("Unable to read image file",exception->reason);
    FgScopeGuard                sg0(boost::bind(DestroyImage,imgPtr));
    FgScopePtr<CacheView>       view(AcquireCacheView(imgPtr),DestroyCacheView);
    img.resize(uint(imgPtr->columns),uint(imgPtr->rows));
    for(uint row = 0; row < imgPtr->rows; ++row) {
        for(uint column = 0; column < imgPtr->columns; ++column) {
            PixelPacket pixel;
            FGASSERT(MagickTrue == 
                    GetOneCacheViewAuthenticPixel(view.get(),column,row,&pixel,exception.get()));
            img.xy(column,row).red() = uchar(pixel.red);
            img.xy(column,row).green() = uchar(pixel.green);
            img.xy(column,row).blue() = uchar(pixel.blue);
            img.xy(column,row).alpha() =  255 - uchar(pixel.opacity);
        }
    }
}
void
fgLoadImgAnyFormat(const FgString & fname,FgImgUC & ret)
{
    FgImgRgbaUb     img = fgLoadImgAnyFormat(fname);
    ret.resize(img.dims());
    for (size_t ii=0; ii<ret.numPixels(); ++ii)
        ret[ii] = img[ii].rec709();
}

FgImg4UC
fgLoadImg4UC(const FgString & fname)
{
    FgImg4UC        ret;
    if (!fgFileReadable(fname))
        fgThrow("Unable to read file",fname);
    fgEnsureMagick();
    FgScopePtr<ExceptionInfo>   exception(AcquireExceptionInfo(),DestroyExceptionInfo);
    FgScopePtr<ImageInfo>       image_info(CloneImageInfo(0),DestroyImageInfo);
    (void) strcpy(image_info->filename,fname.as_utf8_string().c_str());
    // TODO: Do we need to convert into a specific colourspace?
    Image *imgPtr = ReadImage(image_info.get(),exception.get());
    if (imgPtr == 0)
        // exception.description is NULL. exception->reason includes the filename:
        fgThrow("Unable to read image file",exception->reason);
    FgScopeGuard                sg0(boost::bind(DestroyImage,imgPtr));
    FgScopePtr<CacheView>       view(AcquireCacheView(imgPtr),DestroyCacheView);
    ret.resize(uint(imgPtr->columns),uint(imgPtr->rows));
    for(uint row= 0; row < imgPtr->rows; ++row) {
        for(uint column = 0; column < imgPtr->columns; ++column) {
            PixelPacket pixel;
            FGASSERT(MagickTrue == 
                    GetOneCacheViewAuthenticPixel(view.get(),column,row,&pixel,exception.get()));
            ret.xy(column,row)[0] = uchar(pixel.red);
            ret.xy(column,row)[1] = uchar(pixel.green);
            ret.xy(column,row)[2] = uchar(pixel.blue);
            ret.xy(column,row)[3] =  255 - uchar(pixel.opacity);
        }
    }
    return ret;
}

void
fgLoadImgAnyFormat(
    const FgString &    fname,
    FgImgF &            img)
{
    if (!fgFileReadable(fname))
        fgThrow("Unable to read file",fname);
    fgEnsureMagick();
    FgScopePtr<ExceptionInfo>   exception(AcquireExceptionInfo(),DestroyExceptionInfo);
    FgScopePtr<ImageInfo>       image_info(CloneImageInfo(0),DestroyImageInfo);
    (void) strcpy(image_info->filename,fname.as_utf8_string().c_str());
    // TODO: Do we need to convert into a specific colourspace?
    Image *imgPtr = ReadImage(image_info.get(),exception.get());
    if (imgPtr == 0)
        // exception.description is NULL. exception->reason includes the filename:
        fgThrow("Unable to read image file",exception->reason);
    FgScopeGuard                sg0(boost::bind(DestroyImage,imgPtr));
    FgScopePtr<CacheView>       view(AcquireCacheView(imgPtr),DestroyCacheView);
    img.resize(uint(imgPtr->columns),uint(imgPtr->rows));
    for(uint row = 0; row < imgPtr->rows; ++row) {
        for(uint column = 0; column < imgPtr->columns; ++column) {
            PixelPacket pixel;
            FGASSERT(MagickTrue == 
                    GetOneCacheViewAuthenticPixel(view.get(),column,row,&pixel,exception.get()));
            img.xy(column,row) = pixel.red;   // All channels same value if single-channel read
        }
    }
}

void
fgSaveImgAnyFormat(const FgString & fname,const FgImgRgbaUb & img)
{
    FGASSERT(fname.length() > 0);
    fgEnsureMagick();
    FgScopePtr<ImageInfo>       image_info(CloneImageInfo(0),DestroyImageInfo);
    FgScopePtr<ExceptionInfo>   exception(AcquireExceptionInfo(),DestroyExceptionInfo);
    FgScopePtr<Image>           image(ConstituteImage(img.width(),img.height(),"RGBA",
                                                   CharPixel,
                                                   img.dataPtr(),
                                                   exception.get()),
                                   DestroyImage);
    (void) strcpy(image_info->filename,fname.as_utf8_string().c_str());
    MagickBooleanType   res = WriteImages(image_info.get(),image.get(),fname.as_utf8_string().c_str(),exception.get());
    if (res != MagickTrue)
        // Filename already included in 'exception->reason':
        fgThrow("Unable to save image to file",exception->reason);
}

void
fgSaveImgAnyFormat(const FgString & fname,const FgImgUC & img)
{
    FgImgRgbaUb         tmp;
    fgImgConvert(img,tmp);
    fgSaveImgAnyFormat(fname,tmp);
}

vector<string>
fgImgSupportedFormats()
{
    vector<string>              ret;
    fgEnsureMagick();
    FgScopePtr<ExceptionInfo>   exception(AcquireExceptionInfo(),DestroyExceptionInfo);
    size_t                      number_formats = 0;
    FgScopePtr<char*>           formats(GetMagickList("*",&number_formats,exception.get()),RelinquishMagickMemory);
    for(size_t ii=0; ii<number_formats; ++ii)
        ret.push_back(string(*(formats.get()+ii)));
    ret.push_back("TIF");       // TIFF is supported but this abbreviation is not in the list
    return ret;
}

vector<string>
fgImgCommonFormats()
{
    return
        fgSvec(
            string("png"),string("jpg"),string("bmp"),
            string("gif"),string("tif"),string("tga"));
}

string
fgImgCommonFormatsDescription()
{
    vector<string>  cf = fgImgCommonFormats();
    string  retval("(");
    retval += cf[0];
    for (size_t ii=1; ii<cf.size(); ++ii)
        retval += string(" | ") + cf[ii];
    return (retval + ")");
}

bool
fgIsImgFilename(const FgString & fname)
{
    string          ext = fgToUpper(fgPathToExt(fname).m_str);
    vector<string>  lst = fgImgSupportedFormats();
    return (std::find(lst.begin(),lst.end(),ext) != lst.end());
}

std::vector<std::string>
fgFindImgFiles(const FgString & baseName)
{
    vector<string>      ret,
                        cifs = fgImgCommonFormats();
    for (size_t ii=0; ii<cifs.size(); ++ii)
        if (fgFileReadable(baseName+"."+cifs[ii]))
            ret.push_back(cifs[ii]);
    return ret;
}

bool
fgImgFindLoadAnyFormat(const FgString & baseName,FgImgRgbaUb & img)
{
    vector<string>  exts = fgFindImgFiles(baseName);
    if (exts.empty())
        return false;
    FgString        fname = baseName + "." + exts[0];
    if (exts.size() > 1)
        fgout << fgnl << "WARNING: Selecting first of possible image files: " << fname;
    fgLoadImgAnyFormat(fname,img);
    return true;
}

void
fgImgTestWrite(const FgArgs & args)
{
    FGTESTDIR
    wstring         chin = L"\u4EE5";       // Chinese symbol in unicode
    FgString        chinese(chin);
    FgImgRgbaUb     redImg(16,16,FgRgbaUB(255,0,0,255));
    fgSaveImgAnyFormat(chinese+"0.jpg",redImg);
    fgSaveImgAnyFormat(chinese+"0.png",redImg);
}
