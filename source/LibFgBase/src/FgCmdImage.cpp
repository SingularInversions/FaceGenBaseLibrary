//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgCommand.hpp"
#include "FgImageIo.hpp"
#include "FgParse.hpp"
#include "FgImgDisplay.hpp"

using namespace std;

namespace Fg {

namespace {

void                cmdAlpha(CLArgs const & args)
{
    Syntax              syn(args,
        "<rgb>.<ext> <alpha>.<ext> <out>.<ext>\n"
        "    <ext>      - " + clOptionsStr(getImgExts()) + "\n"
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

void                cmdAnnotate(CLArgs const & args)
{
    Syntax              syn {args,R"(<images>
    <images>        - ( <list>.txt | (<fileName>.<ext>)+ )
    <list>.txt      - must contain a whitespace-separated list of <fileName>.<ext>
OUTPUT:
    <fileName>.lms.<ext> - for each <filename>.<ext> with associated landmarks (file name <filename>.lms.txt);
    the image annotated with landmarks as green crosses.)"
    };
    Strings             imgFiles;
    if (endsWith(syn.peekNext(),".txt"))
        imgFiles = splitWhitespace(loadRawString(syn.next()));
    else while (syn.more())
        imgFiles.push_back(syn.next());
    if (imgFiles.empty())
        syn.error("no images to annotate");
    for (String const & imgFile : imgFiles) {
        String8             lmsFile = pathToDirBase(imgFile)+".lms.txt";
        if (fileExists(lmsFile)) {
            NameVec2Fs          lms = loadLandmarks(lmsFile);
            ImgRgba8            img = loadImage(imgFile);
            for (auto lm : lms)
                paintCrosshair(img,mapRound<int>(lm.vec));
            Path                path {imgFile};
            String8             outFile = path.dirBase() + ".lms." + path.ext;
            saveImage(outFile,img);
        }
        else
            fgout << fgnl << imgFile << ": no corresponding landmarks file found";
    }
}

void                cmdCompare(CLArgs const & args)
{
    Syntax              syn {args,R"(<img0>.<ext> <img1>.<ext>
    <ext>       - )" + clOptionsStr(getImgExts())
    };
    Img3F               img0 = toUnit3F(loadImage(syn.next())),
                        img1 = toUnit3F(loadImage(syn.next()));
    if (img0.dims() != img1.dims())
        syn.error("Images not of equal pixel dimensions");
    compareImages(img0,img1);
}

void                cmdComposite(CLArgs const & args)
{
    Syntax              syn(args,
        "<base>.<imgExt> <overlay>.<imgExt> <output>.<imgExt>\n"
        "    Composite images of equal pixel dimensions.\n"
        "    <overlay>.<imgExt> must have an alpha channel\n"
        "    <imgExt> = " + clOptionsStr(getImgExts()));
    ImgRgba8            base = loadImage(syn.next()),
                        overlay = loadImage(syn.next());
    if (base.dims() != overlay.dims())
        syn.error("The images must have identical pixel dimensions");
    saveImage(syn.next(),composite(overlay,base));
}

void                cmdConvert(CLArgs const & args)
{
    String              desc;
    for (ImgFormatInfo const & ifi : getImgFormatsInfo())
        desc += "\n    " + ifi.description ;
    Syntax              syn {args,R"(<in>.<ext> <out>.<ext>
    <ext>      - )" + clOptionsStr(getImgExts()) + R"(
NOTES:)" + desc
    };
    ImgRgba8            img = loadImage(syn.next());
    saveImage(syn.next(),img);
}

void                cmdCreateCheckerboard(CLArgs const & args)
{
    Syntax              syn {args,
        R"(<num> <size> <out>.<ext>
    <num>       - number of checkerboard squares across
    <size>      - size of each square in pixels
    <ext>       - )" + clOptionsStr(getImgExts()) + R"(
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

void                cmdCreateConst(CLArgs const & args)
{
    Syntax              syn {args,
        R"(<X size> <Y size> <R> <G> <B> <A> <out>.<ext>
    <X,Y size>      - in pixels
    <R,G,B>         - [0,255]
    <ext>           - )" + clOptionsStr(getImgExts())
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

void                cmdCreate(CLArgs const & args)
{
    Cmds            cmds {
        {cmdCreateConst,"const","Create a constant-valued image"},
        {cmdCreateCheckerboard,"create","create checkerboard images"},
    };
    doMenu(args,cmds);
}

void                cmdMark(CLArgs const & args)
{
    Syntax              syn {args,
        R"([-b] [-o] <landmarks> <images>
    -b              - brighten the image with gamma correction for easier viewing of dark areas
    -o              - overwrite existing landmarks of the same name
    <landmarks>     - ( <list>.txt | (<landmarkName>)+ )
    <images>        - ( <list>.txt | (<fileName>.<ext>)+ )
    <list>.txt      - must contain a whitespace-separated list
    <landmarkName>  - cannot contain '.' character
    <ext>           - )" + clOptionsStr(getImgExts()) + R"(
OUTPUT:
    <fileName>.lms.txt  - for each <fileName> image containing landmarks added
NOTES:
    * images will be displayed in order of appearance
    * landmark names will be prompted in order of appearance
    * the .lms.txt format is one landmark per line each line consisting of <name> <X> <Y>
      where <X> and <y> are the raster positions (floating point permitted).
      Lines beginning with # are ignored (comments).
    * landmarks already existing in <fileName>.lms.txt will not be prompted for update,
      unless '-o' has been selected)"
    };
    bool                brighten = false,
                        overwrite = false;
    if (beginsWith(syn.peekNext(),"-")) {
        String              option = syn.next();
        if (option == "-b")
            brighten = true;
        else if (option == "-o")
            overwrite = true;
        else
            syn.error("Unknown flag",option);
    }
    Strings             lmNames;
    Strings             imgFiles;
    if (endsWith(syn.peekNext(),".txt"))
        lmNames = splitWhitespace(loadRawString(syn.next()));
    else {
        do
            lmNames.push_back(syn.next());
        while (!contains(syn.peekNext(),'.'));
    }
    if (lmNames.empty())
        syn.error("no landmarks to place");
    if (endsWith(syn.peekNext(),".txt"))
        imgFiles = splitWhitespace(loadRawString(syn.next()));
    else {
        do
            imgFiles.push_back(syn.next());
        while (syn.more());
    }
    if (containsDuplicates(cSort(lmNames)))
        fgout << "WARNING: There are landmark name duplicates";
    if (containsDuplicates(cSort(imgFiles)))
        fgout << "WARNING: There are file name duplicates";
    for (String const & imgFile : imgFiles) {
        PushIndent          pind {imgFile+": "};
        ImgRgba8            img = loadImage(imgFile);
        if (brighten)
            for (Rgba8 & rgba : img.m_data)
                for (uint cc=0; cc<3; ++cc)
                    rgba[cc] = scast<uchar>(pow(rgba[cc]/255.0f,1/2.5f)*255.0f+0.5f);
        NameVec2Fs          lms;
        String8             lmsFile = pathToDirBase(imgFile)+".lms.txt";
        if (fileExists(lmsFile))
            lms = loadLandmarks(lmsFile);
        if (overwrite)          // remove any existing LMs we want to overwrite:
            lms = findAll(lms,[&](NameVec2F const & l){return !contains(lmNames,l.name); });
        Strings             lmsDefined = sliceMember(lms,&NameVec2F::name);
        if (containsAll(lmsDefined,lmNames)) {
            fgout << "All LMs already defined";
            continue;
        }
        NameVec2Fs          lmsNew = markImage(img,lms,lmNames);
        if (lmsNew == lms)
            fgout << "No landmarks placed, nothing saved";
        else {
            saveLandmarks(lmsNew,lmsFile);
            fgout << lmsNew.size() << " landmarks saved in " << lmsFile;
        }
    }
}

void                cmdShrink(CLArgs const & args)
{
    Syntax              syn {args,R"(<gamma> <count> <in>.<ext> <out>.<ext>
    <ext>           - )" + clOptionsStr(getImgExts()) + R"(
    <gamma>         - [1,2] the gamma-correction built into <in> for gamma-correct resizing (use 2 if not sure)
    <count>         - Shrink the image <count> times by a factor of 2
NOTES:
    * The image is 2x2 block subsampled <count> times in linear brightness space (given by <gamma>))"
    };
    float               gamma = syn.nextAs<float>();
    if ((gamma < 1) || (gamma > 3))
        syn.error("invalid value for <gamma>");
    size_t              count = syn.nextAs<uint>();
    if (count < 1)
        syn.error("invalid value for <count>");
    ImgC4F              lin = mapGamma(toUnitC4F(loadImage(syn.next())),gamma);
    for (size_t ii=0; ii<count; ++ii)
        lin = shrink2(lin);
    saveImage(syn.next(),toRgba8(mapGamma(lin,1.0f/gamma)));
}

void                cmdImgops(CLArgs const & args)
{
    Cmds            cmds {
        {cmdAlpha,"alpha","Add/replace an alpha channel from an another image"},
        {cmdAnnotate,"anno","Paint landmark points as green crosses on an image"},
        {cmdCompare,"comp","compare two images of equal pixel dimensions"},
        {cmdComposite,"composite","Composite an image with transparency over another"},
        {cmdConvert,"convert","Convert images between different formats"},
        {cmdCreate,"create","create simple agorithmic images"},
        {cmdMark,"mark","Place landmark points on an image"},
        {cmdShrink,"shrink","Shrink images by factors of 2"},
    };
    doMenu(args,cmds);
}

}

Cmd                 getCmdImage() {return Cmd{cmdImgops,"image","Image operations"}; }

}

// */
