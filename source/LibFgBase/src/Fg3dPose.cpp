//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     April 22, 2005
//

#include "stdafx.h"

#include "Fg3dPose.hpp"
#include "FgException.hpp"
#include "FgStdStream.hpp"
#include "FgBounds.hpp"
#include "FgMath.hpp"
#include "FgStdSet.hpp"

using namespace std;

void
fgReadp(std::istream & is,FgMorph & m)
{
    fgReadp(is,m.name);
    fgReadp(is,m.verts);
}

void
fgWritep(std::ostream & os,const FgMorph & m)
{
    fgWritep(os,m.name);
    fgWritep(os,m.verts);
}

void
fgReadp(std::istream & is,FgIndexedMorph & m)
{
    fgReadp(is,m.name);
    fgReadp(is,m.baseInds);
    fgReadp(is,m.verts);
}

void
fgWritep(std::ostream & os,const FgIndexedMorph & m)
{
    fgWritep(os,m.name);
    fgWritep(os,m.baseInds);
    fgWritep(os,m.verts);
}

void
fgAccDeltaMorphs(
    const vector<FgMorph> &     deltaMorphs,
    const FgFlts &              coord,
    FgVerts &                   accVerts)
{
    FGASSERT(deltaMorphs.size() == coord.size());
    for (size_t ii=0; ii<deltaMorphs.size(); ++ii) {
        const FgMorph &     morph = deltaMorphs[ii];
        FGASSERT(morph.verts.size() == accVerts.size());
        for (size_t jj=0; jj<accVerts.size(); ++jj)
            accVerts[jj] += morph.verts[jj] * coord[ii];
    }
}

void
fgAccTargetMorphs(
    const FgVerts &             allVerts,
    const vector<FgIndexedMorph> & targMorphs,
    const FgFlts &              coord,
    FgVerts &                   accVerts)
{
    FGASSERT(targMorphs.size() == coord.size());
    size_t          numTargVerts = 0;
    for (size_t ii=0; ii<targMorphs.size(); ++ii)
        numTargVerts += targMorphs[ii].baseInds.size();
    FGASSERT(accVerts.size() + numTargVerts == allVerts.size());
    size_t          idx = accVerts.size();
    for (size_t ii=0; ii<targMorphs.size(); ++ii) {
        const FgUints &     inds = targMorphs[ii].baseInds;
        for (size_t jj=0; jj<inds.size(); ++jj) {
            size_t          baseIdx = inds[jj];
            FgVect3F        del = allVerts[idx++] - allVerts[baseIdx];
            accVerts[baseIdx] += del * coord[ii];
        }
    }
}

void
fgPoseDeltas(const std::map<FgString,float> & poseVals,const FgMorphs & deltaMorphs,FgVerts & acc)
{
    for (size_t ii=0; ii<deltaMorphs.size(); ++ii) {
        const FgMorph &     morph = deltaMorphs[ii];
        std::map<FgString,float>::const_iterator it = poseVals.find(morph.name);
        if (it != poseVals.end())
            morph.applyAsDelta(acc,it->second);
    }
}

void
fgPoseDeltas(const std::map<FgString,float> & poseVals,const FgIndexedMorphs & targMorphs,const FgVerts & indivShape,
    const FgVerts & targShape,FgVerts & acc)
{
    size_t      idx = 0;
    for (size_t ii=0; ii<targMorphs.size(); ++ii) {
        const FgIndexedMorph &  morph = targMorphs[ii];
        std::map<FgString,float>::const_iterator  it = poseVals.find(morph.name);
        if (it != poseVals.end())
            morph.applyAsTarget_(indivShape,targShape,idx,it->second,acc);
    }
}

// */
