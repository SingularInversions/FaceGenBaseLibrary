//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//


#include "stdafx.h"

#include "Fg3dRayCaster.hpp"

using namespace std;

namespace Fg {

Fg3dRayCastMesh::Fg3dRayCastMesh(
    Mesh const &        mesh,
    Vec3Fs const &         verts,
    const Normals &     norms,
    AffineEw2F            itcsToIucs)
    :
    vertsPtr(&verts),
    normsPtr(&norms),
    uvsPtr(&mesh.uvs),
    material(mesh.surfaces[0].material)
{
    for (Surf const & s : mesh.surfaces) {
        surfs.push_back(s.convertToTris());
        surfs.back().material.albedoMap = s.material.albedoMap;
    }
    invDepths.resize(verts.size());
    vertsIucs.resize(verts.size());
    for (size_t ii=0; ii<verts.size(); ++ii) {
        Vec3F    vertOecs = verts[ii];
        FGASSERT(vertOecs[2] < 0.0f);
        invDepths[ii] = -1.0f / vertOecs[2];
        Vec2F    vertItcs;
        // project with -Z then flip Y and Z axes to get ITCS. Simplified gives:
        vertItcs[0] = -vertOecs[0] / vertOecs[2];
        vertItcs[1] = vertOecs[1] / vertOecs[2];
        vertsIucs[ii] = itcsToIucs * vertItcs;
    }
    grid = gridTriangles(vertsIucs,surfs[0].tris.posInds);
}

FgBestN<float,TriPoint,8>
Fg3dRayCastMesh::cast(Vec2F posIucs) const
{
    FgBestN<float,TriPoint,8> retval;
    vector<TriPoint>          intersects = grid.intersects(surfs[0].tris.posInds,vertsIucs,posIucs);
    for (size_t ii=0; ii<intersects.size(); ++ii) {
        TriPoint  isect = intersects[ii];
        float   newInvDepth =
            isect.baryCoord[0] * invDepths[isect.pointInds[0]] +
            isect.baryCoord[1] * invDepths[isect.pointInds[1]] +
            isect.baryCoord[2] * invDepths[isect.pointInds[2]];
        retval.update(newInvDepth,isect);
    }
    return retval;
}

RgbaF
Fg3dRayCastMesh::shade(const TriPoint & intersect,const Lighting & lighting) const
{
    Vec3UI           tri = intersect.pointInds;
    Vec3F            bCoord = intersect.baryCoord;
    Vec3F            norm = bCoord[0] * normsPtr->vert[tri[0]] +
                            bCoord[1] * normsPtr->vert[tri[1]] +
                            bCoord[2] * normsPtr->vert[tri[2]];
    norm /= norm.len();
    Vec2Fs const &       uvs = *uvsPtr;
    Vec2F                uv {0};
    if (!uvs.empty()) {
        Vec3UI           uvInds = surfs[0].tris.uvInds[intersect.triInd];
        uv = bCoord[0] * uvs[uvInds[0]] +
             bCoord[1] * uvs[uvInds[1]] +
             bCoord[2] * uvs[uvInds[2]];
        uv[1] = 1.0f - uv[1];   // OTCS to IUCS
    }
    const ImgC4UC * img = (surfs[0].material.albedoMap) ? &(*surfs[0].material.albedoMap) : NULL;
    Vec3F           acc {0};
    RgbaF           texSample = (img && (!img->empty())) ?
        RgbaF(sampleClipIucs(*img,uv)) :
        RgbaF(230.0f,230.0f,230.0f,255.0f);
	float	        aw = texSample.alpha() / 255.0f;
    Vec3F        surfColour = texSample.m_c.subMatrix<3,1>(0,0) * aw;
    for (size_t ll=0; ll<lighting.lights.size(); ++ll) {
        Light     lgt = lighting.lights[ll];
        float       fac = cDot(norm,lgt.direction);
        if (fac > 0.0f) {
            acc += mapMul(surfColour,lgt.colour) * fac;
            if (material.shiny) {
                Vec3F    reflectDir = norm * fac * 2.0f - lgt.direction;
                if (reflectDir[2] > 0.0f) {
                    float   deltaSqr = sqr(reflectDir[0]) + sqr(reflectDir[1]),
                            val = exp(-deltaSqr * 32.0f);
                    acc += Vec3F(255.0f * val);
                }
            }
        }
    }
    acc += mapMul(surfColour,lighting.ambient);
    return RgbaF(acc[0],acc[1],acc[2],texSample.alpha());
}

Fg3dRayCaster::Fg3dRayCaster(
    const Meshes &          meshes,
    const Vec3Fss &            vertss,
    const Normalss &        normss,
    const Lighting &          lighting,
    AffineEw2F                itcsToIucs,
    RgbaF                     background)
    :
    lightingPtr(&lighting),
    m_background(background)
{
    FGASSERT(meshes.size() == vertss.size());
    FGASSERT(meshes.size() == normss.size());
    rayMesh.reserve(meshes.size());
    for (size_t ii=0; ii<meshes.size(); ++ii)
        rayMesh.push_back(Fg3dRayCastMesh(meshes[ii],vertss[ii],normss[ii],itcsToIucs));
}

RgbaF
Fg3dRayCaster::cast(Vec2F posIucs) const
{
    // Find closest ray intersection:
    FgBestN<float,Best,8>   bestAll;
    for (size_t ii=0; ii<rayMesh.size(); ++ii) {
        FgBestN<float,TriPoint,8>     best = rayMesh[ii].cast(posIucs);
        for (uint jj=0; jj<best.size(); ++jj)
            if (!bestAll.update(best[jj].first,Best(ii,best[jj].second)))
                break;
    }
    RgbaF     acc = m_background;
    for (uint ii=bestAll.size(); ii>0; --ii)    // Render back to front
        acc = compositeFragment(rayMesh[bestAll[ii-1].second.surfIdx].shade(bestAll[ii-1].second.intersect,*lightingPtr),acc);
    return acc;
}

}

// */
