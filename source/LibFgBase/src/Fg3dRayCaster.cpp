//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Andrew Beatty
//

#include "stdafx.h"

#include "Fg3dRayCaster.hpp"

using namespace std;

Fg3dRayCaster::Fg3dRayCaster(
    vector<FgSurfPtr>           rs,
    FgFuncShader                shader,
    FgAffine3F                  modelview,
    FgAffineCw2F                itcsToIucs,
    FgRgbaF                     background)
    :
    m_shader(shader),
    m_background(background)
{
    m_surfs.reserve(rs.size());
    for (size_t ii=0; ii<rs.size(); ++ii)
        m_surfs.push_back(FgSurfRay(rs[ii],modelview,itcsToIucs));
}

FgSurfRay::FgSurfRay(
    FgSurfPtr               rs,
    FgAffine3F              modelview,
    FgAffineCw2F            itcsToIucs)
    :
    surf(rs)
{
    const FgVerts &         verts = *(surf.verts);
    depth.resize(verts.size());
    vertsIucs.resize(verts.size());
    for (size_t ii=0; ii<verts.size(); ++ii) {
        FgVect3F    vertOecs = modelview * verts[ii];
        FGASSERT(vertOecs[2] < 0.0f);
        depth[ii] = -vertOecs[2];
        FgVect2F    vertItcs;
        // project with -Z then flip Y and Z axes to get ITCS. Simplified gives:
        vertItcs[0] = -vertOecs[0] / vertOecs[2];
        vertItcs[1] = vertOecs[1] / vertOecs[2];
        vertsIucs[ii] = itcsToIucs * vertItcs;
    }
    grid = fgGridTriangles(vertsIucs,*(rs.vertInds));
    fgTransform_(rs.norms->vert,norms,modelview.linear);
}

FgRgbaF
Fg3dRayCaster::operator()(FgVect2F posIucs) const
{
    // Find closest ray intersection:
    FgBestN<float,Best,8>   bestAll;
    for (size_t ii=0; ii<m_surfs.size(); ++ii) {
        FgBestN<float,FgTriPoint,8>
            best = m_surfs[ii].cast(posIucs);
        for (uint jj=0; jj<best.num(); ++jj)
            if (!bestAll.update(best[jj].key,Best(ii,best[jj].val)))
                break;
    }
    FgRgbaF     acc = m_background;
    for (uint ii=bestAll.num(); ii>0; --ii)
        acc = fgCompositeFragment(
            m_surfs[bestAll[ii-1].val.surfIdx].shade(m_shader,bestAll[ii-1].val.intersect),
            acc);
    return acc;
}

FgBestN<float,FgTriPoint,8>
FgSurfRay::cast(FgVect2F posIucs)
    const
{
    FgBestN<float,FgTriPoint,8> retval;
    std::vector<FgTriPoint>  intersects = grid.intersects(*(surf.vertInds),vertsIucs,posIucs);
    for (size_t ii=0; ii<intersects.size(); ++ii) {
        FgTriPoint  isect = intersects[ii];
        float   newDepth =
            isect.baryCoord[0] * depth[isect.pointInds[0]] +
            isect.baryCoord[1] * depth[isect.pointInds[1]] +
            isect.baryCoord[2] * depth[isect.pointInds[2]];
        retval.update(newDepth,isect);
    }
    return retval;
}

FgRgbaF
FgSurfRay::shade(
    FgFuncShader        shader,
    const FgTriPoint &  intersect)
    const
{
    FgVect3UI   tri = intersect.pointInds;
    FgVect3F    bCoord = intersect.baryCoord;
    FgVect3F    norm = bCoord[0] * norms[tri[0]] +
                       bCoord[1] * norms[tri[1]] +
                       bCoord[2] * norms[tri[2]];
    norm /= norm.length();
    FgVect3UI   uvInds = (*surf.uvInds)[intersect.triInd];
    FgVect2F    uv = bCoord[0] * (*surf.uvs)[uvInds[0]] +
                     bCoord[1] * (*surf.uvs)[uvInds[1]] +
                     bCoord[2] * (*surf.uvs)[uvInds[2]];
    uv[1] = 1.0f - uv[1];   // OTCS to IUCS
    return shader(norm,uv,surf.material,surf.texImg);
}

// */
