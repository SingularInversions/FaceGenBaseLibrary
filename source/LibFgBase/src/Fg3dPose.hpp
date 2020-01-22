//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#ifndef FG3DPOSE_HPP
#define FG3DPOSE_HPP

#include "FgStdLibs.hpp"

#include "FgStdVector.hpp"
#include "FgStdString.hpp"
#include "FgMatrixC.hpp"
#include "FgMatrixV.hpp"
#include "Fg3dSurface.hpp"
#include "FgImage.hpp"
#include "FgStdStream.hpp"
#include "FgOpt.hpp"
#include "FgQuaternion.hpp"

namespace Fg {

// Per-vertex morph (aka blendshape) represented by deltas from base shape:
struct  Morph
{
    Ustring             name;
    Vec3Fs              verts;      // 1-1 correspondence with base verts

    Morph() {}
    Morph(Ustring const & n,Vec3Fs const & v)
        : name(n), verts(v) {}

    void
    accAsDelta_(float val,Vec3Fs & accVerts) const
    {
        FGASSERT(verts.size() == accVerts.size());
        for (size_t ii=0; ii<verts.size(); ++ii)
            accVerts[ii] += verts[ii] * val;
    }
};
typedef Svec<Morph>     Morphs;

void fgReadp(std::istream &,Morph &);
void fgWritep(std::ostream &,Morph const &);

void
macAsTargetMorph_(
    Vec3Fs const &  baseVerts,
    Uints const &   indices,        // Indices into 'baseVerts'
    Vec3F const *   targVertsPtr,   // Must point to a continguous array of at least 'indices.size()' elements
    float           val,
    Vec3Fs &        accVerts);      // Must be same size as 'baseVerts'

// Indexed vertices morph (aka blendshape) represented by absolute position:
struct  IndexedMorph
{
    Ustring             name;
    Uints               baseInds;   // Indices of base vertices to be morphed.
    // Can represent target position or delta depending on type of morph.
    // Must be same size() as baseInds:
    Vec3Fs              verts;

    void
    accAsTarget_(Vec3Fs const & baseVerts,float val,Vec3Fs & accVerts) const
    {macAsTargetMorph_(baseVerts,baseInds,verts.data(),val,accVerts); }

    // Name of morph does not affect equality:
    bool
    operator==(const IndexedMorph & rhs) const
    {return ((baseInds == rhs.baseInds) && (verts == rhs.verts)); }
};

void    fgReadp(std::istream &,IndexedMorph &);
void    fgWritep(std::ostream &,const IndexedMorph &);

typedef Svec<IndexedMorph>  IndexedMorphs;

inline
size_t
fgSumVerts(IndexedMorphs const & ims)
{
    size_t      ret = 0;
    for (size_t ii=0; ii<ims.size(); ++ii)
        ret += ims[ii].verts.size();
    return ret;
}

void
fgAccDeltaMorphs(
    const Svec<Morph> &     deltaMorphs,
    const Floats &              coord,
    Vec3Fs &                   accVerts);  // MODIFIED: morphing delta accumualted here

// This version of target morph application is more suited to SSM dataflow, where the
// target positions have been transformed as part of the 'allVerts' array:
void
fgAccTargetMorphs(
    Vec3Fs const &             allVerts,   // Base verts plus all target morph verts
    const Svec<IndexedMorph> & targMorphs,  // Only 'baseInds' is used.
    const Floats &              coord,      // morph coefficient for each target morph
    Vec3Fs &                   accVerts);  // MODIFIED: target morphing delta accumulated here

struct  PoseVal
{
    Ustring             name;
    Vec2F               bounds;
    float               neutral;

    PoseVal() : bounds(Vec2F(0,1)), neutral(0) {}

    // Provide an ordering so we can make a set. We do not differentiate based on bounds and neutral
    // value since this is too complicated in the rest of the code; client must beware not to
    // overload pose names:
    bool
    operator<(const PoseVal & rhs) const
    {return (name < rhs.name); }

    // Allow for finding by name:
    bool
    operator==(Ustring const & rhsName) const
    {return (name == rhsName); }
};
typedef Svec<PoseVal>          PoseVals;

inline
PoseVals
cPoseVals(const Morphs & morphs)
{
    PoseVals         ret(morphs.size());
    for (size_t ii=0; ii<ret.size(); ++ii)
        ret[ii].name = morphs[ii].name;
    return ret;
}

inline
PoseVals
cPoseVals(IndexedMorphs const & morphs)
{
    PoseVals         ret(morphs.size());
    for (size_t ii=0; ii<ret.size(); ++ii)
        ret[ii].name = morphs[ii].name;
    return ret;
}

// Accumulate deltas for delta morphs stored as a vertex array:
void
accPoseDeltas_(
    const std::map<Ustring,float> & poseVals,
    const Morphs &                  deltaMorphs,
    Vec3Fs &                        acc);

// Accumulate deltas for target morphs stored as indexed vertex array:
void
accPoseDeltas_(
    const std::map<Ustring,float> & poseVals,
    IndexedMorphs const &           targMorphs,     // Only used for names and indices; targets positions ignored
    Vec3Fs const &                  baseShape,
    // Target morph data in continguous order of 'targMorphs'
    // These are used instead of the ones stored in the 'targMorphs':
    Vec3Fs const &                  targShapes,
    Vec3Fs &                        acc);

}

#endif
