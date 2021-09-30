//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// * Possible improvments are marked 'TODO:'

#include "stdafx.h"

#include "FgRayCaster.hpp"
#include "FgGeometry.hpp"

using namespace std;

namespace Fg {

RayCaster::RayCaster(
    Meshes const &      meshes,
    SimilarityD         modelview,
    AffineEw2D          itcsToIucs_,
    Lighting const &    lighting_,
    RgbaF               background_,
    bool                useMaps_,
    bool                allShiny_)
    :
    itcsToIucs(itcsToIucs_),
    // TODO: set up grid only after seeing how many verts fall in frustum, possibly use smaller grid size,
    // and what their bounding box is for setting client to grid transform:
    grid {Mat22F{0,1,0,1},cNumTriEquivs(meshes)},
    lighting(lighting_),
    background(background_),
    useMaps(useMaps_),
    allShiny(allShiny_)
{
    trisss.resize(meshes.size());
    materialss.resize(meshes.size());
    vertss.resize(meshes.size());
    uvsPtrs.resize(meshes.size());
    normss.resize(meshes.size());
    iucsVertss.resize(meshes.size());
    for (size_t mm=0; mm<meshes.size(); ++mm) {
        Mesh const &    mesh = meshes[mm];
        Triss &           triss = trisss[mm];
        Materials &       materials = materialss[mm];
        triss.reserve(mesh.surfaces.size());
        materials.reserve(mesh.surfaces.size());
        for (size_t ss=0; ss<mesh.surfaces.size(); ++ss) {
            triss.push_back(mesh.surfaces[ss].asTris());
            materials.push_back(mesh.surfaces[ss].material);
        }
        Vec3Fs &           verts = vertss[mm];
        verts = mapMul(Affine3F{modelview.asAffine()},mesh.verts);
        uvsPtrs[mm] = &mesh.uvs;
        normss[mm] = cNormals(mesh.surfaces,verts);
        Vec3Fs &           iucsVerts = iucsVertss[mm];
        iucsVerts.reserve(verts.size());
        for (Vec3F v : verts)
            iucsVerts.push_back(oecsToIucs(v));
        for (size_t ss=0; ss<triss.size(); ++ss) {
            Tris const &  tris = triss[ss];
            for (size_t tt=0; tt<tris.posInds.size(); ++tt) {
                Vec3UI       t = tris.posInds[tt];
                Vec3F        v0 = iucsVerts[t[0]],
                                v1 = iucsVerts[t[1]],
                                v2 = iucsVerts[t[2]];
                if ((v0[2] > 0.0f) && (v1[2] > 0.0f) && (v2[2] > 0.0f)) {   // Only render tris fully in front of camera
                    Mat22F    bnds;
                    bnds[0] = cMin(v0[0],v1[0],v2[0]);
                    bnds[1] = cMax(v0[0],v1[0],v2[0]);
                    bnds[2] = cMin(v0[1],v1[1],v2[1]);
                    bnds[3] = cMax(v0[1],v1[1],v2[1]);
                    grid.add(TriInd(tt,ss,mm),bnds);
                }
            }
        }
    }
}

RgbaF
RayCaster::cast(Vec2F posIucs) const
{
    BestN<float,Intersect,4>      best = closestIntersects(posIucs);
    // Compute ray color:
    RgbaF               color = background;
    for (uint ii=best.size(); ii>0; --ii) {             // Render back to front
        Intersect           isct = best[ii-1].second;
        Tris const &        tris = trisss[isct.triInd.meshIdx][isct.triInd.surfIdx];
        Material            material = materialss[isct.triInd.meshIdx][isct.triInd.surfIdx];
        MeshNormals const &     norms = normss[isct.triInd.meshIdx];
        Vec3UI              vis = tris.posInds[isct.triInd.triIdx];
        // TODO: Use perspective-correct normal and UV interpolation (makes very little difference for small tris):
        Vec3F               n0 = norms.vert[vis[0]],
                            n1 = norms.vert[vis[1]],
                            n2 = norms.vert[vis[2]],
                            bc = Vec3F(isct.barycentric),
                            norm = normalize(bc[0]*n0 + bc[1]*n1 + bc[2]*n2);
        RgbaF               albedo(230,230,230,255);
        Vec2Fs const &      uvs = *uvsPtrs[isct.triInd.meshIdx];
        Vec2F               uv {floatMax()};
        if ((!tris.uvInds.empty()) && (!uvs.empty()) && (material.albedoMap) &&
            (!material.albedoMap->empty()) && useMaps) {
            Vec3UI              uvInds = tris.uvInds[isct.triInd.triIdx];
            uv = bc[0]*uvs[uvInds[0]] + bc[1]*uvs[uvInds[1]] + bc[2]*uvs[uvInds[2]];
            uv[1] = 1.0f - uv[1];   // OTCS to IUCS
            albedo = RgbaF(sampleClipIucs(*material.albedoMap,uv));
        }
        Vec3F               acc(0.0f);
	    float	            aw = albedo.alpha() / 255.0f;
        Vec3F               surfColour = Vec3F(cHead<3>(albedo.m_c)) * aw;
        for (size_t ll=0; ll<lighting.lights.size(); ++ll) {
            Light               lgt = lighting.lights[ll];
            float               fac = cDot(norm,lgt.direction);
            if (fac > 0.0f) {
                acc += mapMul(surfColour,lgt.colour) * fac;
                float           shininess = material.shiny ? 1.0f : 0.0f;
                if ((uv[0] != floatMax()) && material.specularMap && !material.specularMap->empty()) {
                    RgbaF           s = sampleClipIucs(*material.specularMap,uv);
                    shininess = scast<float>(s.red()) / 255.0f;
                }
                if (allShiny)
                    shininess = 1.0f;
                if (shininess > 0.0f) {
                    Vec3F           reflectDir = norm * fac * 2.0f - lgt.direction;
                    if (reflectDir[2] > 0.0f) {
                        float       deltaSqr = sqr(reflectDir[0]) + sqr(reflectDir[1]),
                                    val = exp(-deltaSqr * 32.0f);
                        acc += Vec3F(255.0f * val);
                    }
                }
            }
        }
        acc += mapMul(surfColour,lighting.ambient);
        RgbaF           isctColor = RgbaF(acc[0],acc[1],acc[2],albedo.alpha());
        color = compositeFragment(isctColor,color);
     }
     return color;
}

Vec3F
RayCaster::oecsToIucs(Vec3F posOecs) const
{
    // Inverse depth value (map to -1 for points behind camera (OECS)):
    double          id = (posOecs[2] == 0.0f) ? -1.0 : (-1.0 / posOecs[2]);
    Vec2D           itcs(posOecs[0]*id,-posOecs[1]*id),     // Both Y and Z change sign in OECS -> ITCS
                    iucs = itcsToIucs * itcs;
    return Vec3F(iucs[0],iucs[1],id);
}

BestN<float,RayCaster::Intersect,4>
RayCaster::closestIntersects(Vec2F posIucs) const
{
    TriInds const &     triInds = grid[posIucs];
    BestN<float,Intersect,4> best;
    for (TriInd ti : triInds) {
        Tris const &        tris = trisss[ti.meshIdx][ti.surfIdx];
        Vec3UI              vis = tris.posInds[ti.triIdx];
        Vec3Fs const &      iucsVerts = iucsVertss[ti.meshIdx];
        Vec3F               v0 = iucsVerts[vis[0]],
                            v1 = iucsVerts[vis[1]],
                            v2 = iucsVerts[vis[2]];
        Vec2D               u0(v0[0],v0[1]),
                            u1(v1[0],v1[1]),
                            u2(v2[0],v2[1]);
        // TODO: make a float version of fgBarycentriCoords:
        Opt<Vec3D>          bco = cBarycentricCoord(Vec2D(posIucs),u0,u1,u2);
        if (bco.valid()) {       // TODO: filter out degenerate projected tris during cache setup
            Vec3D               bc = bco.val();
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

}

// */
