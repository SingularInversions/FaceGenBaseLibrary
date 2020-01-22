//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
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
fgWritep(std::ostream & os,Morph const & m)
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
macAsTargetMorph_(
    Vec3Fs const &  baseVerts,
    Uints const &   indices,
    Vec3F const *   targVertsPtr,
    float           val,
    Vec3Fs &        accVerts)
{
    FGASSERT(baseVerts.size() == accVerts.size());
    for (size_t ii=0; ii<indices.size(); ++ii) {
        size_t      idx = indices[ii];
        Vec3F       del = targVertsPtr[ii] - baseVerts.at(idx);
        accVerts[idx] += del * val;
    }
}

void
fgAccDeltaMorphs(
    const vector<Morph> &     deltaMorphs,
    const Floats &              coord,
    Vec3Fs &                   accVerts)
{
    FGASSERT(deltaMorphs.size() == coord.size());
    for (size_t ii=0; ii<deltaMorphs.size(); ++ii) {
        Morph const &     morph = deltaMorphs[ii];
        FGASSERT(morph.verts.size() == accVerts.size());
        for (size_t jj=0; jj<accVerts.size(); ++jj)
            accVerts[jj] += morph.verts[jj] * coord[ii];
    }
}

void
fgAccTargetMorphs(
    Vec3Fs const &             allVerts,
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
accPoseDeltas_(const std::map<Ustring,float> & poseVals,const Morphs & deltaMorphs,Vec3Fs & acc)
{
    for (size_t ii=0; ii<deltaMorphs.size(); ++ii) {
        Morph const &     morph = deltaMorphs[ii];
        std::map<Ustring,float>::const_iterator it = poseVals.find(morph.name);
        if (it != poseVals.end())
            morph.accAsDelta_(it->second,acc);
    }
}

void
accPoseDeltas_(
    map<Ustring,float> const &      poseVals,
    IndexedMorphs const &           targMorphs,
    Vec3Fs const &                  baseShape,
    Vec3Fs const &                  targShapes,
    Vec3Fs &                        acc)
{
    size_t      idx = 0;
    for (IndexedMorph const & morph : targMorphs) {
        FGASSERT(targShapes.size() >= idx+morph.baseInds.size());
        auto        it = poseVals.find(morph.name);
        if (it != poseVals.end())
            macAsTargetMorph_(baseShape,morph.baseInds,&targShapes[idx],it->second,acc);
        idx += morph.baseInds.size();
    }
}

}

// */
