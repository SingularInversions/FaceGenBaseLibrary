//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Sohail Somani
//

#include "stdafx.h"
#include "FgImageIo.hpp"
#include "FgFileSystem.hpp"
#include "FgScopeGuard.hpp"
#include "FgCommand.hpp"

using namespace std;

void
fgLoadImgAnyFormat(const FgString & fname,FgImgUC & ret)
{
    FgImgRgbaUb     img = fgLoadImgAnyFormat(fname);
    ret.resize(img.dims());
    for (size_t ii=0; ii<ret.numPixels(); ++ii)
        ret[ii] = img[ii].rec709();
}

void
fgLoadImgAnyFormat(const FgString & fname,FgImgF & img)
{
    FgImgRgbaUb     tmp;
    fgLoadImgAnyFormat(fname,tmp);
    img.resize(tmp.dims());
    for (size_t ii=0; ii<tmp.numPixels(); ++ii)
        img.m_data[ii] = tmp.m_data[ii].rec709();
}

void
fgSaveImgAnyFormat(const FgString & fname,const FgImgUC & img)
{
    FgImgRgbaUb         tmp;
    fgImgConvert(img,tmp);
    fgSaveImgAnyFormat(fname,tmp);
}

vector<string>
fgImgCommonFormats()
{
    return fgSvec<string>("png","jpg","jpeg","bmp","tga");
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
    string          ext = fgToLower(fgPathToExt(fname).m_str);
    return fgContains(fgImgCommonFormats(),ext);
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
    char32_t        ch = 0x00004EE5;            // A Chinese character
    FgString        chinese(ch);
    FgImgRgbaUb     redImg(16,16,FgRgbaUB(255,0,0,255));
    fgSaveImgAnyFormat(chinese+"0.jpg",redImg);
    fgSaveImgAnyFormat(chinese+"0.png",redImg);
}
