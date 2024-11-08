//
// Copyright (c) 2023 Singular Inversions Inc.
//
// Authors:     Andrew Beatty
// Created:     October 25, 2012
//

#include "stdafx.h"

#include "FgVolume.hpp"
#include "FgCommand.hpp"

using namespace std;

namespace Fg {

VArray<CoordWgt3,8> lerpCoordsCull(Vec3UI dims,Vec3D ircs)
{
    FGASSERT(dims.elemsProduct() > 0);
    VArray<CoordWgt3,8> ret;
    Vec3I               lov = Vec3I(mapFloor(ircs)),
                        hiv = lov + Vec3I(1);
    Mat32UI             indsEub;                    // Bounds of valid indices
    for (size_t dd=0; dd<3; ++dd) {
        int             lo = lov[dd],
                        hi = hiv[dd],
                        eub = int(dims[dd]);
        if ((lo >= eub) || (hi < 0))                // All sample points outside volume
            return ret;
        indsEub.rc(dd,0) = (lo < 0) ? 1 : 0;
        indsEub.rc(dd,1) = (hi >= eub) ? 1 : 2;
    }
    Mat32UI             coords(catHoriz(lov,hiv));  // OK to cast to unsigned since negative values won't be used
    Vec3D               wgtHi = ircs - Vec3D(lov),
                        wgtLo = Vec3D(1) - wgtHi;
    Mat32D              weights = catHoriz(wgtLo,wgtHi);
    for (Iter3UI it(indsEub); it.valid(); it.next()) {
        CoordWgt3       cw;
        cw.wgt = 1.0;
        for (uint dd=0; dd<3; ++dd) {
            cw.coordIrcs[dd] = coords.rc(dd,it()[dd]);
            cw.wgt *= weights.rc(dd,it()[dd]);
        }
        ret.append(cw);
    }
    return ret;
}

static void         testLerpCull()
{
    Vec3UI              dims {2,2,2};
    VArray<CoordWgt3,8> cull;
    CoordWgt3           cw;
    {
        Vec3D           ircs {0.1,0.5,0.7};
        cull = lerpCoordsCull(dims,ircs);
        FGASSERT(cull.size() == 8);
        cw = cull[0];
        FGASSERT(cw.coordIrcs == Vec3UI(0,0,0));
        FGASSERT(isApproxEqualPrec(cw.wgt,0.9*0.5*0.3));
        cw = cull[1];
        FGASSERT(cw.coordIrcs == Vec3UI(1,0,0));
        FGASSERT(isApproxEqualPrec(cw.wgt,0.1*0.5*0.3));
        cw = cull[2];
        FGASSERT(cw.coordIrcs == Vec3UI(0,1,0));
        FGASSERT(isApproxEqualPrec(cw.wgt,0.9*0.5*0.3));
        cw = cull[4];
        FGASSERT(cw.coordIrcs == Vec3UI(0,0,1));
        FGASSERT(isApproxEqualPrec(cw.wgt,0.9*0.5*0.7));
        cw = cull[7];
        FGASSERT(cw.coordIrcs == Vec3UI(1,1,1));
        FGASSERT(isApproxEqualPrec(cw.wgt,0.1*0.5*0.7));
    }
    {
        Vec3D           pacs {0,0,0};
        cull = lerpCoordsCull(dims,pacs);
        FGASSERT(cull.size() == 8);
        cw = cull[0];
        FGASSERT(cw.coordIrcs == Vec3UI(0,0,0));
        FGASSERT(cw.wgt == 1.0);
    }
    {
        Vec3D           pacs {-0.1,-0.1,-0.1};
        cull = lerpCoordsCull(dims,pacs);
        FGASSERT(cull.size() == 1);
        cw = cull[0];
        FGASSERT(cw.coordIrcs == Vec3UI(0,0,0));
        FGASSERT(cw.wgt == cube(0.9));
    }
}

static void         testVolCtr()
{
    VolF            vol(1,1.0f);
    Vec3UIs         maxs = cMaxima(vol);
    FGASSERT(maxs.size() == 1);
    FGASSERT(maxs[0] == Vec3UI(0));
    vol = VolF(3,1.0f);
    Vec3UI          p0(0,0,0),
                    p2(2,2,2);
    vol[p0] = 2.0f;
    vol[p2] = 2.0f;
    maxs = cMaxima(vol);
    FGASSERT(maxs.size() == 2);
    FGASSERT(maxs[0] == p0);
    FGASSERT(maxs[1] == p2);
    vol.xyz(1,1,1) = 2.0f;
    maxs = cMaxima(vol);
    FGASSERT(maxs.empty());     // No longer strict maxima
}

void                testVolume(CLArgs const &)
{
    testVolCtr();
    testLerpCull();
}

}

// */
