//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Jan 8, 2005
//

#ifndef FG3DSURFACE_HPP
#define FG3DSURFACE_HPP

#include "FgStdString.hpp"
#include "FgTypes.hpp"
#include "FgMatrix.hpp"
#include "FgImage.hpp"

struct  FgLabelledVert
{
    FgVect3F    pos;
    string      label;

    // Handy for lookup:
    bool operator==(const string & lab) const {return (label == lab); }
};

typedef vector<FgLabelledVert>  FgLabelledVerts;

struct FgSurfPoint
{
    uint            triEquivIdx;
    FgVect3F        weights;        // Barycentric coordinate of point in triangle
    string          label;          // Optional

    FgSurfPoint() {};
    FgSurfPoint(uint t,const FgVect3F & w) : triEquivIdx(t), weights(w) {}

    bool
    operator==(const string & rhs) const
    {return (label == rhs); }
};

void    fgReadp(std::istream &,FgSurfPoint &);
void    fgWritep(std::ostream &,const FgSurfPoint &);

typedef std::vector<FgSurfPoint>    FgSurfPoints;

inline
FgVect3F
fgBarycentricPos(FgVect3UI inds,FgVect3F weights,const FgVerts & verts)
{
    return
        verts[inds[0]] * weights[0] +
        verts[inds[1]] * weights[1] +
        verts[inds[2]] * weights[2];
}

template<uint dim>
struct  FgFacetInds
{
    typedef FgMatrixC<uint,dim,1>   Ind;
    vector<Ind>                vertInds;   // CC winding
    vector<Ind>                uvInds;     // size() == 0 || vertInds.size()

    FgFacetInds() {}

    explicit
    FgFacetInds(const vector<Ind> & vtInds) : vertInds(vtInds) {}

    FgFacetInds(const vector<Ind> & vtInds, const vector<Ind> & uvIds) : vertInds(vtInds), uvInds(uvIds) {}

    void
    offset(size_t vertsOff,size_t uvsOff)
    {
        Ind     vo = Ind(uint(vertsOff)),
                uo = Ind(uint(uvsOff));
        for (size_t ii=0; ii<vertInds.size(); ++ii)
            vertInds[ii] += vo;
        for (size_t ii=0; ii<uvInds.size(); ++ii)
            uvInds[ii] += uo;
    }

    bool
    valid() const
    {return ((uvInds.size() == 0) || (uvInds.size() == vertInds.size())); }

    bool
    empty() const
    {return vertInds.empty(); }

    bool
    hasUvs() const
    {return (vertInds.size() == uvInds.size()); }
};

template<uint dim>
void
fgReadp(std::istream & is,FgFacetInds<dim> & fi)
{
    fgReadp(is,fi.vertInds);
    fgReadp(is,fi.uvInds);
}

template<uint dim>
void
fgWritep(std::ostream & os,const FgFacetInds<dim> & fi)
{
    fgWritep(os,fi.vertInds);
    fgWritep(os,fi.uvInds);
}

template<uint dim>
void
fgAppend(FgFacetInds<dim> & lhs,const FgFacetInds<dim> & rhs)
{
    // Avoid 'hasUvs()' compare if one is empty:
    if (rhs.empty())
        return;
    if (lhs.empty()) {
        lhs = rhs;
        return;
    }
    fgAppend(lhs.vertInds,rhs.vertInds);
    if (lhs.hasUvs() != rhs.hasUvs()) {
        fgout << fgnl << "WARNING: Merging surfaces with UVs and without. UVs discarded.";
        lhs.uvInds.clear();
    }
    else
        fgAppend(lhs.uvInds,rhs.uvInds);
}

vector<FgVect3UI>
fgQuadsToTris(const vector<FgVect4UI> & quads);

// A grouping of facets (tri and quad) sharing material properties:
struct  Fg3dSurface
{
    FgString                    name;
    FgFacetInds<3>              tris;
    FgFacetInds<4>              quads;
    FgSurfPoints                surfPoints;
    boost::shared_ptr<FgImgRgbaUb> albedoMap;   // Can be Null but should not be empty. Not serialized.

    Fg3dSurface() {}

    explicit
    Fg3dSurface(const FgVect3UIs & ts) : tris(ts) {}

    explicit
    Fg3dSurface(const FgVect4UIs & ts) : quads(ts) {}

    Fg3dSurface(const vector<FgVect4UI> & verts,const vector<FgVect4UI> & uvs) : quads(verts,uvs) {}

    bool
    empty() const
    {return (tris.empty() && quads.empty()); }

    uint
    numTris() const
    {return uint(tris.vertInds.size()); }

    uint
    numQuads() const
    {return uint(quads.vertInds.size()); }

    uint
    numFacets() const
    {return (numTris() + numQuads()); }

    uint
    numTriEquivs() const
    {return numTris() + 2*numQuads(); }

    uint
    vertIdxMax() const;

    std::set<uint>
    vertsUsed() const;

    FgVect3UI
    getTri(uint ind) const
    {return tris.vertInds[ind]; }

    FgVect4UI
    getQuad(uint ind) const
    {return quads.vertInds[ind]; }

    // Returns the vertex indices for the tri equiv:
    FgVect3UI
    getTriEquiv(uint ind) const;

    FgFacetInds<3>
    getTriEquivs() const;

    bool
    hasUvIndices() const
    {return !(tris.uvInds.empty() && quads.uvInds.empty()); }

    FgVect3F
    surfPointPos(const FgVerts & verts,size_t idx) const;

    // Label must correspond to a surface point:
    FgVect3F
    surfPointPos(const FgVerts & verts,const string & label) const;

    FgLabelledVerts
    surfPointsAsVertLabels(const FgVerts &) const;

    FgVect3UIs
    asTrisOnly() const
    {return fgCat(tris.vertInds,fgQuadsToTris(quads.vertInds)); }

    Fg3dSurface
    convertToTris() const;

    void
    merge(const Fg3dSurface & surf);

    void
    checkMeshConsistency(uint maxCoordIdx,uint maxUvIdx);

    // Clears everything but debug
    void
    clear();

    // Return a surface with all indices offset by the given amounts:
    Fg3dSurface
    offset(size_t vertsOffset,size_t uvsOffset) const
    {
        Fg3dSurface ret(*this);
        ret.tris.offset(vertsOffset,uvsOffset);
        ret.quads.offset(vertsOffset,uvsOffset);
        return ret;
    }

    // Useful for fgFindFirstIdx:
    bool
    operator==(const FgString & str) const
    {return (name == str); }

    void
    setAlbedoMap(const FgImgRgbaUb & img)
    {albedoMap = boost::make_shared<FgImgRgbaUb>(img); }

    FgImgRgbaUb &
    albedoMapRef()
    {
        if (!albedoMap)
            albedoMap = boost::make_shared<FgImgRgbaUb>();
        return *albedoMap;
    }

    // Only implemented for tri-only surfaces:
    void
    removeTri(size_t triIdx);

private:
    void
    checkInternalConsistency();
};

void    fgReadp(std::istream &,Fg3dSurface &);
void    fgWritep(std::ostream &,const Fg3dSurface &);

std::ostream& operator<<(std::ostream&,const Fg3dSurface&);

typedef std::vector<Fg3dSurface>    Fg3dSurfaces;

// Return a surface with only the selected tris (no quads), and any surface points that remain valid:
Fg3dSurface
fgSelectTris(const Fg3dSurface & surf,const vector<FgBool> & sel);

Fg3dSurface
fgRemoveDuplicateFacets(const Fg3dSurface &);

Fg3dSurface
fgMergeSurfaces(const Fg3dSurfaces & surfs);

// Split a surface into its (one or more) discontiguous surfaces
vector<Fg3dSurface>
fgSplitSurface(const Fg3dSurface & surf);

// Name any unnamed surfaces as numbered extensions of the given base name,
// or just the base name if there is only a single (unnamed) surface:
Fg3dSurfaces
fgEnsureNamed(const Fg3dSurfaces & surfs,const FgString & baseName);

bool
fgHasUnusedVerts(const FgVect3UIs & tris,const FgVerts & verts);

FgUints
fgRemoveUnusedVertsRemap(const FgVect3UIs & tris,const FgVerts & verts);

// Handy for very simple surfaces:
struct  FgTriSurf
{
    FgVerts             verts;
    FgVect3UIs          tris;

    bool
    hasUnusedVerts() const
    {return fgHasUnusedVerts(tris,verts); }
};

FgTriSurf
fgRemoveUnusedVerts(const FgVerts & verts,const FgVect3UIs & tris);

inline
FgTriSurf
fgRemoveUnusedVerts(const FgTriSurf & ts)
{return fgRemoveUnusedVerts(ts.verts,ts.tris); }

#endif
