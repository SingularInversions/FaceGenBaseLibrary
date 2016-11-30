//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:		Andrew Beatty
// Created:		April 7, 2010
//

#include "stdafx.h"

#include "FgSoftRender.hpp"
#include "FgSampler.hpp"
#include "Fg3dMesh.hpp"
#include "FgAffineCwC.hpp"
#include "FgGridTriangles.hpp"
#include "Fg3dRayCaster.hpp"
#include "FgMath.hpp"
#include "FgTestUtils.hpp"
#include "Fg3dMeshIo.hpp"
#include "Fg3dCamera.hpp"
#include "FgTime.hpp"

using namespace std;

struct  ShaderBasic : FgShader
{
    FgLighting      m_lighting;

    ShaderBasic(const FgLighting & lighting)
    : m_lighting(lighting)
    {}

    virtual FgRgbaF
    operator()(
        FgVect3F            normOecs,
        FgVect2F            uvIucs,
        FgMaterial          material,
        const FgImgRgbaUb * img = NULL) const
    {
        FgVect3F        acc(0.0f);
        FgRgbaF         texSample = (img) ?
            FgRgbaF(fgLerpClip(*img,uvIucs)) :
            FgRgbaF(230.0f,230.0f,230.0f,255.0f);
		float	aw = texSample.alpha() / 255.0f;
        FgVect3F        surfColour = texSample.m_c.subMatrix<3,1>(0,0) * aw;
        for (size_t ll=0; ll<m_lighting.m_lights.size(); ++ll) {
            FgLight     lgt = m_lighting.m_lights[ll];
            float       fac = fgDot(normOecs,lgt.m_direction);
            if (fac > 0.0f) {
                acc += fgMapMul(surfColour,lgt.m_colour) * fac;
                if (material.shiny) {
                    FgVect3F    reflectDir = normOecs * fac * 2.0f - lgt.m_direction;
                    if (reflectDir[2] > 0.0f) {
                        float   deltaSqr = fgSqr(reflectDir[0]) + fgSqr(reflectDir[1]),
                                val = exp(-deltaSqr * 32.0f);
                        acc += FgVect3F(255.0f * val);
                    }
                }
            }
        }
        acc += fgMapMul(surfColour,m_lighting.m_ambient);
        return FgRgbaF(acc[0],acc[1],acc[2],texSample.alpha());
    }
};

FgImgRgbaUb
fgSoftRender(
    FgVect2UI                   pxSz,
    const vector<Fg3dMesh> &    meshes,
    const FgLighting &          light,
    FgAffine3D                  modelview,
    FgAffineCw2D                itcsToIucs,
    FgRgbaF                     backgroundColor,
    uint                        antiAliasBitDepth)
{
    FgImgRgbaUb     img(pxSz);
    FgVectF2        colorBounds = fgBounds(backgroundColor.m_c);
    FGASSERT((colorBounds[0] >= 0.0f) && (colorBounds[1] <= 255.0f));
    vector<Fg3dSurface>     surfs(meshes.size());
    vector<FgSurfPtr>       rendSurfs(meshes.size());
    vector<Fg3dNormals>     norms(meshes.size());
    for (size_t ii=0; ii<meshes.size(); ++ii) {
        const Fg3dMesh &    mesh = meshes[ii];
        FGASSERT(mesh.surfaces.size() == 1);
        fgCalcNormals(mesh.surfaces,mesh.verts,norms[ii]);
        surfs[ii] = mesh.surfaces[0].convertToTris();
        FgSurfPtr   &       rs = rendSurfs[ii];
        rs.material = mesh.material;
        rs.verts = &mesh.verts;
        rs.vertInds = &surfs[ii].tris.vertInds;
        rs.norms = &norms[ii];
        rs.uvs = &mesh.uvs;
        rs.uvInds = &surfs[ii].tris.uvInds;
        rs.texImg = (mesh.surfaces[0].albedoMap ? mesh.surfaces[0].albedoMap.get() : NULL);
    }
    ShaderBasic     shader(light);
    Fg3dRayCaster
        rc(
            rendSurfs,
            &shader,
            modelview,
            fgD2F(itcsToIucs),
            backgroundColor);
    fgSampler(rc,img,antiAliasBitDepth);
    return img;
}

// */
