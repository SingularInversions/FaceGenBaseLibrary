//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#ifndef FGIMGDISPLAY_HPP
#define FGIMGDISPLAY_HPP

#include "FgStdLibs.hpp"

#include "FgImage.hpp"
#include "FgMatrixV.hpp"
#include "FgImageIo.hpp"
#include "FgImagePoint.hpp"

namespace Fg {

// View image at exact pixel size with no controls:
void            viewImageFixed(ImgRgba8 const &);

// Interactive image view with controls:
void
viewImage(
    ImgRgba8 const &     img,
    Vec2Fs const &      ptsIucs=Vec2Fs{},           // Points to be displayed as red dots
    String const &      printHeader=String{});      // If non-empty, stream info to fgout with this header name

void    viewImage(const Img<ushort> &);
void    viewImage(const ImgF &);
void    viewImage(const ImgD &);
void    viewImage(const ImgV3F &);

// Components must be in range [0,1]:
inline void viewImage(ImgC4F const & img) {viewImage(toRgba8(img)); }

// Returns the list of landmarks after editing by user (may be less than initial):
ImagePoints
markImage(
    ImgRgba8 const &        img,
    ImagePoints const &     initial,
    // Any of these not included in 'initial' will be prompted in order for user to place:
    Strings const &         labels);

// View multiple images with selector:
void            viewImages(ImgRgba8s const & images,String8s names={});

// Images analysis / comparison.
// Image pixel values must be [0,1] per channel and have same pixel dimensions:
void            compareImages(Img3F const & image0,Img3F const & image1);

}

#endif
