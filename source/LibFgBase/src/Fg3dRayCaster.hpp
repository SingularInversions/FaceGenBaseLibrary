//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     April 29, 2010
//

#ifndef FG3DRAYCASTER_HPP
#define FG3DRAYCASTER_HPP

#include "FgSampler.hpp"
#include "Fg3dNormals.hpp"
#include "FgGridTriangles.hpp"
#include "FgBestN.hpp"
#include "FgAffineCwC.hpp"

typedef boost::function<FgRgbaF(FgVect3F,FgVect2F,FgMaterial,const FgImgRgbaUb *)>   FgFuncShader;

struct  FgSurfPtr
{
    FgMaterial                  material;
    const FgVerts *             verts;
    const vector<FgVect3UI> *   vertInds;
    const Fg3dNormals *         norms;
    const vector<FgVect2F> *    uvs;        // Can be NULL
    const vector<FgVect3UI> *   uvInds;     // Can be NULL
    const FgImgRgbaUb *         texImg;     // Can be NULL
};

struct  FgSurfRay
{
    FgSurfPtr                   surf;
    FgGridTriangles             grid;       // IUCS
    vector<float>               depth;      // CCS Z
    vector<FgVect3F>            norms;
    FgVect2Fs                   vertsIucs;

    FgSurfRay() {}
    FgSurfRay(
        FgSurfPtr               rs,
        FgAffine3F              modelview,
        FgAffineCw2F            itcsToIucs);

    FgBestN<float,FgTriPoint,8>
    cast(FgVect2F posIucs) const;

    FgRgbaF
    shade(
        FgFuncShader            shader,
        const FgTriPoint &      intersect) const;
};

struct  Fg3dRayCaster
{
    vector<FgSurfRay>           m_surfs;
    FgFuncShader                m_shader;
    FgRgbaF                     m_background;

    Fg3dRayCaster(
        vector<FgSurfPtr>       rs,
        FgFuncShader            shader,
        FgAffine3F              modelview,
        FgAffineCw2F            itcsToIucs,
        FgRgbaF                 background);

    virtual FgRgbaF
    operator()(FgVect2F posIucs) const;

    FgRgbaF
    cast(FgVect2F p) const
    {return this->operator()(p); }

    struct Best
    {
        size_t                  surfIdx;
        FgTriPoint              intersect;

        Best() {}

        Best(size_t s,FgTriPoint i)
        : surfIdx(s), intersect(i)
        {}
    };
};

#endif

// */
