//
// Coypright (c) 2022 Singular Inversions Inc. (facegen.com)
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

FileFormats         getImageFileFormats()
{
    static FileFormats   ret = {
        {"PNG - RGBA, lossless compression low ratio",{"png"}},
        {"JPEG - RGB, lossy compression high ratio",{"jpg","jpeg"}},
        {"TARGA - RGBA, lossless compression very low ratio",{"tga"}},
        {"BMP - RGB Lossless compression medium ratio",{"bmp"}}
    };
    return ret;
}
Strings             getImageFileExts()
{
    Strings         ret;
    for (FileFormat const & iff : getImageFileFormats())
        cat_(ret,iff.extensions);
    return ret;
}
string              getImageFileExtCLDescriptions()
{
    Strings  cf = getImageFileExts();
    string  retval("(");
    retval += cf[0];
    for (size_t ii=1; ii<cf.size(); ++ii)
        retval += string(" | ") + cf[ii];
    return (retval + ")");
}
bool                hasImageFileExt(String8 const & fname)
{
    string              ext = toLower(pathToExt(fname).m_str);
    return contains(getImageFileExts(),ext);
}
Strings             getImageFiles(String8 const & baseName)
{
    Strings             ret,
                        exts = getImageFileExts();
    for (size_t ii=0; ii<exts.size(); ++ii)
        if (fileReadable(baseName+"."+exts[ii]))
            ret.push_back(exts[ii]);
    return ret;
}
bool                loadImageAnyFormat_(String8 const & baseName,ImgRgba8 & img)
{
    Strings             exts = getImageFiles(baseName);
    if (exts.empty())
        return false;
    String8             fname = baseName + "." + exts[0];
    loadImage_(fname,img);
    return true;
}
ImgRgba8            loadImageAnyFormat(String8 const & baseName)
{
    ImgRgba8            ret;
    if (!loadImageAnyFormat_(baseName,ret))
        fgThrow("Unable to find image file with base name",baseName);
    return ret;
}
NameVec2Fs          loadImageLandmarks(String8 const & fname)
{
    Strings             lines = splitLines(loadRaw(fname),'#');     // removes empty lines
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
void                saveImageLandmarks(NameVec2Fs const & lmsIrcs,String8 const & fname)
{
    Ofstream            ofs {fname};
    ofs
        << "# lines of the form: <name> <X> <Y>\n"
        << "# <X> and <Y> are in raster coordinates (origin at center of top left pixel)\n"
        << "# <X> and <Y> can be signed floating point values";
    for (NameVec2F const & nv : lmsIrcs)
        ofs << "\n" << nv.name << " " << nv.vec[0] << " " << nv.vec[1];
}

void                fgImgTestWrite(CLArgs const & args)
{
    FGTESTDIR
    char32_t            ch = 0x00004EE5;            // A Chinese character
    String8             chinese(ch);
    ImgRgba8            redImg(16,16,Rgba8(255,0,0,255));
    saveImage(chinese+"0.jpg",redImg);
    saveImage(chinese+"0.png",redImg);
}

}
