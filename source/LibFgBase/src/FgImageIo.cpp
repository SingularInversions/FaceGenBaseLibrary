//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
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
loadImage_(String8 const & fname,ImgUC & ret)
{
    ImgC4UC     img = loadImage(fname);
    ret.resize(img.dims());
    for (size_t ii=0; ii<ret.numPixels(); ++ii)
        ret[ii] = img[ii].rec709();
}

void
loadImage_(String8 const & fname,ImgF & img)
{
    ImgC4UC     tmp;
    loadImage_(fname,tmp);
    img.resize(tmp.dims());
    for (size_t ii=0; ii<tmp.numPixels(); ++ii)
        img.m_data[ii] = tmp.m_data[ii].rec709();
}

void
saveImage(String8 const & fname,const ImgUC & img)
{
    ImgC4UC         tmp;
    imgConvert_(img,tmp);
    saveImage(fname,tmp);
}

ImgFileFormats
imgFileFormats()
{
    static ImgFileFormats   ret = {
        {"PNG - RGBA, lossless compression low ratio",{"png"}},
        {"JPEG - RGB, lossy compression high ratio",{"jpg","jpeg"}},
        {"TARGA - RGBA, lossless compression very low ratio",{"tga"}},
        {"BMP - RGB Lossless compression medium ratio",{"bmp"}}
    };
    return ret;
}

Strings
imgFileExtensions()
{
    Strings         ret;
    for (ImgFileFormat const & iff : imgFileFormats())
        cat_(ret,iff.extensions);
    return ret;
}

string
imgFileExtensionsDescription()
{
    Strings  cf = imgFileExtensions();
    string  retval("(");
    retval += cf[0];
    for (size_t ii=1; ii<cf.size(); ++ii)
        retval += string(" | ") + cf[ii];
    return (retval + ")");
}

bool
hasImgExtension(String8 const & fname)
{
    string          ext = toLower(pathToExt(fname).m_str);
    return contains(imgFileExtensions(),ext);
}

std::vector<std::string>
imgFindFiles(String8 const & baseName)
{
    Strings      ret,
                        cifs = imgFileExtensions();
    for (size_t ii=0; ii<cifs.size(); ++ii)
        if (fileReadable(baseName+"."+cifs[ii]))
            ret.push_back(cifs[ii]);
    return ret;
}

bool
imgFindLoadAnyFormat(String8 const & baseName,ImgC4UC & img)
{
    Strings  exts = imgFindFiles(baseName);
    if (exts.empty())
        return false;
    String8        fname = baseName + "." + exts[0];
    if (exts.size() > 1)
        fgout << fgnl << "WARNING: Selecting first of possible image files: " << fname;
    loadImage_(fname,img);
    return true;
}

void
fgImgTestWrite(CLArgs const & args)
{
    FGTESTDIR
    char32_t        ch = 0x00004EE5;            // A Chinese character
    String8        chinese(ch);
    ImgC4UC     redImg(16,16,RgbaUC(255,0,0,255));
    saveImage(chinese+"0.jpg",redImg);
    saveImage(chinese+"0.png",redImg);
}

}
