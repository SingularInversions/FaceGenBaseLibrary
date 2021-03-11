//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
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

// Per-vertex morph, typically represented as deltas from base shape (aka blendshape):
struct  Morph
{
    String8             name;
    Vec3Fs              verts;      // 1-1 correspondence with base verts

    Morph() {}
    explicit Morph(String8 const & n) : name(n) {}
    Morph(String8 const & n,Vec3Fs const & v)
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

void
macAsTargetMorph_(
    Vec3Fs const &  baseVerts,
    Uints const &   indices,        // Indices into 'baseVerts'
    Vec3F const *   targVertsPtr,   // Must point to a continguous array of at least 'indices.size()' elements
    float           val,
    Vec3Fs &        accVerts);      // Must be same size as 'baseVerts'

// Indexed vertices morph representation is useful when only a small fraction of the base vertices
// are affected:
struct  IndexedMorph
{
    String8             name;
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

typedef Svec<IndexedMorph>  IndexedMorphs;

inline
size_t
cNumVerts(IndexedMorphs const & ims)
{
    size_t      ret = 0;
    for (IndexedMorph const & im : ims)
        ret += im.verts.size();
    return ret;
}

void
accDeltaMorphs(
    Morphs const &              deltaMorphs,
    Floats const &              coord,
    Vec3Fs &                    accVerts);  // MODIFIED: morphing delta accumualted here

// This version of target morph application is more suited to SSM dataflow, where the
// target positions have been transformed as part of the 'allVerts' array:
void
accTargetMorphs(
    Vec3Fs const &              allVerts,   // Base verts plus all target morph verts
    IndexedMorphs const &       targMorphs,  // Only 'baseInds' is used.
    Floats const &              coord,      // morph coefficient for each target morph
    Vec3Fs &                    accVerts);  // MODIFIED: target morphing delta accumulated here

struct  PanTilt
{
    uint                boneVertIdx;        // Vertex of rotation centre
    Vec3F               tilt;               // Normalized. Applied first.
    Vec3F               pan;                // Normalized. Applied second.
};

struct  PoseDef
{
    String8             name;
    Vec2F               bounds;
    float               neutral;

    PoseDef() : bounds(Vec2F(0,1)), neutral(0) {}

    // Provide an ordering so we can make a set. We do not differentiate based on bounds and neutral
    // value since this is too complicated in the rest of the code; client must beware not to
    // overload pose names:
    bool
    operator<(const PoseDef & rhs) const
    {return (name < rhs.name); }

    // Allow for finding by name:
    bool
    operator==(String8 const & rhsName) const
    {return (name == rhsName); }
};
typedef Svec<PoseDef>          PoseDefs;

inline
PoseDefs
cPoseDefs(Morphs const & morphs)
{
    PoseDefs         ret(morphs.size());
    for (size_t ii=0; ii<ret.size(); ++ii)
        ret[ii].name = morphs[ii].name;
    return ret;
}

inline
PoseDefs
cPoseDefs(IndexedMorphs const & morphs)
{
    PoseDefs         ret(morphs.size());
    for (size_t ii=0; ii<ret.size(); ++ii)
        ret[ii].name = morphs[ii].name;
    return ret;
}

// Accumulate deltas for delta morphs stored as a vertex array:
void
accPoseDeltas_(
    const std::map<String8,float> & poseVals,
    Morphs const &                  deltaMorphs,
    Vec3Fs &                        acc);

// Accumulate deltas for target morphs stored as indexed vertex array:
void
accPoseDeltas_(
    const std::map<String8,float> & poseVals,
    IndexedMorphs const &           targMorphs,     // Only used for names and indices; targets positions ignored
    Vec3Fs const &                  baseShape,
    // Target morph data in continguous order of 'targMorphs'
    // These are used instead of the ones stored in the 'targMorphs':
    Vec3Fs const &                  targShapes,
    Vec3Fs &                        acc);

}

#endif
