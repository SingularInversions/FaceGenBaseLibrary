//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Coordinate system definitions that don't have a natural place elsewhere

#ifndef FGCOORDSYSTEM_HPP
#define FGCOORDSYSTEM_HPP

#include "FgMatrixC.hpp"
#include "FgAffineCwC.hpp"
#include "FgMath.hpp"

// FHCS := FaceGen head CS
//    Origin in face centre on saggital plane
//    Units are base head millimetres ie. an IPD of ~ 63.5mm on the base head.
//    X - Head’s left.
//    Y - Head’s up.
//    Z - Head’s forward. 
//
// CCS := Camera CS
//    Origin at effective pinhole, no units specified
//    X - camera’s right
//    Y - camera’s down
//    Z - camera’s facing 
//
// ITCS := Image Tangent CS
//    Origin at principal point (where optical axis intersects image plane), units are tangent of view angle.
//    X - viewer’s right
//    Y - viewer’s down 
//
// OpenGL Overview:
//    modelview: World frame -> OECS
//    projection matrix: OECS -> ONDCS
//    viewport transform: ONDCS -> OWCS 
//
// OECS := OpenGL Eye CS / Camera Frame / OCCS
//    Origin at effective pinhole, no units specified.
//    X - viewer’s right
//    Y - viewer’s up
//    Z - opposite of camera viewing direction
//
// ONDCS := OpenGL Normalized Device CS / Canonical Clipping Volume
//    The view volume (frustum) is transformed into (x,y,z) in the range [-1,1].
//    Note that the Z values are inverted, so the most distant objects have the largest Z values
//    and the values are non-linear (one over the distance from the camera). 
//
// OICS := OpenGL Image CS
//    Informal term referring to just the X,Y elements of ONDCS
//
// OWCS := OpenGL Window CS (Screen Coordinates once z is dropped)
//    X - viewer’s right    [0,1]
//    Y - viewer’s up       [0,1]
//    Z - Negative inverse of the depth in frustum, used for z-buffer.
//
// OTCS := OGL Texture CS
//    X - viewer’s right    [0,1]
//    Y - viewer’s up       [0,1]
//

namespace Fg {

template<typename T>
Mat<T,3,3>
fgHcsToCcs()
{
    return Mat<T,3,3> {
        1, 0, 0,
        0,-1, 0,
        0, 0,-1
    };
}

template<typename T>
Mat<T,4,4>
fgHcsToCcsH()
{
    return Mat<T,4,4> {{{
        1, 0, 0, 0,
        0,-1, 0, 0,
        0, 0,-1, 0,
        0, 0, 0, 1
    }}};
}

template<typename T>
Mat<T,4,4>
fgProjectOecsToItcs()
{
    return Mat<T,4,4> {{{
        1, 0, 0, 0,
        0,-1, 0, 0,
        0, 0, 0, 1,
        0, 0,-1, 0
    }}};
}

}

// */

#endif
