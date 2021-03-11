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
    Syntax          syntax(args,
        "<rgb>.<ext> <alpha>.<ext> <out>.<ext>\n"
        "    <ext>      - " + imgFileExtensionsDescription() + "\n"
        "    <rgb>      - Any existing alpha channel will be ignored\n"
        "    <alpha>    - If this is not a single-channel image, a greyscale mapping will be applied"
        );
    ImgC4UC         rgb = loadImage(syntax.next()),
                    alpha = loadImage(syntax.next());
    if (rgb.dims() != alpha.dims())
        fgThrow("<rgb> and <alpha> images have different pixel dimensions");
    for (size_t ii=0; ii<rgb.m_data.size(); ++ii)
        rgb.m_data[ii].alpha() = alpha.m_data[ii].rec709();
    saveImage(syntax.next(),rgb);
}

void
cmdComposite(CLArgs const & args)
{
    Syntax          syn(args,
        "<base>.<imgExt> <overlay>.<imgExt> <output>.<imgExt>\n"
        "    Composite images of equal pixel dimensions.\n"
        "    <overlay>.<imgExt> must have an alpha channel\n"
        "    <imgExt> = " + imgFileExtensionsDescription());
    ImgC4UC         base = loadImage(syn.next()),
                    overlay = loadImage(syn.next());
    if (base.dims() != overlay.dims())
        syn.error("The images must have identical pixel dimensions");
    saveImage(syn.next(),composite(overlay,base));
}

void
convert(CLArgs const & args)
{
    Syntax    syntax(args,
        "<in>.<ext> <out>.<ext>\n"
        "    <ext>      - " + imgFileExtensionsDescription()
        );
    ImgC4UC     img = loadImage(syntax.next());
    saveImage(syntax.next(),img);
}

void
shrink2(CLArgs const & args)
{
    Syntax    syntax(args,
        "<in>.<ext> <out>.<ext>\n"
        "    <ext>      - " + imgFileExtensionsDescription()
        );
    ImgC4UC     img = loadImage(syntax.next());
    img = imgShrink2(img);
    saveImage(syntax.next(),img);
}

void
formats(CLArgs const &)
{
    Strings      fs = imgFileExtensions();
    fgout << fgnl << fs.size() << " formats supported:" << fgpush;
    char        previous = 'Z';
    for (string const & f : fs) {
        if (f[0] != previous) {
            previous = f[0];
            fgout << fgnl;
        }
        fgout << f << ", ";
    }
    fgout << fgpop;
}

}

static
void
imgops(CLArgs const & args)
{
    Cmds   ops;
    ops.push_back(Cmd(addalpha,"addalpha","Add/replace an alpha channel from an another image"));
    ops.push_back(Cmd(cmdComposite,"cmdComposite","Composite an image with transparency over another"));
    ops.push_back(Cmd(convert,"convert","Convert images between different formats"));
    ops.push_back(Cmd(formats,"formats","List all supported formats by file extension"));
    ops.push_back(Cmd(shrink2,"shrink2","Shrink images by a factor of 2"));
    doMenu(args,ops);
}

Cmd
getImgopsCmd()
{return Cmd(imgops,"image","Image operations"); }

}

// */
