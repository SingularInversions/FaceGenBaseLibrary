//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "Fg3dPose.hpp"
#include "FgException.hpp"
#include "FgStdStream.hpp"
#include "FgBounds.hpp"
#include "FgMath.hpp"
#include "FgStdSet.hpp"

using namespace std;

namespace Fg {

void
fgReadp(std::istream & is,Morph & m)
{
    fgReadp(is,m.name);
    fgReadp(is,m.verts);
}

void
fgWritep(std::ostream & os,const Morph & m)
{
    fgWritep(os,m.name);
    fgWritep(os,m.verts);
}

void
fgReadp(std::istream & is,IndexedMorph & m)
{
    fgReadp(is,m.name);
    fgReadp(is,m.baseInds);
    fgReadp(is,m.verts);
}

void
fgWritep(std::ostream & os,const IndexedMorph & m)
{
    fgWritep(os,m.name);
    fgWritep(os,m.baseInds);
    fgWritep(os,m.verts);
}

void
fgAccDeltaMorphs(
    const vector<Morph> &     deltaMorphs,
    const Floats &              coord,
    Vec3Fs &                   accVerts)
{
    FGASSERT(deltaMorphs.size() == coord.size());
    for (size_t ii=0; ii<deltaMorphs.size(); ++ii) {
        const Morph &     morph = deltaMorphs[ii];
        FGASSERT(morph.verts.size() == accVerts.size());
        for (size_t jj=0; jj<accVerts.size(); ++jj)
            accVerts[jj] += morph.verts[jj] * coord[ii];
    }
}

void
fgAccTargetMorphs(
    const Vec3Fs &             allVerts,
    const vector<IndexedMorph> & targMorphs,
    const Floats &              coord,
    Vec3Fs &                   accVerts)
{
    FGASSERT(targMorphs.size() == coord.size());
    size_t          numTargVerts = 0;
    for (size_t ii=0; ii<targMorphs.size(); ++ii)
        numTargVerts += targMorphs[ii].baseInds.size();
    FGASSERT(accVerts.size() + numTargVerts == allVerts.size());
    size_t          idx = accVerts.size();
    for (size_t ii=0; ii<targMorphs.size(); ++ii) {
        const Uints &     inds = targMorphs[ii].baseInds;
        for (size_t jj=0; jj<inds.size(); ++jj) {
            size_t          baseIdx = inds[jj];
            Vec3F        del = allVerts[idx++] - allVerts[baseIdx];
            accVerts[baseIdx] += del * coord[ii];
        }
    }
}

void
fgPoseDeltas(const std::map<Ustring,float> & poseVals,const Morphs & deltaMorphs,Vec3Fs & acc)
{
    for (size_t ii=0; ii<deltaMorphs.size(); ++ii) {
        const Morph &     morph = deltaMorphs[ii];
        std::map<Ustring,float>::const_iterator it = poseVals.find(morph.name);
        if (it != poseVals.end())
            morph.applyAsDelta(acc,it->second);
    }
}

void
fgPoseDeltas(const std::map<Ustring,float> & poseVals,const IndexedMorphs & targMorphs,const Vec3Fs & indivShape,
    const Vec3Fs & targShape,Vec3Fs & acc)
{
    size_t      idx = 0;
    for (size_t ii=0; ii<targMorphs.size(); ++ii) {
        const IndexedMorph &  morph = targMorphs[ii];
        std::map<Ustring,float>::const_iterator  it = poseVals.find(morph.name);
        if (it != poseVals.end())
            morph.applyAsTarget_(indivShape,targShape,idx,it->second,acc);
    }
}

}

// */
