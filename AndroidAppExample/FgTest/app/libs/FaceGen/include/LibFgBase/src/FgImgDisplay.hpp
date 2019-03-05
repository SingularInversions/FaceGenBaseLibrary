//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     May 6, 2011
//

#ifndef FGIMGDISPLAY_HPP
#define FGIMGDISPLAY_HPP

#include "FgStdLibs.hpp"

#include "FgAlgs.hpp"
#include "FgImage.hpp"
#include "FgMatrixV.hpp"
#include "FgImageIo.hpp"

void    fgImgDisplay(const FgImgRgbaUb &,vector<FgVect2F> ptsIucs=vector<FgVect2F>());
void    fgImgDisplay(const FgImgUC &);
void    fgImgDisplay(const FgImage<ushort> &);
void    fgImgDisplay(const FgImgF &);
void    fgImgDisplay(const FgImgD &);
void    fgImgDisplay(const FgImg3F &);

// Components must be in range [0,1]:
void    fgImgDisplay(const FgImgRgbaF &);

void
fgImgDisplayColorize(const FgImgD &);

#endif
