//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#ifndef FGIMGDISPLAY_HPP
#define FGIMGDISPLAY_HPP

#include "FgSerial.hpp"

#include "FgImage.hpp"
#include "FgMatrixV.hpp"
#include "FgImageIo.hpp"

namespace Fg {

// Interactive image view with controls:
void                viewImage(ImgRgba8 const & img,Vec2Fs const & ptsIrcs={});

struct      AnnotatedImg
{
    ImgRgba8            img;
    Vec2Fs              ptsIrcs;
    String8             name;
};
typedef Svec<AnnotatedImg>  AnnotatedImgs;

// View multiple images with selector:
void                viewImages(AnnotatedImgs const &);
void                viewImages(ImgRgba8s const & imgs);     // no pts and numerical names

// Returns the list of landmarks after editing by user (may be less than initial):
NameVec2Fs          markImage(
    ImgRgba8 const &        img,
    NameVec2Fs const &      initial,
    // Any of these not included in 'initial' will be prompted in order for user to place:
    Strings const &         labels);

// Images analysis / comparison.
// Image pixel values must be [0,1] per channel and have same pixel dimensions:
void                compareImages(Img3F const & image0,Img3F const & image1);

}

#endif
