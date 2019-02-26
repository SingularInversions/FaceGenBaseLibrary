//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Feb 28, 2017
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

// Per-vertex morph (aka blendshape) represented by deltas from base shape:
struct  FgMorph
{
    FgString            name;
    FgVerts             verts;      // 1-1 correspondence with base verts

    FgMorph() {}
    FgMorph(const FgString & n,const FgVerts & v)
        : name(n), verts(v) {}

    void
    applyAsDelta(FgVerts & accVerts,float val) const
    {
        FGASSERT(verts.size() == accVerts.size());
        for (size_t ii=0; ii<verts.size(); ++ii)
            accVerts[ii] += verts[ii] * val;
    }
};

void    fgReadp(std::istream &,FgMorph &);
void    fgWritep(std::ostream &,const FgMorph &);

typedef     vector<FgMorph>     FgMorphs;

// Indexed vertices morph (aka blendshape) represented by absolute position:
struct  FgIndexedMorph
{
    FgString            name;
    FgUints             baseInds;   // Indices of base vertices to be morphed.
    // Can represent target position or delta depending on type of morph.
    // Must be same size() as baseInds.:
    FgVerts             verts;

    void
    applyAsTarget_(const FgVerts & baseVerts,const FgVerts & targVerts,size_t & idxTarg,float val,FgVerts & accVerts) const
    {
        for (size_t ii=0; ii<baseInds.size(); ++ii) {
            uint        idx = baseInds[ii];
            FgVect3F    del = targVerts[idxTarg+ii] - baseVerts[idx];
            accVerts[idx] += del * val;
        }
        idxTarg += baseInds.size();
    }

    void
    applyAsTarget_(const FgVerts & baseVerts,float val,FgVerts & accVerts) const
    {
        size_t      idx = 0;
        applyAsTarget_(baseVerts,verts,idx,val,accVerts);
    }

    // Name of morph does not affect equality:
    bool
    operator==(const FgIndexedMorph & rhs) const
    {return ((baseInds == rhs.baseInds) && (verts == rhs.verts)); }
};

void    fgReadp(std::istream &,FgIndexedMorph &);
void    fgWritep(std::ostream &,const FgIndexedMorph &);

typedef vector<FgIndexedMorph>  FgIndexedMorphs;

inline
size_t
fgSumVerts(const FgIndexedMorphs & ims)
{
    size_t      ret = 0;
    for (size_t ii=0; ii<ims.size(); ++ii)
        ret += ims[ii].verts.size();
    return ret;
}

void
fgAccDeltaMorphs(
    const vector<FgMorph> &     deltaMorphs,
    const FgFlts &              coord,
    FgVerts &                   accVerts);  // MODIFIED: morphing delta accumualted here

// This version of target morph application is more suited to SSM dataflow, where the
// target positions have been transformed as part of the 'allVerts' array:
void
fgAccTargetMorphs(
    const FgVerts &             allVerts,   // Base verts plus all target morph verts
    const vector<FgIndexedMorph> & targMorphs,  // Only 'baseInds' is used.
    const FgFlts &              coord,      // morph coefficient for each target morph
    FgVerts &                   accVerts);  // MODIFIED: target morphing delta accumulated here

// Current implementation supports at most 3 skin weights per vertex.
struct  FgSkinWgt
{
    FgVect3UI       skinInds;       // Corresponding to below
    FgVect3F        skinWgts;       // Non-zero values should be at beginning
};

typedef vector<FgSkinWgt>       FgSkinWgts;

// We use skin-based poses only for rotations, eg. eyes, eyelids, jaw, neck:
struct  FgSkinRot
{
    FgString            name;
    FgVect3F            axis;       // Normalized axis of rotation direction (RHR)
    FgVect3F            bone;       // Any point on the axis of rotation
    FgVect2F            bounds;     // Pose limit bounds (radians). Lower val non-zero for eg. eye movements.
    float               neutral;    // Neutral pose value (radians). Zero for eyes but non-zero for eg. eyelids.
};

typedef vector<FgSkinRot>       FgSkinRots;

struct  FgPose
{
    FgString            name;
    FgVect2F            bounds;
    float               neutral;

    FgPose() : bounds(FgVect2F(0,1)), neutral(0) {}

    // Provide an ordering so we can make a set. We do not differentiate based on bounds and neutral
    // value since this is too complicated in the rest of the code; client must beware not to
    // overload pose names:
    bool
    operator<(const FgPose & rhs) const
    {return (name < rhs.name); }

    // Allow for finding by name:
    bool
    operator==(const FgString & rhsName) const
    {return (name == rhsName); }
};

typedef vector<FgPose>          FgPoses;

inline
FgPoses
fgPoses(const FgMorphs & morphs)
{
    FgPoses         ret(morphs.size());
    for (size_t ii=0; ii<ret.size(); ++ii)
        ret[ii].name = morphs[ii].name;
    return ret;
}

inline
FgPoses
fgPoses(const FgIndexedMorphs & morphs)
{
    FgPoses         ret(morphs.size());
    for (size_t ii=0; ii<ret.size(); ++ii)
        ret[ii].name = morphs[ii].name;
    return ret;
}

// Accumulate deltas for delta morphs stored as a vertex array:
void
fgPoseDeltas(const std::map<FgString,float> & poseVals,const FgMorphs & deltaMorphs,FgVerts & acc);

// Accumulate deltas for target morphs stored as indexed vertex array:
void
fgPoseDeltas(const std::map<FgString,float> & poseVals,const FgIndexedMorphs & targMorphs,const FgVerts & indivShape,
    const FgVerts & targShape,FgVerts & acc);

#endif
