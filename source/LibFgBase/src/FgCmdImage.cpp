//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgCommand.hpp"
#include "FgSyntax.hpp"
#include "FgMetaFormat.hpp"
#include "FgImageIo.hpp"
#include "FgParse.hpp"
#include "FgImgDisplay.hpp"

using namespace std;

namespace Fg {

namespace {

void
cmdAlpha(CLArgs const & args)
{
    Syntax              syn(args,
        "<rgb>.<ext> <alpha>.<ext> <out>.<ext>\n"
        "    <ext>      - " + getImageFileExtCLDescriptions() + "\n"
        "    <rgb>      - Any existing alpha channel will be ignored\n"
        "    <alpha>    - If this is not a single-channel image, a greyscale mapping will be applied"
        );
    ImgRgba8            rgb = loadImage(syn.next()),
                        alpha = loadImage(syn.next());
    if (rgb.dims() != alpha.dims())
        fgThrow("<rgb> and <alpha> images have different pixel dimensions");
    for (size_t ii=0; ii<rgb.m_data.size(); ++ii)
        rgb.m_data[ii].alpha() = alpha.m_data[ii].rec709();
    saveImage(syn.next(),rgb);
}

void
cmdComposite(CLArgs const & args)
{
    Syntax              syn(args,
        "<base>.<imgExt> <overlay>.<imgExt> <output>.<imgExt>\n"
        "    Composite images of equal pixel dimensions.\n"
        "    <overlay>.<imgExt> must have an alpha channel\n"
        "    <imgExt> = " + getImageFileExtCLDescriptions());
    ImgRgba8            base = loadImage(syn.next()),
                        overlay = loadImage(syn.next());
    if (base.dims() != overlay.dims())
        syn.error("The images must have identical pixel dimensions");
    saveImage(syn.next(),composite(overlay,base));
}

void
cmdConst(CLArgs const & args)
{
    Syntax              syn {args,
        R"(<X size> <Y size> <R> <G> <B> <A> <out>.<ext>
    <X,Y size>      - in pixels
    <R,G,B>         - [0,255]
    <ext>           - )" + getImageFileExtCLDescriptions()
    };
    uint                X = syn.nextAs<uint>(),
                        Y = syn.nextAs<uint>();
    Rgba8              rgba;
    for (uint ii=0; ii<4; ++ii) {
        uint                v = syn.nextAs<uint>();
        if (v > 255)
            syn.error("RGBA values must be <= 255",toStr(v));
        rgba[ii] = scast<uchar>(v);
    }
    saveImage(syn.next(),ImgRgba8{X,Y,rgba});
}

void
cmdConvert(CLArgs const & args)
{
    Syntax              syn(args,
        "<in>.<ext> <out>.<ext>\n"
        "    <ext>      - " + getImageFileExtCLDescriptions()
        );
    ImgRgba8            img = loadImage(syn.next());
    saveImage(syn.next(),img);
}

void            cmdCreate(CLArgs const & args)
{
    Syntax              syn {args,
        R"(<num> <size> <out>.<ext>
    <num>       - number of checkerboard squares across
    <size>      - size of each square in pixels
    <ext>       - )" + getImageFileExtCLDescriptions() + R"(
OUTPUT:
    <img>.<ext> - generated image saved here
NOTES:
    Creates a black and white checkerboard image)"
    };
    size_t              sz = syn.nextAs<size_t>(),
                        num = syn.nextAs<size_t>();
    ImgRgba8            img {Vec2UI{uint(sz*num)},Rgba8{0,0,0,255}};
    for (size_t ny=0; ny<num; ++ny) {
        for (size_t nx=0; nx<num; ++nx) {
            uchar           col = scast<uchar>((scast<uint>(ny+nx) & 0x1U) * 255U);
            Rgba8           rgba {col,col,col,255};
            for (size_t sy=0; sy<sz; ++sy)
                for (size_t sx=0; sx<sz; ++sx)
                    img.xy(nx*sz+sx,ny*sz+sy) = rgba;
            col = ~col;
        }
    }
    saveImage(syn.next(),img);
}

void
cmdShrink(CLArgs const & args)
{
    Syntax              syn(args,
        "<in>.<ext> <out>.<ext>\n"
        "    <ext>      - " + getImageFileExtCLDescriptions()
        );
    ImgRgba8            img = loadImage(syn.next());
    img = shrink2(img);
    saveImage(syn.next(),img);
}

void
cmdFormats(CLArgs const &)
{
    Strings             fs = getImageFileExts();
    fgout << fgnl << fs.size() << " cmdFormats supported:" << fgpush;
    char                previous = 'Z';
    for (string const & f : fs) {
        if (f[0] != previous) {
            previous = f[0];
            fgout << fgnl;
        }
        fgout << f << ", ";
    }
    fgout << fgpop;
}

void        cmdMark(CLArgs const & args)
{
    Syntax              syn {args,
        R"([-b] <landmarks> <images>
    -b              - brighten the image with gamma correction for easier viewing of dark areas
    <landmarks>     - ( <list>.txt | (<landmarkName>)+ )
    <images>        - ( <list>.txt | (<fileName>.<ext>)+ )
    <list>.txt      - must contain a whitespace-separated list
    <landmarkName>  - cannot contain '.' character
    <ext>           - )" + getImageFileExtCLDescriptions()
    };
    bool                brighten = false;
    if (beginsWith(syn.peekNext(),"-")) {
        if (syn.next() == "-b")
            brighten = true;
        else
            syn.error("Unknown flag",syn.curr());
    }
    Strings             lmNames;
    Strings             imgFiles;
    if (endsWith(syn.peekNext(),".txt"))
        lmNames = splitWhitespace(loadRaw(syn.next()));
    else {
        do
            lmNames.push_back(syn.next());
        while (!contains(syn.peekNext(),'.'));
    }
    if (endsWith(syn.peekNext(),".txt"))
        imgFiles = splitWhitespace(loadRaw(syn.next()));
    else {
        do
            imgFiles.push_back(syn.next());
        while (syn.more());
    }
    for (String const & imgFile : imgFiles) {
        ImgRgba8            img = loadImage(imgFile);
        if (brighten)
            for (Rgba8 & rgba : img.m_data)
                for (uint cc=0; cc<3; ++cc)
                    rgba[cc] = scast<uchar>(pow(rgba[cc]/255.0f,1/2.5f)*255.0f+0.5f);
        ImagePoints         lms;
        String8             lmsFile = pathToDirBase(imgFile)+".lms.txt";
        if (fileExists(lmsFile))
            lms = loadImagePoints(lmsFile);
        ImagePoints         lmsNew = markImage(img,lms,lmNames);
        if (lmsNew == lms)
            fgout << fgnl << "No landmarks placed, nothing saved";
        else {
            saveImagePoints(lmsNew,lmsFile);
            fgout << fgnl << lmsNew.size() << " landmarks saved in " << lmsFile;
        }
    }
}

void
cmdImgops(CLArgs const & args)
{
    Cmds            cmds {
        {cmdAlpha,"alpha","Add/replace an alpha channel from an another image"},
        {cmdComposite,"composite","Composite an image with transparency over another"},
        {cmdConst,"const","Create a constant-valued image"},
        {cmdConvert,"convert","Convert images between different cmdFormats"},
        {cmdCreate,"create","create checkerboard images"},
        {cmdFormats,"formats","List all supported formats by file extension"},
        {cmdMark,"mark","Place landmark points on an image"},
        {cmdShrink,"shrink2","Shrink images by a factor of 2"},
    };
    doMenu(args,cmds);
}

}

Cmd
getCmdImage()
{return Cmd(cmdImgops,"image","Image operations"); }

}

// */
