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

using namespace std;

namespace Fg {

namespace {

void
addalpha(CLArgs const & args)
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
formats(CLArgs const &)
{
    Strings             fs = getImageFileExts();
    fgout << fgnl << fs.size() << " formats supported:" << fgpush;
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

void
cmdImgops(CLArgs const & args)
{
    Cmds            cmds {
        {addalpha,"addalpha","Add/replace an alpha channel from an another image"},
        {cmdComposite,"composite","Composite an image with transparency over another"},
        {cmdConst,"const","Create a constant-valued image"},
        {cmdConvert,"convert","Convert images between different formats"},
        {formats,"formats","List all supported formats by file extension"},
        {cmdShrink,"shrink2","Shrink images by a factor of 2"},
    };
    doMenu(args,cmds);
}

}

Cmd
getImgopsCmd()
{return Cmd(cmdImgops,"image","Image operations"); }

}

// */
