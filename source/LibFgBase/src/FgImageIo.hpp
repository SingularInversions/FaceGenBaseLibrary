//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     May 10, 2005
//
// Functions for reading and writing images to various file formats.
//

#ifndef FGIMAGEIO_HPP
#define FGIMAGEIO_HPP

#include "FgStdLibs.hpp"

#include "FgAlgs.hpp"
#include "FgImage.hpp"
#include "FgMatrixV.hpp"

void
fgLoadImgAnyFormat(const FgString & fname,FgImgRgbaUb & img);
void
fgLoadImgAnyFormat(const FgString & fname,FgImgF & img);
void
fgLoadImgAnyFormat(const FgString & fname,FgImgUC & img);
inline
FgImgRgbaUb
fgLoadImgAnyFormat(const FgString & fname)
{
    FgImgRgbaUb         ret;
    fgLoadImgAnyFormat(fname,ret);
    return ret;
}

void
fgSaveImgAnyFormat(const FgString & fname,const FgImgRgbaUb & img);

void
fgSaveImgAnyFormat(const FgString & fname,const FgImgUC & img);

// List of file extensions of the 6 most commonly used formats in LOWER CASE:
std::vector<std::string>
fgImgCommonFormats();

// Command-line options string of the formats above:
std::string
fgImgCommonFormatsDescription();

bool
fgIsImgFilename(const FgString & fname);

// Returns a list of image extensions for which there are readable files 'baseName.ext':
std::vector<std::string>
fgFindImgFiles(const FgString & baseName);

// Look for an image file in any common format starting with 'baseName' and load it if found.
// Return true if found and false otherwise.
bool
fgImgFindLoadAnyFormat(const FgString & baseName,FgImgRgbaUb & img);

// data must be 4 channel RGBA of size wid*hgt*4:
std::vector<uchar>
fgEncodeJpeg(uint wid,uint hgt,const uchar * data,int quality);

// JFIF format (can be dumped to .jpg file):
std::vector<uchar>
fgEncodeJpeg(const FgImgRgbaUb & img,int quality=100);

FgImgRgbaUb
fgDecodeJpeg(const std::vector<uchar> &  fileContents);

#endif
