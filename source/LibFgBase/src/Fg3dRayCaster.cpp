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

Fg3dRayCastMesh::Fg3dRayCastMesh(
        const Fg3dMesh &        mesh,
        const FgVerts &         verts_,
        const Fg3dNormals &     norms_,
        FgAffineCw2F            itcsToIucs)
    :
    verts(verts_),
    norms(norms_),
    uvs(mesh.uvs),
    material(mesh.surfaces[0].material)
{
    for (const Fg3dSurface & s : mesh.surfaces) {
        surfs.push_back(s.convertToTris());
        surfs.back().material.albedoMap = s.material.albedoMap;
    }
    invDepths.resize(verts.size());
    vertsIucs.resize(verts.size());
    for (size_t ii=0; ii<verts.size(); ++ii) {
        FgVect3F    vertOecs = verts[ii];
        FGASSERT(vertOecs[2] < 0.0f);
        invDepths[ii] = -1.0f / vertOecs[2];
        FgVect2F    vertItcs;
        // project with -Z then flip Y and Z axes to get ITCS. Simplified gives:
        vertItcs[0] = -vertOecs[0] / vertOecs[2];
        vertItcs[1] = vertOecs[1] / vertOecs[2];
        vertsIucs[ii] = itcsToIucs * vertItcs;
    }
    grid = fgGridTriangles(vertsIucs,surfs[0].tris.vertInds);
}

FgBestN<float,FgTriPoint,8>
Fg3dRayCastMesh::cast(FgVect2F posIucs) const
{
    FgBestN<float,FgTriPoint,8> retval;
    vector<FgTriPoint>          intersects = grid.intersects(surfs[0].tris.vertInds,vertsIucs,posIucs);
    for (size_t ii=0; ii<intersects.size(); ++ii) {
        FgTriPoint  isect = intersects[ii];
        float   newInvDepth =
            isect.baryCoord[0] * invDepths[isect.pointInds[0]] +
            isect.baryCoord[1] * invDepths[isect.pointInds[1]] +
            isect.baryCoord[2] * invDepths[isect.pointInds[2]];
        retval.update(newInvDepth,isect);
    }
    return retval;
}

FgRgbaF
Fg3dRayCastMesh::shade(const FgTriPoint & intersect,const FgLighting & lighting) const
{
    FgVect3UI           tri = intersect.pointInds;
    FgVect3F            bCoord = intersect.baryCoord;
    FgVect3F            norm = bCoord[0] * norms.vert[tri[0]] +
                            bCoord[1] * norms.vert[tri[1]] +
                            bCoord[2] * norms.vert[tri[2]];
    norm /= norm.length();
    FgVect2F                uv;
    if (!uvs.empty()) {
        FgVect3UI           uvInds = surfs[0].tris.uvInds[intersect.triInd];
        uv = bCoord[0] * uvs[uvInds[0]] +
             bCoord[1] * uvs[uvInds[1]] +
             bCoord[2] * uvs[uvInds[2]];
        uv[1] = 1.0f - uv[1];   // OTCS to IUCS
    }
    const FgImgRgbaUb *     img = (surfs[0].material.albedoMap) ? &(*surfs[0].material.albedoMap) : NULL;
    FgVect3F        acc(0.0f);
    FgRgbaF         texSample = (img && (!img->empty())) ?
        FgRgbaF(fgBlerpClipIucs(*img,uv)) :
        FgRgbaF(230.0f,230.0f,230.0f,255.0f);
	float	        aw = texSample.alpha() / 255.0f;
    FgVect3F        surfColour = texSample.m_c.subMatrix<3,1>(0,0) * aw;
    for (size_t ll=0; ll<lighting.m_lights.size(); ++ll) {
        FgLight     lgt = lighting.m_lights[ll];
        float       fac = fgDot(norm,lgt.m_direction);
        if (fac > 0.0f) {
            acc += fgMapMul(surfColour,lgt.m_colour) * fac;
            if (material.shiny) {
                FgVect3F    reflectDir = norm * fac * 2.0f - lgt.m_direction;
                if (reflectDir[2] > 0.0f) {
                    float   deltaSqr = fgSqr(reflectDir[0]) + fgSqr(reflectDir[1]),
                            val = exp(-deltaSqr * 32.0f);
                    acc += FgVect3F(255.0f * val);
                }
            }
        }
    }
    acc += fgMapMul(surfColour,lighting.m_ambient);
    return FgRgbaF(acc[0],acc[1],acc[2],texSample.alpha());
}

Fg3dRayCaster::Fg3dRayCaster(
    const Fg3dMeshes &          meshes,
    const FgVertss &            vertss,
    const Fg3dNormalss &        normss,
    const FgLighting &          lighting_,
    FgAffineCw2F                itcsToIucs,
    FgRgbaF                     background)
    :
    lighting(lighting_),
    m_background(background)
{
    FGASSERT(meshes.size() == vertss.size());
    FGASSERT(meshes.size() == normss.size());
    rayMesh.reserve(meshes.size());
    for (size_t ii=0; ii<meshes.size(); ++ii)
        rayMesh.push_back(Fg3dRayCastMesh(meshes[ii],vertss[ii],normss[ii],itcsToIucs));
}

FgRgbaF
Fg3dRayCaster::cast(FgVect2F posIucs) const
{
    // Find closest ray intersection:
    FgBestN<float,Best,8>   bestAll;
    for (size_t ii=0; ii<rayMesh.size(); ++ii) {
        FgBestN<float,FgTriPoint,8>     best = rayMesh[ii].cast(posIucs);
        for (uint jj=0; jj<best.size(); ++jj)
            if (!bestAll.update(best[jj].first,Best(ii,best[jj].second)))
                break;
    }
    FgRgbaF     acc = m_background;
    for (uint ii=bestAll.size(); ii>0; --ii)    // Render back to front
        acc = fgCompositeFragment(rayMesh[bestAll[ii-1].second.surfIdx].shade(bestAll[ii-1].second.intersect,lighting),acc);
    return acc;
}

// */
