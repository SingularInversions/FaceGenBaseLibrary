//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     May 24, 2005
//

#ifndef FGLIGHT_HPP
#define FGLIGHT_HPP

#include "FgStdLibs.hpp"
#include "FgMatrix.hpp"
#include "FgImage.hpp"

struct  FgLight
{
    FgVect3F       colour;          // RGB range [0,1]
    FgVect3F       direction;       // Unit direction vector to light in OECS (all lights at infinity)

    FG_SERIALIZE2(colour,direction);

    FgLight() :
        colour(0.6f,0.6f,0.6f),
        direction(0.0f,0.0f,1.0f)   // At infinity behind camera (OECS)
    {}
};

struct  FgLighting
{
    FgVect3F            ambient;    // RGB range [0,1]
    vector<FgLight>     lights;

    FG_SERIALIZE2(ambient,lights);

    FgLighting() :
    ambient(0.4f,0.4f,0.4f)
    {lights.resize(1); }

    FgImgRgbaUb
    createSpecularMap() const;
};

#endif
