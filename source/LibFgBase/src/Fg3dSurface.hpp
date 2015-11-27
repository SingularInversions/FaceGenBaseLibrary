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

struct  Fg3dTopology;

struct  FgVertLabel
{
    FgVect3F    vert;
    string      label;
};

struct FgSurfPoint
{
    uint            triEquivIdx;
    FgVect3F        weights;
    string          label;      // Optional

    FG_SERIALIZE3(triEquivIdx,weights,label)

    FgSurfPoint() {};
    FgSurfPoint(uint t,const FgVect3F & w) : triEquivIdx(t), weights(w) {}

    bool
    operator==(const string & rhs) const
    {return (label == rhs); }
};

template<uint dim>
struct  FgFacetInds
{
    typedef FgMatrixC<uint,dim,1>   Ind;
    vector<Ind>                vertInds;   // CC winding
    vector<Ind>                uvInds;     // size() == 0 || vertInds.size()
    FG_SERIALIZE2(vertInds,uvInds)

    FgFacetInds() {}
    FgFacetInds(const vector<Ind> & v, const vector<Ind> & u)
    : vertInds(v), uvInds(u)
    {FGASSERT(valid()); }

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

struct  Fg3dSurface
{
    FgString                    name;
    FgString                    material;
    FgFacetInds<3>              tris;
    FgFacetInds<4>              quads;
    vector<FgSurfPoint>         surfPoints;
    FG_SERIALIZE5(name,material,tris,quads,surfPoints)

    Fg3dSurface() {}
    Fg3dSurface(
        const vector<FgVect4UI> &  quads,
        const vector<FgVect4UI> &  texs);
    explicit
    Fg3dSurface(
        const vector<FgVect3UI> & tris,
        const vector<FgVect4UI> & quads = vector<FgVect4UI>(),
        const vector<FgVect3UI> & tris_uvinds = vector<FgVect3UI>(),
        const vector<FgVect4UI> & quads_uvinds = vector<FgVect4UI>(),
        const vector<FgSurfPoint> & surfPoints = vector<FgSurfPoint>());

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

    uint
    numSurfPoints() const
    {return uint(surfPoints.size()); };

    template<class T>
    FgMatrixC<T,3,1>
    getSurfPoint(const vector<FgMatrixC<T,3,1> > & verts,size_t idx) const
    {
        FgVect3UI  vertInds = getTriEquiv(surfPoints[idx].triEquivIdx);
        FgVect3F   vertWeights = surfPoints[idx].weights;
        return (verts[vertInds[0]] * static_cast<T>(vertWeights[0]) +
                verts[vertInds[1]] * static_cast<T>(vertWeights[1]) +
                verts[vertInds[2]] * static_cast<T>(vertWeights[2]));
    }

    vector<FgVertLabel>
    surfPointsAsVertLabels(const FgVerts &) const;

    Fg3dSurface
    convertToTris() const;

    Fg3dSurface
    subdivideFlat(FgVerts & verts) const;

    Fg3dSurface
    subdivideLoop(const FgVerts & vertsIn,FgVerts & vertsOut) const;

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

private:
    void
    subdivideTris(
        const Fg3dTopology &        topo,
        uint                        newVertsBaseIdx,
        vector<FgVect3UI> &    tris,           // RETURNED
        vector<FgSurfPoint> &  surfPoints)     // RETURNED
        const;

    void
    checkInternalConsistency();
};

std::ostream& operator<<(std::ostream&,const Fg3dSurface&);

// Return a surface with only the selected tris (no quads), and any surface points that remain valid:
Fg3dSurface
fgSelectTris(const Fg3dSurface & surf,const vector<FgBool> & sel);

Fg3dSurface
fgRemoveDuplicateFacets(const Fg3dSurface &);

#endif
