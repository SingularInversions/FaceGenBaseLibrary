//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#ifndef FGLIGHT_HPP
#define FGLIGHT_HPP

#include "FgStdLibs.hpp"
#include "FgMatrixC.hpp"
#include "FgMatrixV.hpp"
#include "FgImage.hpp"

namespace Fg {

struct  FgLight
{
    Vec3F       colour {0.6f,0.6f,0.6f}; // RGB range [0,1]
    Vec3F       direction {0,0,1};       // Unit direction vector to light in OECS (all lights at infinity)

    FG_SERIALIZE2(colour,direction);

    FgLight() {}
    FgLight(Vec3F c,Vec3F d) : colour(c), direction(d) {}
};

typedef Svec<FgLight>    FgLights;

struct  FgLighting
{
    Vec3F            ambient;    // RGB range [0,1]
    FgLights            lights;

    FG_SERIALIZE2(ambient,lights);

    FgLighting() : ambient(0.4f) {lights.resize(1); }
    FgLighting(Vec3F a) : ambient(a) {}
    FgLighting(Vec3F a,FgLight l) : ambient(a), lights(fgSvec(l)) {}
    FgLighting(Vec3F a,FgLights const & l) : ambient(a), lights(l) {}

    ImgC4UC
    createSpecularMap() const;
};

}

#endif
