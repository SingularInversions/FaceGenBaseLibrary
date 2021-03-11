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

namespace Fg {

void
viewImage(
    ImgC4UC const &     img,
    Vec2Fs const &      ptsIucs=Vec2Fs{},           // Points to be displayed as red dots
    String const &      printHeader=String{});      // If non-empty, stream info to fgout with this header name

void    viewImage(const ImgUC &);
void    viewImage(const Img<ushort> &);
void    viewImage(const ImgF &);
void    viewImage(const ImgD &);
void    viewImage(const ImgV3F &);

// Components must be in range [0,1]:
inline void viewImage(ImgC4F const & img) {viewImage(toImgC4UC(img)); }

void
fgImgDisplayColorize(const ImgD &);

}

#endif
