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
fgLoadImgAnyFormat(
    const FgString &    fname,
    FgImgRgbaUb &       img);

inline
FgImgRgbaUb
fgLoadImgAnyFormat(const FgString & fname)
{
    FgImgRgbaUb         ret;
    fgLoadImgAnyFormat(fname,ret);
    return ret;
}

FgImg4UC
fgLoadImg4UC(const FgString & fname);

void
fgLoadImgAnyFormat(
    const FgString &    fname,
    FgImgF &            img);

void
fgSaveImgAnyFormat(
    const FgString &    fname,
    const FgImgRgbaUb & img);

// List of all supported image file format extensions in capitals:
std::vector<std::string>
fgImgSupportedFormats();

// List of file extensions of the 6 most commonly used formats:
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

void
fgImgSaveJfif(
    const FgImgRgbaUb &     img,
    const FgString &        fname,
    int                     quality=100);

void
fgImgSaveJfif(
    const FgImgRgbaUb &     img,
    std::vector<uchar> &    buffer,
    int                     quality=100);

void
fgImgLoadJfif(
    const FgString &        fname,
    FgImgRgbaUb &           img);

void
fgImgLoadJfif(
    const std::vector<uchar> &  fileContents,
    FgImgRgbaUb &           img);

#endif
