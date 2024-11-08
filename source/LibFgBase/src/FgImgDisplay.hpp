//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#ifndef FGIMGDISPLAY_HPP
#define FGIMGDISPLAY_HPP

#include "FgImage.hpp"
#include "FgImageIo.hpp"

namespace Fg {

// Interactive image view with controls:
void                viewImage(ImgRgba8 const & img,Vec2Fs const & lmsIrcs={});

struct      ImageLmsName
{
    ImgRgba8            img;
    NameVec2Fs          lmsIrcs;
    String8             name;

    ImageLmsName() {}
    explicit ImageLmsName(String8 const & imgFname);     // loads related LMS file if present
    ImageLmsName(String8 const & imgFname,String8 const & lmsFname);
    ImageLmsName(ImgRgba8 const & i,NameVec2Fs const & l,String8 const & n) : img{i}, lmsIrcs{l}, name{n} {}
};
typedef Svec<ImageLmsName>  ImageLmsNames;
std::ostream &      operator<<(std::ostream &,ImageLmsName const &);

// View multiple images with selector:
void                viewImages(ImageLmsNames const &);
void                viewImages(ImgRgba8s const & imgs);     // no pts and numerical names

// Returns the list of landmarks after editing by user (may be less than initial):
NameVec2Fs          markImage(
    ImgRgba8 const &        img,
    NameVec2Fs const &      initial,
    // Any of these not included in 'initial' will be prompted in order for user to place:
    Strings const &         labels);

// make multiple landmarks with the same label. Return value includes existing. Inputs and outputs in IRCS:
NameVec2Fs          markImageMulti(ImgRgba8 const & img,NameVec2Fs const & existing,String label);

// Images analysis / comparison.
// Image pixel values must be [0,1] per channel and have same pixel dimensions:
void                compareImages(Img3F const & image0,Img3F const & image1);

}

#endif
