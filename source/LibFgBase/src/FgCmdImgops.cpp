//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Andrew Beatty
// Created: Jan 11, 2014
//

#include "stdafx.h"

#include "FgCommand.hpp"
#include "FgSyntax.hpp"
#include "FgMetaFormat.hpp"
#include "FgImage.hpp"

using namespace std;

static
void
addalpha(const FgArgs & args)
{
    FgSyntax    syntax(args,
        "<rgb>.<ext> <alpha>.<ext> <out>.<ext>\n"
        "    <ext>      - " + fgImgCommonFormatsDescription() + "\n"
        "    <rgb>      - Any existing alpha channel will be ignored\n"
        "    <alpha>    - If this is not a single-channel image, a greyscale mapping will be applied"
        );
    FgImgRgbaUb     rgb,alpha;
    fgLoadImgAnyFormat(syntax.next(),rgb);
    fgLoadImgAnyFormat(syntax.next(),alpha);
    if (rgb.dims() != alpha.dims())
        fgThrow("<rgb> and <alpha> images have different pixel dimensions");
    for (size_t ii=0; ii<rgb.m_data.size(); ++ii)
        rgb.m_data[ii].alpha() = alpha.m_data[ii].rec709();
    fgSaveImgAnyFormat(syntax.next(),rgb);
}

static
void
convert(const FgArgs & args)
{
    FgSyntax    syntax(args,
        "<in>.<ext> <out>.<ext>\n"
        "    <ext>      - " + fgImgCommonFormatsDescription()
        );
    FgImgRgbaUb     img;
    fgLoadImgAnyFormat(syntax.next(),img);
    fgSaveImgAnyFormat(syntax.next(),img);
}

static
void
shrink2(const FgArgs & args)
{
    FgSyntax    syntax(args,
        "<in>.<ext> <out>.<ext>\n"
        "    <ext>      - " + fgImgCommonFormatsDescription()
        );
    FgImgRgbaUb     img;
    fgLoadImgAnyFormat(syntax.next(),img);
    img = fgImgShrink2(img);
    fgSaveImgAnyFormat(syntax.next(),img);
}

static
void
formats(const FgArgs &)
{
    FgStrs      fs = fgImgSupportedFormats();
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

static
void
imgops(const FgArgs & args)
{
    vector<FgCmd>   ops;
    ops.push_back(FgCmd(addalpha,"addalpha","Add/replace an alpha channel from an another image"));
    ops.push_back(FgCmd(convert,"convert","Convert images between different formats"));
    ops.push_back(FgCmd(formats,"formats","List all supported formats by file extension"));
    ops.push_back(FgCmd(shrink2,"shrink2","Shrink images by a factor of 2"));
    fgMenu(args,ops);
}

FgCmd
fgCmdImgopsInfo()
{return FgCmd(imgops,"imgops","Operations on images"); }

// */
