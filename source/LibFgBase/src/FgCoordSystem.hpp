//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Feb 9, 2012
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
//    Origin at principal point, units are tangent of view angle.
//    X - viewer’s right
//    Y - viewer’s down 
//
// OpenGL Overview:
//    modelview: World frame -> OECS
//    projection matrix: OECS -> ONDCS
//    viewport transform: ONDCS -> OWCS 
//
// OECS := OpenGL Eye CS / Camera Frame
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
// OWCS := OpenGL Window CS (Screen Coordinates once z is dropped)
//    Origin: Bottom left corner of image area, (1,1) top right corner of image area
//    X - viewer’s right
//    Y - viewer’s up
//    Z - Negative inverse of the depth in frustum, used for z-buffer. 
//
// OTCS := OGL Texture CS
//    Origin at bottom left corner of image area, (1,1) at top right corner.
//    X - viewer’s right
//    Y - viewer’s up 
//

inline
FgMat33D
fgHcsToCcs()
{return FgMat33D(1,0,0,0,-1,0,0,0,-1); }

inline
FgMat44D
fgHcsToCcsH()
{
    double  m[] = {
        1.0,  0.0,  0.0,  0.0,
        0.0, -1.0,  0.0,  0.0,
        0.0,  0.0, -1.0,  0.0,
        0.0,  0.0,  0.0,  1.0 };
    return FgMat44D::fromPtr(m);
}

// */

#endif
