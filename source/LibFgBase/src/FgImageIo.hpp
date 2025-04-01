//
// Copyright (c) 2025 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Functions for reading and writing images to various file formats.
//

#ifndef FGIMAGEIO_HPP
#define FGIMAGEIO_HPP

#include "FgImage.hpp"
#include "FgFile.hpp"

namespace Fg {

// read/write image file formats, thanks to Sean Barrett. First is default for writing.
enum struct     ImgFormat {png, jpg, tga, bmp};

typedef Svec<ImgFormat>     ImgFormats;

struct          ImgFormatInfo
{
    ImgFormat       format;
    Strings         extensions;             // lower case, first is most common if more than one
    String          description;

    bool            operator==(ImgFormat rhs) const {return (format==rhs); }
};
typedef Svec<ImgFormatInfo>     ImgFormatInfos;

ImgFormatInfos const &  getImgFormatsInfo();    // first is default for writing
inline ImgFormatInfo    getImgFormatInfo(ImgFormat f) {return findFirst(getImgFormatsInfo(),f); }
// List of supported image file format extensions in lower case, including synonyms (all formats can be read & written):
Strings const &         getImgExts();
// true if the file extension is a read/write supported image file format:
bool                    hasImgFileExt(String8 const & fname);
// Load an image from any supported format based on filename extension.
// If the image file contains alpha values note that color channels will NOT be premultiplied (explicit for PNG).
// If the image file does not contain alpha values they will all be set to 255:
void                    loadImage_(String8 const & fname,ImgRgba8 & img);
ImgRgba8                loadImage(String8 const & fname);
// Save image to any supported format:
// WARNING: some formats do not support alpha, and some (PNG) will zero all color channels if alpha is zero:
void                    saveImage(ImgRgba8 const & img,String8 const & fname);
// load / save image landmarks to a simple text format with 1 point per line of the form: <name> <X> <Y>
// where <X> and <Y> are in raster coordinates (origin at center of top left pixel) and can be signed
// floating point values. (aka "YOLO" format).
// These should be stored in the file <imgBase>.lms.txt, where the image is in <imgBase>.<ext>
NameVec2Fs              loadLandmarks(String8 const & fname);
void                    saveLandmarks(NameVec2Fs const & lmsIrcs,String8 const & fname);
// swaps the extension and loads landmarks from <basename>.lms.txt (throw if that file doesn't exist):
inline NameVec2Fs       loadImageLandmarks(String8 const & imgFname) {return loadLandmarks(pathToDirBase(imgFname)+".lms.txt"); }

//  JPEG-specific:
void                    saveJfif(
    ImgRgba8 const &    img,           // Alpha channel will be ignored
    String8 const &     fname,
    // Quality level 90 is high quality and visually comparable to the ImageMagick defaults previously used,
    // however STB encoding is about 25% larger:
    uint                quality=90);    // [1,100] where 100 saves with lossless compression
// Encode to JFIF format blob (can be dumped to .jpg file), data must be 4 channel RGBA of size wid*hgt*4:
Bytes                  encodeJpeg(uint wid,uint hgt,uchar const * data,int quality=90);
Bytes                  encodeJpeg(ImgRgba8 const & img,int quality=90);
// Decode from JFIF format blob (ie. JFIF format .jpg contents)
ImgRgba8                decodeJpeg(Bytes const & jfifBlob);

}

#endif
