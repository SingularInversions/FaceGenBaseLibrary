//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

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
addalpha(const CLArgs & args)
{
    Syntax    syntax(args,
        "<rgb>.<ext> <alpha>.<ext> <out>.<ext>\n"
        "    <ext>      - " + imgFileExtensionsDescription() + "\n"
        "    <rgb>      - Any existing alpha channel will be ignored\n"
        "    <alpha>    - If this is not a single-channel image, a greyscale mapping will be applied"
        );
    ImgC4UC     rgb,alpha;
    imgLoadAnyFormat(syntax.next(),rgb);
    imgLoadAnyFormat(syntax.next(),alpha);
    if (rgb.dims() != alpha.dims())
        fgThrow("<rgb> and <alpha> images have different pixel dimensions");
    for (size_t ii=0; ii<rgb.m_data.size(); ++ii)
        rgb.m_data[ii].alpha() = alpha.m_data[ii].rec709();
    imgSaveAnyFormat(syntax.next(),rgb);
}

void
composite(const CLArgs & args)
{
    Syntax    syn(args,
        "<base>.<imgExt> <overlay>.<imgExt> <output>.<imgExt>\n"
        "    Composite images of equal pixel dimensions.\n"
        "    <overlay>.<imgExt> must have an alpha channel\n"
        "    <imgExt> = " + imgFileExtensionsDescription());
    ImgC4UC     base = imgLoadAnyFormat(syn.next()),
                    overlay = imgLoadAnyFormat(syn.next());
    if (base.dims() != overlay.dims())
        syn.error("The images must have identical pixel dimensions");
    imgSaveAnyFormat(syn.next(),fgComposite(overlay,base));
}

void
convert(const CLArgs & args)
{
    Syntax    syntax(args,
        "<in>.<ext> <out>.<ext>\n"
        "    <ext>      - " + imgFileExtensionsDescription()
        );
    ImgC4UC     img;
    imgLoadAnyFormat(syntax.next(),img);
    imgSaveAnyFormat(syntax.next(),img);
}

void
shrink2(const CLArgs & args)
{
    Syntax    syntax(args,
        "<in>.<ext> <out>.<ext>\n"
        "    <ext>      - " + imgFileExtensionsDescription()
        );
    ImgC4UC     img;
    imgLoadAnyFormat(syntax.next(),img);
    img = fgImgShrink2(img);
    imgSaveAnyFormat(syntax.next(),img);
}

void
formats(const CLArgs &)
{
    Strings      fs = imgFileExtensions();
    fgout << fgnl << fs.size() << " formats supported:" << fgpush;
    char        previous = 'Z';
    for (const string & f : fs) {
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
imgops(const CLArgs & args)
{
    vector<Cmd>   ops;
    ops.push_back(Cmd(addalpha,"addalpha","Add/replace an alpha channel from an another image"));
    ops.push_back(Cmd(composite,"composite","Composite an image with transparency over another"));
    ops.push_back(Cmd(convert,"convert","Convert images between different formats"));
    ops.push_back(Cmd(formats,"formats","List all supported formats by file extension"));
    ops.push_back(Cmd(shrink2,"shrink2","Shrink images by a factor of 2"));
    fgMenu(args,ops);
}

Cmd
fgCmdImgopsInfo()
{return Cmd(imgops,"image","Image operations"); }

}

// */
