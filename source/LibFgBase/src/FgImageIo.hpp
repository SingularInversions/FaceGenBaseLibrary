//
// Coypright (c) 2022 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Functions for reading and writing images to various file formats.
//

#ifndef FGIMAGEIO_HPP
#define FGIMAGEIO_HPP

#include "FgStdExtensions.hpp"
#include "FgImage.hpp"
#include "FgMatrixV.hpp"

namespace Fg {

// Load an image from any supported format based on filename extension:
void            loadImage_(String8 const & fname,ImgRgba8 & img);
ImgRgba8        loadImage(String8 const & fname);
// Save image to any supported format:
void            saveImage(String8 const & fname,ImgRgba8 const & img);

struct  FileFormat
{
    String      description;
    Strings     extensions;     // Usually 1 but can be 2 (eg. 'jpg', 'jpeg'). Preferred listed first.
};
typedef Svec<FileFormat>     FileFormats;

// List of supported image file formats in order of desired use in upper case:
FileFormats     getImageFileFormats();
// List of supported image file format extensions in lower case, including synonyms:
Strings         getImageFileExts();
// Command-line options string of the formats above:
String          getImageFileExtCLDescriptions();
bool            hasImageFileExt(String8 const & fname);
// Returns a list of image extensions for which there are readable files 'baseName.ext':
Strings         getImageFiles(String8 const & baseName);
// Look for an image file in any common format starting with 'baseName' and load it if found.
// Return true if found and false otherwise.
bool            loadImageAnyFormat_(String8 const & baseName,ImgRgba8 & img);
ImgRgba8        loadImageAnyFormat(String8 const & baseName);           // Throws if not found
void            saveJfif(
    ImgRgba8 const &  img,           // Alpha channel will be ignored
    String8 const & fname,
    // Quality level 90 is high quality and visually comparable to the ImageMagick defaults previously used,
    // however STB encoding is about 25% larger:
    uint            quality=90);    // [1,100] where 100 saves with lossless compression
// Encode to JFIF format blob (can be dumped to .jpg file):
Uchars          encodeJpeg(ImgRgba8 const & img,int quality=100);
// As above. Data must be 4 channel RGBA of size wid*hgt*4:
Uchars          encodeJpeg(uint wid,uint hgt,uchar const * data,int quality);
// Decode from JFIF format blob (can be read from JFIF format .jpg file):
ImgRgba8        decodeJpeg(Uchars const & jfifBlob);
// load / save image landmarks to simple text format with 1 point per line of the form: <name> <X> <Y>
// where <X> and <Y> are in raster coordinates (origin at center of top left pixel)
// and can be signed floating point values:
NameVec2Fs      loadImageLandmarks(String8 const & fname);
void            saveImageLandmarks(NameVec2Fs const & lmsIrcs,String8 const & fname);

}

#endif
