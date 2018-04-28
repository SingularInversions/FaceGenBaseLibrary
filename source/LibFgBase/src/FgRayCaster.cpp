//
// Copyright (c) 2018 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Andrew Beatty
// Created: 18.04.24
//
// * Possible improvments are marked 'TODO:'

#include "stdafx.h"

#include "FgRayCaster.hpp"
#include "FgGeometry.hpp"

using namespace std;

FgTriInd::FgTriInd(size_t triIdx_,size_t surfIdx_,size_t meshIdx_)
    : triIdx(uint32(triIdx_)), surfIdx(uint16(surfIdx_)), meshIdx(uint16(meshIdx_))
{
    FGASSERT(triIdx < numeric_limits<uint32>::max());
    FGASSERT(surfIdx < numeric_limits<uint16>::max());
    FGASSERT(meshIdx < numeric_limits<uint16>::max());
}

FgRayCaster::FgRayCaster(
    const Fg3dMeshes &      meshes,
    FgAffine3D              modelview,
    FgAffineCw2D            itcsToIucs_,
    const FgLighting &      lighting_,
    FgRgbaF                 background_)
    :
    itcsToIucs(itcsToIucs_),
    lighting(lighting_),
    background(background_)
{
    trisss.resize(meshes.size());
    materialss.resize(meshes.size());
    vertss.resize(meshes.size());
    uvsPtrs.resize(meshes.size());
    normss.resize(meshes.size());
    iucsVertss.resize(meshes.size());
    // TODO: set up grid only after seeing how many verts fall in frustum, possibly use smaller grid size,
    // and what their bounding box is for setting client to grid transform:
    grid.setup(FgMat22F(0,1,0,1),uint(fgNumTriEquivs(meshes)));
    for (size_t mm=0; mm<meshes.size(); ++mm) {
        const Fg3dMesh &    mesh = meshes[mm];
        FgTriss &           triss = trisss[mm];
        FgMaterials &       materials = materialss[mm];
        triss.reserve(mesh.surfaces.size());
        materials.reserve(mesh.surfaces.size());
        for (size_t ss=0; ss<mesh.surfaces.size(); ++ss) {
            triss.push_back(mesh.surfaces[ss].asTris());
            materials.push_back(mesh.surfaces[ss].material);
        }
        FgVerts &           verts = vertss[mm];
        verts = fgTransform(mesh.verts,FgAffine3F(modelview));
        uvsPtrs[mm] = &mesh.uvs;
        normss[mm] = fgNormals(mesh.surfaces,verts);
        FgVerts &           iucsVerts = iucsVertss[mm];
        iucsVerts.reserve(verts.size());
        for (FgVect3F v : verts)
            iucsVerts.push_back(oecsToIucs(v));
        for (size_t ss=0; ss<triss.size(); ++ss) {
            const FgTris &  tris = triss[ss];
            for (size_t tt=0; tt<tris.vertInds.size(); ++tt) {
                FgVect3UI       t = tris.vertInds[tt];
                FgVect3F        v0 = iucsVerts[t[0]],
                                v1 = iucsVerts[t[1]],
                                v2 = iucsVerts[t[2]];
                if ((v0[2] > 0.0f) && (v1[2] > 0.0f) && (v2[2] > 0.0f)) {   // Only render tris fully in front of camera
                    FgMat22F    bnds;
                    bnds[0] = fgMin(v0[0],v1[0],v2[0]);
                    bnds[1] = fgMax(v0[0],v1[0],v2[0]);
                    bnds[2] = fgMin(v0[1],v1[1],v2[1]);
                    bnds[3] = fgMax(v0[1],v1[1],v2[1]);
                    grid.add(FgTriInd(tt,ss,mm),bnds);
                }
            }
        }
    }
}

FgRgbaF
FgRayCaster::cast(FgVect2F posIucs) const
{
    FgBestN<float,Intersect,4>      best = closestIntersects(posIucs);

    // Compute ray color:
    FgRgbaF             color = background;
    for (uint ii=best.size(); ii>0; --ii) {             // Render back to front
        Intersect                   isct = best[ii-1].second;
        const FgTris &              tris = trisss[isct.triInd.meshIdx][isct.triInd.surfIdx];
        FgMaterial                  material = materialss[isct.triInd.meshIdx][isct.triInd.surfIdx];
        const Fg3dNormals &         norms = normss[isct.triInd.meshIdx];
        FgVect3UI                   vis = tris.vertInds[isct.triInd.triIdx];
        // TODO: Use perspective-correct normal and UV interpolation (makes very little difference for small tris):
        FgVect3F                    n0 = norms.vert[vis[0]],
                                    n1 = norms.vert[vis[1]],
                                    n2 = norms.vert[vis[2]],
                                    bc = FgVect3F(isct.barycentric),
                                    norm = fgNormalize(bc[0]*n0 + bc[1]*n1 + bc[2]*n2);
        FgRgbaF                     albedo(230,230,230,255);
        const FgVect2Fs &           uvs = *uvsPtrs[isct.triInd.meshIdx];
        if ((!tris.uvInds.empty()) && (!uvs.empty()) && (material.albedoMap) && (!material.albedoMap->empty())) {
            FgVect2F                uv;
            FgVect3UI               uvInds = tris.uvInds[isct.triInd.triIdx];
            uv = bc[0]*uvs[uvInds[0]] + bc[1]*uvs[uvInds[1]] + bc[2]*uvs[uvInds[2]];
            uv[1] = 1.0f - uv[1];   // OTCS to IUCS
            albedo = FgRgbaF(fgBlerpClipIucs(*material.albedoMap,uv));
        }
        FgVect3F            acc(0.0f);
	    float	            aw = albedo.alpha() / 255.0f;
        FgVect3F            surfColour = albedo.m_c.subMatrix<3,1>(0,0) * aw;
        for (size_t ll=0; ll<lighting.m_lights.size(); ++ll) {
            FgLight         lgt = lighting.m_lights[ll];
            float           fac = fgDot(norm,lgt.m_direction);
            if (fac > 0.0f) {
                acc += fgMapMul(surfColour,lgt.m_colour) * fac;
                if (material.shiny) {
                    FgVect3F        reflectDir = norm * fac * 2.0f - lgt.m_direction;
                    if (reflectDir[2] > 0.0f) {
                        float       deltaSqr = fgSqr(reflectDir[0]) + fgSqr(reflectDir[1]),
                                    val = exp(-deltaSqr * 32.0f);
                        acc += FgVect3F(255.0f * val);
                    }
                }
            }
        }
        acc += fgMapMul(surfColour,lighting.m_ambient);
        FgRgbaF    isctColor = FgRgbaF(acc[0],acc[1],acc[2],albedo.alpha());
        color = fgCompositeFragment(isctColor,color);
     }
     return color;
}

FgVect3F
FgRayCaster::oecsToIucs(FgVect3F posOecs) const
{
    // Inverse depth value (map to -1 for points behind camera (OECS)):
    double          id = (posOecs[2] == 0.0f) ? -1.0 : (-1.0 / posOecs[2]);
    FgVect2D        itcs(posOecs[0]*id,-posOecs[1]*id),     // Both Y and Z change sign in OECS -> ITCS
                    iucs = itcsToIucs * itcs;
    return FgVect3F(iucs[0],iucs[1],id);
}

FgBestN<float,FgRayCaster::Intersect,4>
FgRayCaster::closestIntersects(FgVect2F posIucs) const
{
    const FgTriInds &           triInds = grid[posIucs];
    FgBestN<float,Intersect,4> best;
    for (FgTriInd ti : triInds) {
        const FgTris &          tris = trisss[ti.meshIdx][ti.surfIdx];
        FgVect3UI               vis = tris.vertInds[ti.triIdx];
        const FgVerts &         iucsVerts = iucsVertss[ti.meshIdx];
        FgVect3F                v0 = iucsVerts[vis[0]],
                                v1 = iucsVerts[vis[1]],
                                v2 = iucsVerts[vis[2]];
        FgVect2D                u0(v0[0],v0[1]),
                                u1(v1[0],v1[1]),
                                u2(v2[0],v2[1]);
        // TODO: make a float version of fgBarycentriCoords:
        FgOpt<FgVect3D>         bco = fgBarycentricCoords(FgVect2D(posIucs),u0,u1,u2);
        if (bco.valid()) {       // TODO: filter out degenerate projected tris during cache setup
            FgVect3D            bc = bco.val();
            // TODO: Use a consistent intersection policy to ensure only 1 tri of an edge-connected pair
            // is ever intersected:
            if ((bc[0] >= 0) && (bc[1] >= 0) && (bc[2] >= 0)) {     // Point landed on triangle:
                double          id = bc[0]*v0[2] + bc[1]*v1[2] + bc[2]*v2[2];   // Interpolate inverse depth
                best.update(id,Intersect(ti,bc));
            }
        }
    }
    return best;
}

// */
