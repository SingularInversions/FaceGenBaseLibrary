//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//


#include "stdafx.h"
#include "FgImageIo.hpp"
#include "FgFileSystem.hpp"
#include "FgScopeGuard.hpp"
#include "FgCommand.hpp"

using namespace std;

namespace Fg {

void
imgLoadAnyFormat(Ustring const & fname,ImgUC & ret)
{
    ImgC4UC     img = imgLoadAnyFormat(fname);
    ret.resize(img.dims());
    for (size_t ii=0; ii<ret.numPixels(); ++ii)
        ret[ii] = img[ii].rec709();
}

void
imgLoadAnyFormat(Ustring const & fname,ImgF & img)
{
    ImgC4UC     tmp;
    imgLoadAnyFormat(fname,tmp);
    img.resize(tmp.dims());
    for (size_t ii=0; ii<tmp.numPixels(); ++ii)
        img.m_data[ii] = tmp.m_data[ii].rec709();
}

void
imgSaveAnyFormat(Ustring const & fname,const ImgUC & img)
{
    ImgC4UC         tmp;
    imgConvert_(img,tmp);
    imgSaveAnyFormat(fname,tmp);
}

vector<string>
imgFileExtensions()
{
    return fgSvec<string>("png","jpg","jpeg","bmp","tga");
}

string
imgFileExtensionsDescription()
{
    vector<string>  cf = imgFileExtensions();
    string  retval("(");
    retval += cf[0];
    for (size_t ii=1; ii<cf.size(); ++ii)
        retval += string(" | ") + cf[ii];
    return (retval + ")");
}

bool
hasImgExtension(Ustring const & fname)
{
    string          ext = fgToLower(fgPathToExt(fname).m_str);
    return fgContains(imgFileExtensions(),ext);
}

std::vector<std::string>
imgFindFiles(Ustring const & baseName)
{
    vector<string>      ret,
                        cifs = imgFileExtensions();
    for (size_t ii=0; ii<cifs.size(); ++ii)
        if (fileReadable(baseName+"."+cifs[ii]))
            ret.push_back(cifs[ii]);
    return ret;
}

bool
imgFindLoadAnyFormat(Ustring const & baseName,ImgC4UC & img)
{
    vector<string>  exts = imgFindFiles(baseName);
    if (exts.empty())
        return false;
    Ustring        fname = baseName + "." + exts[0];
    if (exts.size() > 1)
        fgout << fgnl << "WARNING: Selecting first of possible image files: " << fname;
    imgLoadAnyFormat(fname,img);
    return true;
}

void
fgImgTestWrite(CLArgs const & args)
{
    FGTESTDIR
    char32_t        ch = 0x00004EE5;            // A Chinese character
    Ustring        chinese(ch);
    ImgC4UC     redImg(16,16,RgbaUC(255,0,0,255));
    imgSaveAnyFormat(chinese+"0.jpg",redImg);
    imgSaveAnyFormat(chinese+"0.png",redImg);
}

}
