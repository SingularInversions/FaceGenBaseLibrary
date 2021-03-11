//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Point annonations for images. Load / save to simple YOLO text format.
//

#ifndef FGIMAGEPOINT_HPP
#define FGIMAGEPOINT_HPP

#include "FgImage.hpp"

namespace Fg {

struct  ImagePoint
{
    String          label;
    Vec2F           posIrcs;        // Image raster coordinate system

    ImagePoint() {}
    ImagePoint(String const & l,Vec2F p) : label{l}, posIrcs{p} {}

    bool operator==(ImagePoint const & rhs) const
    {return (label == rhs.label); }
};
typedef Svec<ImagePoint>    ImagePoints;

std::ostream &
operator<<(std::ostream &,ImagePoint const &);

std::ostream &
operator<<(std::ostream &,ImagePoints const &);

ImagePoints loadImagePoints(String8 const & fname);
void        saveImagePoints(ImagePoints const & ips,String8 const & fname);

void
merge_(ImagePoints & lhs,ImagePoints const & rhs);

}

#endif
