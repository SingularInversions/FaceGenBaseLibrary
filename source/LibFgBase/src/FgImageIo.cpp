//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//


#include "stdafx.h"
#include "FgImageIo.hpp"
#include "FgFileSystem.hpp"
#include "FgScopeGuard.hpp"
#include "FgParse.hpp"
#include "FgCommand.hpp"

using namespace std;

namespace Fg {

ImgFormatsInfo const &  getImgFormatsInfo()     // first is default for writing
{
    static ImgFormatsInfo ret {
        {ImgFormat::png,{"png"},        "PNG: lossless, medium file size"},
        {ImgFormat::jpg,{"jpg","jpeg"}, "JPG: good quality, small file size, no transparency"},
        {ImgFormat::tga,{"tga"},        "TGA: lossless, large file size"},
        {ImgFormat::bmp,{"bmp"},        "BMP: Lossless, medium file size, no transparency"},
    };
    return ret;
}

Strings             getImgExts()
{
    Strings         ret;
    for (ImgFormatInfo const & ifi : getImgFormatsInfo())
        cat_(ret,ifi.extensions);
    return ret;
}

bool                hasImgFileExt(String8 const & fname)
{
    string              ext = toLower(pathToExt(fname).m_str);
    return contains(getImgExts(),ext);
}

NameVec2Fs          loadLandmarks(String8 const & fname)
{
    Strings             lines = splitLines(loadRawString(fname),'#');     // removes empty lines
    NameVec2Fs          ret;
    for (String const & line : lines) {
        Strings             objs = splitWhitespace(line);
        if (objs.size() != 3) {
            fgout << fgnl << "WARNING invalid line ignored: " << line;
            continue;
        }
        NameVec2F           nv;
        nv.name = objs[0];
        bool                coordsValid = true;
        for (size_t ii=0; ii<2; ++ii) {
            Opt<float>      vo = fromStr<float>(objs[ii+1]);
            if (!vo.valid()) {
                fgout << fgnl << "WARNING invalid number for point " << nv.name << ": " << objs[ii+1];
                coordsValid = false;
            }
            else
                nv.vec[ii] = vo.val();
        }
        if (coordsValid)
            ret.push_back(nv);
    }
    return ret;
}

void                saveLandmarks(NameVec2Fs const & lmsIrcs,String8 const & fname)
{
    Ofstream            ofs {fname};
    ofs
        << "# lines of the form: <name> <X> <Y>\n"
        << "# <X> and <Y> are in raster coordinates (origin at center of top left pixel)\n"
        << "# <X> and <Y> can be signed floating point values";
    for (NameVec2F const & nv : lmsIrcs)
        ofs << "\n" << nv.name << " " << nv.vec[0] << " " << nv.vec[1];
}

}
