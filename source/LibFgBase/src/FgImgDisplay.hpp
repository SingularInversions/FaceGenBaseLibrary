//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
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

void    imgDisplay(ImgC4UC const &,Svec<Vec2F> ptsIucs=Svec<Vec2F>());
void    imgDisplay(const ImgUC &);
void    imgDisplay(const Img<ushort> &);
void    imgDisplay(const ImgF &);
void    imgDisplay(const ImgD &);
void    imgDisplay(const Img3F &);

// Components must be in range [0,1]:
void    imgDisplay(const ImgC4F &);

void
fgImgDisplayColorize(const ImgD &);

}

#endif
