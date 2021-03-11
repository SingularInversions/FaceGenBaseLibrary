//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#ifndef FG3DSURFACE_HPP
#define FG3DSURFACE_HPP

#include "FgStdString.hpp"
#include "FgTypes.hpp"
#include "FgMatrixC.hpp"
#include "FgMatrixV.hpp"
#include "FgImage.hpp"

namespace Fg {

struct  LabelledVert
{
    Vec3F       pos;
    String      label;

    // Handy for lookup:
    bool operator==(String const & lab) const {return (label == lab); }
};
typedef Svec<LabelledVert>  LabelledVerts;

// Returns the vertices with the selected labels in that order:
Vec3Fs
selectVerts(const LabelledVerts & labVerts,Strings const & labels);

struct SurfPoint
{
    uint            triEquivIdx;
    Vec3F           weights;        // Barycentric coordinate of point in triangle
    String          label;          // Optional

    SurfPoint() {};
    SurfPoint(uint t,Vec3F const & w) : triEquivIdx(t), weights(w) {}

    bool
    operator==(String const & rhs) const
    {return (label == rhs); }
};

typedef Svec<SurfPoint>    SurfPoints;

inline
Vec3F
cBarycentricVert(Vec3UI inds,Vec3F weights,Vec3Fs const & verts)
{
    return
        verts[inds[0]] * weights[0] +
        verts[inds[1]] * weights[1] +
        verts[inds[2]] * weights[2];
}

inline
Vec2F
cBarycentricUv(Vec3UI inds,Vec3F weights,Vec2Fs const & uvs)
{
    return
        uvs[inds[0]] * weights[0] +
        uvs[inds[1]] * weights[1] +
        uvs[inds[2]] * weights[2];
}

template<uint dim>
struct  FacetInds
{
    typedef Mat<uint,dim,1>     Ind;
    Svec<Ind>                   posInds;    // CC winding
    Svec<Ind>                   uvInds;     // CC winding. Empty or same size as 'posInds'

    FacetInds() {}

    explicit
    FacetInds(const Svec<Ind> & vtInds) : posInds(vtInds) {}

    FacetInds(const Svec<Ind> & vtInds, const Svec<Ind> & uvIds) : posInds(vtInds), uvInds(uvIds) {}

    bool
    valid() const
    {return ((uvInds.size() == 0) || (uvInds.size() == posInds.size())); }

    size_t
    size() const
    {return posInds.size(); }

    bool
    empty() const
    {return posInds.empty(); }

    bool
    hasUvs() const
    {return (posInds.size() == uvInds.size()); }

    void
    erase(size_t idx)       // Erase a facet
    {
        FGASSERT(idx < posInds.size());
        if (!uvInds.empty()) {
            FGASSERT(uvInds.size() == posInds.size());
            uvInds.erase(uvInds.begin()+idx);
        }
        posInds.erase(posInds.begin()+idx);
    }

    void
    offsetIndices(size_t vertsOff,size_t uvsOff)
    {
        Ind     vo = Ind(uint(vertsOff)),
                uo = Ind(uint(uvsOff));
        for (size_t ii=0; ii<posInds.size(); ++ii)
            posInds[ii] += vo;
        for (size_t ii=0; ii<uvInds.size(); ++ii)
            uvInds[ii] += uo;
    }
};
typedef FacetInds<3>        Tris;
typedef FacetInds<4>        Quads;
typedef Svec<Tris>          Triss;
typedef Svec<Triss>         Trisss;

template<uint dim>
void
cat_(FacetInds<dim> & lhs,const FacetInds<dim> & rhs)
{
    // Avoid 'hasUvs()' compare if one is empty:
    if (rhs.empty())
        return;
    if (lhs.empty()) {
        lhs = rhs;
        return;
    }
    if (lhs.hasUvs() != rhs.hasUvs()) {
        fgout << fgnl << "WARNING: Merging surfaces with UVs and without. UVs discarded.";
        lhs.uvInds.clear();
    }
    else
        cat_(lhs.uvInds,rhs.uvInds);
    cat_(lhs.posInds,rhs.posInds);
}

Vec3UIs
quadsToTris(const Vec4UIs & quads);

struct      TriUv
{
    Vec3UI       posInds;
    Vec3UI       uvInds;
};

// the returned 'uvInds' is all zeros if there are no UVs:
TriUv
cTriEquiv(Tris const & tris,Quads const & quads,size_t tt);

Vec3UI
cTriEquivPosInds(Tris const & tris,Quads const & quads,size_t tt);

Tris
cTriEquivs(Tris const & tris,Quads const & quads);

Vec3F
cSurfPointPos(
    SurfPoint const &           sp,
    Tris const &                tris,
    Quads const &               quads,
    Vec3Fs const &              verts);

struct  Material
{
    bool                        shiny = false;  // Ignored if 'specularMap' below is non-empty
    Sptr<ImgC4UC>               albedoMap;      // Can be nullptr but should not be the empty image
    Sptr<ImgC4UC>               specularMap;    // TODO: Change to greyscale
};
typedef Svec<Material>          Materials;
typedef Svec<Materials>         Materialss;

// A grouping of facets (tri and quad) sharing material properties:
struct  Surf
{
    String8                     name;
    Tris                        tris;
    Quads                       quads;
    SurfPoints                  surfPoints;
    Material                    material;       // Not saved with mesh - set dynamically

    Surf() {}
    explicit Surf(Vec3UIs const & ts) : tris(ts) {}
    explicit Surf(const Vec4UIs & ts) : quads(ts) {}
    Surf(Vec3UIs const & triPosInds,Vec4UIs const & quadPosInds) : tris(triPosInds), quads(quadPosInds) {}
    Surf(const Svec<Vec4UI> & verts,const Svec<Vec4UI> & uvs) : quads(verts,uvs) {}
    Surf(String8 const & n,Tris const & ts,SurfPoints const & sps,Material const & m) :
        name(n), tris(ts), surfPoints(sps), material(m) {}

    bool
    empty() const
    {return (tris.empty() && quads.empty()); }

    uint
    numTris() const
    {return uint(tris.size()); }

    uint
    numQuads() const
    {return uint(quads.size()); }

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

    TriUv
    getTriEquiv(size_t idx) const
    {return cTriEquiv(tris,quads,idx); }

    Vec3UI
    getTriPosInds(uint ind) const
    {return tris.posInds[ind]; }

    Vec4UI
    getQuadPosInds(uint ind) const
    {return quads.posInds[ind]; }

    // Returns the vertex indices for the tri equiv:
    Vec3UI
    getTriEquivPosInds(size_t ind) const {return cTriEquivPosInds(tris,quads,ind); }

    Tris
    getTriEquivs() const {return cTriEquivs(tris,quads); }

    bool
    isTri(size_t triEquivIdx) const
    {return (triEquivIdx < tris.size()); }

    bool            hasUvIndices() const {return !(tris.uvInds.empty() && quads.uvInds.empty()); }
    Vec3F           surfPointPos(Vec3Fs const & verts,size_t idx) const {return cSurfPointPos(surfPoints[idx],tris,quads,verts); }
    // Label must correspond to a surface point:
    Vec3F           surfPointPos(Vec3Fs const & verts,String const & label) const;
    Vec3Fs          surfPointPositions(Vec3Fs const & verts) const;
    LabelledVerts   surfPointsAsLabelledVerts(Vec3Fs const &) const;
    FacetInds<3>    asTris() const;
    Surf            convertToTris() const {return Surf {name,asTris(),surfPoints,material}; }
    void            merge(Surf const & surf);
    void            checkMeshConsistency(uint maxCoordIdx,uint maxUvIdx) const;

    // Return a surface with all indices offset by the given amounts:
    Surf
    offsetIndices(size_t vertsOffset,size_t uvsOffset) const
    {
        Surf ret(*this);
        ret.tris.offsetIndices(vertsOffset,uvsOffset);
        ret.quads.offsetIndices(vertsOffset,uvsOffset);
        return ret;
    }

    // Useful for searching by name:
    bool
    operator==(String8 const & str) const
    {return (name == str); }

    void
    setAlbedoMap(ImgC4UC const & img)
    {material.albedoMap = std::make_shared<ImgC4UC>(img); }

    ImgC4UC &
    albedoMapRef()
    {
        if (!material.albedoMap)
            material.albedoMap = std::make_shared<ImgC4UC>();
        return *material.albedoMap;
    }

    void
    removeTri(size_t triIdx);

    void
    removeQuad(size_t quadIdx);

private:
    void
    checkInternalConsistency();
};

void    fgReadp(std::istream &,Surf &);
void    fgWritep(std::ostream &,Surf const &);

std::ostream& operator<<(std::ostream&,Surf const&);

typedef Svec<Surf>      Surfs;

Surf
removeDuplicateFacets(Surf const &);

Surf
mergeSurfaces(const Surfs & surfs);

// Split a surface into its (one or more) discontiguous (by vertex index) surfaces:
Surfs
splitByContiguous(Surf const & surf);

// Name any unnamed surfaces as numbered extensions of the given base name,
// or just the base name if there is only a single (unnamed) surface:
Surfs
fgEnsureNamed(const Surfs & surfs,String8 const & baseName);

Vec3Fs
cVertsUsed(Vec3UIs const & tris,Vec3Fs const & verts);

bool
hasUnusedVerts(Vec3UIs const & tris,Vec3Fs const & verts);

Uints
removeUnusedVertsRemap(Vec3UIs const & tris,Vec3Fs const & verts);

struct  TriSurf
{
    Vec3Fs          verts;
    Vec3UIs         tris;

    bool
    hasUnusedVerts() const
    {return Fg::hasUnusedVerts(tris,verts); }
};

Vec3UIs
reverseWinding(Vec3UIs const &);

TriSurf
reverseWinding(TriSurf const &);

TriSurf
meshRemoveUnusedVerts(Vec3Fs const & verts,Vec3UIs const & tris);

inline
TriSurf
meshRemoveUnusedVerts(TriSurf const & ts)
{return meshRemoveUnusedVerts(ts.verts,ts.tris); }

struct      TriSurfFids
{
    TriSurf         surf;
    Vec3Fs          fids;   // When it's handy to have fiducial / landmark points explicitly separate from surface
};

typedef Svec<TriSurfFids>   TriSurfFidss;

struct      QuadSurf
{
    Vec3Fs          verts;
    Vec4UIs         quads;
};

}

#endif
