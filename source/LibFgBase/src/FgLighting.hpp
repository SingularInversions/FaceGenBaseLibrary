//
// Copyright (c) 2022 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#ifndef FGLIGHT_HPP
#define FGLIGHT_HPP

#include "FgSerial.hpp"
#include "FgMatrixC.hpp"
#include "FgMatrixV.hpp"
#include "FgImage.hpp"

namespace Fg {

struct  Light
{
    Vec3F       colour {0};             // RGB range [0,1]
    Vec3F       direction {0,0,1};      // Unit direction vector to light in OECS (all lights at infinity)
    FG_SER2(colour,direction)

    Light() {}
    Light(Vec3F c) : colour(c) {}
    Light(Vec3F c,Vec3F d) : colour(c), direction(d) {}
};

typedef Svec<Light>    Lights;

struct  Lighting
{
    Vec3F               ambient;    // RGB range [0,1]
    Lights              lights;
    FG_SER2(ambient,lights)

    Lighting() : ambient{0.4f}, lights{Light{Vec3F{0.6f}}} {}
    Lighting(Vec3F a) : ambient{a} {}
    Lighting(Vec3F a,Light l) : ambient{a}, lights{l} {}
    Lighting(Vec3F a,Lights const & l) : ambient{a}, lights{l} {}
    Lighting(Light const & l) : ambient{0}, lights{l} {}

    ImgRgba8
    createSpecularMap() const;
};

}

#endif
