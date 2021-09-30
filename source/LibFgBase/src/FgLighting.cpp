//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgLighting.hpp"
#include "FgImage.hpp"
#include "FgMath.hpp"

using namespace std;

namespace Fg {

ImgRgba8
Lighting::createSpecularMap() const
{
    FGASSERT(lights.size() > 0);

    float   fresnelLow = 1.5f,      // 90 degree angle brightness
            fresnelHigh = 1.5f,     // 0 degree angle brightness
            falloffStd = 0.1f;      // Must be greater than 0 and less than 1/sqrt(2).

    float                    invVar = 1.0f / (2.0f * sqr(falloffStd));
    ImgRgba8              img(128,128);
    for (uint py=0; py<128; py++) {
        float    yy = ((float)py - 63.5f) / 64.0f;
        for (uint px=0; px<128; px++) {
            float   xx = ((float)px - 63.5f) / 64.0f,
                    sq = xx*xx + yy*yy;
            if (sq > 1.0f)                    // Outside valid spherical region.
                img.xy(px,py) = RgbaUC(0,0,0,1);
            else {
                float   aa = 2.0f * sqrt(1.0f - sq),
                        sr = sqrt(sq),
                        fresnel = (1.0f - sr) * fresnelLow + sr * fresnelHigh;
                Vec3F        r(aa*xx,aa*yy,1.0f-2.0f*sq);
                RgbaF         pix(0,0,0,255);
                for (uint ll=0; ll<lights.size(); ll++)
                {
                    float       diffSqr = (r - lights[ll].direction).mag(),
                                bright = exp(-diffSqr * invVar) * fresnel * 255.0f;
                    pix.red() += lights[ll].colour[0] * bright;
                    pix.green() += lights[ll].colour[1] * bright;
                    pix.blue() += lights[ll].colour[2] * bright;
                }
                if (pix.red() > 255.0f) pix.red() = 255.0f;
                if (pix.green() > 255.0f) pix.green() = 255.0f;
                if (pix.blue() > 255.0f) pix.blue() = 255.0f;
                deepCast_(pix,img.xy(px,py));
            }
        }
    }
    return img;
}

}

// */
