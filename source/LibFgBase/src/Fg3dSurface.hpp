//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#ifndef FG3DSURFACE_HPP
#define FG3DSURFACE_HPP

#include "FgImage.hpp"

namespace Fg {

// interpolate a 3D barycentric coordinate of arbitrary type given 3 indices into an array of the type:
template<typename T,typename F,FG_ENABLE_IF(F,is_floating_point)>
inline T            indexInterp(        // barycentric coordinate interpolation
    Vec3UI              tri,            // triangle vertex indices into 'vals'
    Mat<F,3,1> const &  baryCoord,      // respective weights should sum to 1 (barycentric); not checked
    Svec<T> const &     vals)           // values to be interpolated
{
    return multAcc(mapIndex(tri.m,vals),baryCoord.m);
}

// index into 'tris', then beyond that each quad is implicitly 2 tris; [012] and [230]
Vec3UI              getTriEquivalent(size_t triEquivIdx,Vec3UIs const & tris,Vec4UIs const & quads);

struct  SurfPoint
{
    uint            triEquivIdx;    // index into tris then quads where each quad counts as 2 tris
    Vec3F           weights;        // barycentric coordinate of point in triangle (must sum to 1)
    FG_SER2(triEquivIdx,weights)

    SurfPoint() {}
    SurfPoint(uint t,Vec3F const & w) : triEquivIdx(t), weights(w) {}

    Vec3F           pos(Vec3UIs const & tris,Vec4UIs const & quads,Vec3Fs const & verts) const;
    Vec3F           pos(Vec3UIs const & tris,Vec3Fs const & verts) const {return pos(tris,{},verts); }
    Vec3D           pos(Vec3UIs const & tris,Vec4UIs const & quads,Vec3Ds const & verts) const;
    Vec3D           pos(Vec3UIs const & tris,Vec3Ds const & verts) const {return pos(tris,{},verts); }
};
typedef Svec<SurfPoint> SurfPoints;

struct  SurfPointName
{
    SurfPoint       point;
    // for left/right surface points on saggitally symmetric meshes, this label should end in
    // the character L or R, which will be automatically swapped if the mesh is mirrored:
    String          label;          // Can be empty
    FG_SER2(point,label)

    SurfPointName() {}
    SurfPointName(SurfPoint const & p,String const & l) : point{p}, label{l} {}
    SurfPointName(uint t,Vec3F const & w) : point{t,w} {}
    SurfPointName(uint t,Vec3F const & w,String const & l) : point{t,w}, label{l} {}

    bool        operator==(String const & rhs) const {return (label == rhs); }
};
typedef Svec<SurfPointName> SurfPointNames;

NameVec3Fs          toNameVecs(
    SurfPointNames const &  sps,
    Vec3UIs const &     tris,
    Vec4UIs const &     quads,
    Vec3Fs const &      verts);

// returns a mapping from index to (contiguous) group number, [0,N) where N is the number of groups,
// including the group of unused index values.
Uints               cContiguousMap(Vec3UIs const & triInds,Vec4UIs const & quadInds);

template<uint N>
struct  NPolys
{
    typedef Mat<uint,N,1>       Ind;        // CC winding unless otherwise specified
    Svec<Ind>                   vertInds;
    Svec<Ind>                   uvInds;     // must be empty or same size as 'vertInds'
    FG_SER2(vertInds,uvInds)
    FG_EQ_M2(NPolys,vertInds,uvInds)

    NPolys() {}
    explicit NPolys(Svec<Ind> const & vs) : vertInds (vs) {}
    NPolys(Svec<Ind> const & vtInds,Svec<Ind> const & uvIds) : vertInds(vtInds), uvInds(uvIds) {}

    void            reserve(size_t numFacets)
    {
        vertInds.reserve(numFacets);
        uvInds.reserve(numFacets);
    }

    void            validate(uint numVerts,uint numUvs) const       // throws description if not valid
    {
        if (!uvInds.empty() && (uvInds.size() != vertInds.size()))
            fgThrow(
                "UV indices list has a different size than vertex indices list",
                toStr(uvInds.size())+"!="+toStr(vertInds.size()));
        for (Ind inds : vertInds)
            for (uint idx : inds.m)
                if (idx >= numVerts)
                    fgThrow("vertex index exceeds vertex count",toStr(idx)+">="+toStr(numVerts));
        for (Ind inds : uvInds)
            for (uint idx : inds.m)
                if (idx >= numUvs)
                    fgThrow("UV index exceeds UV count",toStr(idx)+">="+toStr(numUvs));
    }
    bool            validSize() const {return ((uvInds.size() == 0) || (uvInds.size() == vertInds.size())); }
    size_t          size() const {return vertInds.size(); }
    bool            empty() const {return vertInds.empty(); }
    bool            hasUvs() const {return (vertInds.size() == uvInds.size()); }
    void            erase(size_t idx)       // Erase a facet
    {
        FGASSERT(idx < vertInds.size());
        if (!uvInds.empty()) {
            FGASSERT(uvInds.size() == vertInds.size());
            uvInds.erase(uvInds.begin()+idx);
        }
        vertInds.erase(vertInds.begin()+idx);
    }
    void            offsetIndices(size_t vertsOff,size_t uvsOff)
    {
        Ind     vo = Ind(uint(vertsOff)),
                uo = Ind(uint(uvsOff));
        for (size_t ii=0; ii<vertInds.size(); ++ii)
            vertInds[ii] += vo;
        for (size_t ii=0; ii<uvInds.size(); ++ii)
            uvInds[ii] += uo;
    }
};
typedef NPolys<3>           TriInds;
typedef NPolys<4>           QuadInds;   // treated as 2 tris for many operations; i0-i1-i2 and i2-i3-i0
typedef Svec<TriInds>       TriIndss;

// UVs must be discarded if one of the arguments has position indices but no UV indices (to preserve validity):
template<uint N>
void                merge_(NPolys<N> & l,NPolys<N> const & r)
{
    if (l.hasUvs() && r.hasUvs()) {
        cat_(l.vertInds,r.vertInds);
        cat_(l.uvInds,r.uvInds);
    }
    else {
        cat_(l.vertInds,r.vertInds);
        l.uvInds.clear();
    }
}
template<uint N>
inline NPolys<N>    merge(NPolys<N> l,NPolys<N> const & r)
{
    merge_(l,r); return l;
}
template<uint N>
NPolys<N>           merge(Svec<NPolys<N>> const & triIndss)
{
    NPolys<N>           ret;
    for (NPolys<N> const & triInds : triIndss)
        merge_(ret,triInds);
    return ret;
}
Vec3UIs         asTris(Vec4UIs const & quads);      // [i0,i1,i2,i3] -> [i0,i1,i2],[i2,i3,i0]
inline TriInds  asTris(QuadInds const & quads) {return TriInds{asTris(quads.vertInds),asTris(quads.uvInds)}; }

template<uint N>
NPolys<N>       offsetIndices(NPolys<N> const & polys,uint vtsOffset,uint uvsOffset)
{
    Mat<uint,N,1> const voff {vtsOffset},
                        uoff {uvsOffset};
    return {
        mapAdd(polys.vertInds,voff),
        mapAdd(polys.uvInds,uoff),
    };
}

// color pixels corresponding to each vertex on the given map with the given color:
void            markVertOnMap(
    Vec2Fs const &          uvs,                // must be in OECS [0,1)
    TriInds const &         triInds,
    size_t                  vertIdx,            // mark this vertex in 'triInds'
    Rgba8                   color,
    ImgRgba8 &              map_);              // the closest pixel is set to 'color'

struct  Material
{
    bool                        shiny = false;  // Ignored if 'specularMap' below is non-empty
    Sptr<ImgRgba8>              albedoMap;      // Can be nullptr but should not be the empty image
    Sptr<ImgRgba8>              specularMap;    // TODO: Change to greyscale
    Material() {}
    Material(bool s,Sptr<ImgRgba8> const & a) : shiny{s}, albedoMap{a} {}
};
typedef Svec<Material>          Materials;
typedef Svec<Materials>         Materialss;

// A grouping of facets (tri and quad) sharing material properties:
struct  Surf
{
    String8                     name;
    TriInds                     tris;
    QuadInds                    quads;
    SurfPointNames              surfPoints;
    Material                    material;       // Not saved with mesh - set dynamically
    FG_SER4(name,tris,quads,surfPoints)

    Surf() {}
    explicit Surf(String const & n) : name{n} {}
    explicit Surf(Vec3UIs const & t) : tris{t} {}                           // tris only, no UVs
    Surf(Vec3UIs const & t,Vec4UIs const & q) : tris{t}, quads{q} {}        // no UVs
    Surf(TriInds const & t,QuadInds const & q={},SurfPointNames const & s={},Material const & m={})
        : tris{t}, quads{q}, surfPoints{s}, material{m} {}
    Surf(String8 const & n,TriInds const & t,QuadInds const & q,SurfPointNames const & s={},Material const & m={})
        : name(n), tris(t), quads(q), surfPoints(s), material(m) {}

    void            validate(uint maxCoordIdx,uint maxUvIdx) const;
    bool            empty() const {return (tris.empty() && quads.empty()); }
    uint            numTris() const {return uint(tris.size()); }
    uint            numQuads() const {return uint(quads.size()); }
    uint            numPolys() const {return (numTris() + numQuads()); }
    uint            numTriEquivs() const {return numTris() + 2*numQuads(); }
    uint            vertIdxMax() const;
    std::set<uint>  vertsUsed() const;
    Vec3UI          getTriEquivUvInds(size_t idx) const {return getTriEquivalent(idx,tris.uvInds,quads.uvInds); }
    Vec3UI          getTriEquivVertInds(size_t idx) const {return getTriEquivalent(idx,tris.vertInds,quads.vertInds); }
    TriInds         getTriEquivs() const {return Fg::merge(tris,Fg::asTris(quads)); }
    bool            hasUvIndices() const {return !(tris.uvInds.empty() && quads.uvInds.empty()); }
    Vec3F           surfPointPos(Vec3Fs const & verts,SurfPoint const & sp) const;
    Vec3F           surfPointPos(Vec3Fs const & verts,size_t surfPointIdx) const;
    Vec3F           surfPointPos(Vec3Fs const & verts,String const & label) const;
    Vec3Fs          surfPointPositions(Vec3Fs const & verts) const;
    NameVec3Fs      surfPointsAsNameVecs(Vec3Fs const & verts) const
                    {return toNameVecs(surfPoints,tris.vertInds,quads.vertInds,verts); }
    Surf            convertToTris() const {return Surf {name,getTriEquivs(),QuadInds{},surfPoints,material}; }
    void            merge(TriInds const &,QuadInds const &,SurfPointNames const &);
    void            merge(Surf const & s) {merge(s.tris,s.quads,s.surfPoints); }
    // Return a surface with all indices offset by the given amounts:
    Surf            offsetIndices(size_t vertsOffset,size_t uvsOffset) const
    {
        Surf ret(*this);
        ret.tris.offsetIndices(vertsOffset,uvsOffset);
        ret.quads.offsetIndices(vertsOffset,uvsOffset);
        return ret;
    }
    // Useful for searching by name:
    bool            operator==(String8 const & str) const {return (name == str); }
    void            setAlbedoMap(ImgRgba8 const & img) {material.albedoMap = std::make_shared<ImgRgba8>(img); }
    ImgRgba8 &      albedoMapRef()
    {
        if (!material.albedoMap)
            material.albedoMap = std::make_shared<ImgRgba8>();
        return *material.albedoMap;
    }
    void            removeTri(size_t triIdx);
    void            removeQuad(size_t quadIdx);
private:
    void            checkInternalConsistency();
};

std::ostream& operator<<(std::ostream&,Surf const&);

typedef Svec<Surf>      Surfs;
typedef Svec<Surfs>     Surfss;

Vec3UIs         asTriVertInds(Surf const &);
Vec3UIs         asTriVertInds(Surfs const &);
NameVec3Fs      surfPointsToNameVecs(Surfs const & surfs,Vec3Fs const & verts);
Vec3D           surfPointPos(Surfs const & surfs,Vec3Ds const & verts,String const & name);
Vec3Ds          surfPointPoss(Surfs const & surfs,Vec3Ds const & verts,Strings const & names);
// Only preserves name and polygons. Splits into <name>-## surfaces for each occupied UV domain and
// modifies the UVs to be in [0,1]. If UV tiles are not used, just returns the input surface:
Surfs           splitByUvTile_(Surf const & surf,Vec2Fs & uvs);
Surfs           splitSurfContiguousUvs(Surf const &);       // surf points not yet working properly
Surfs           splitSurfContiguousVerts(Surf const &);     // "
Surf            removeDuplicateFacets(Surf const &);
Surf            merge(Surfs const & surfs);     // Retains name & material of first surface
Surfs           splitByContiguous(Surf const & surf);
Vec3Fs          cVertsUsed(Vec3UIs const & tris,Vec3Fs const & verts);
Vec3Ds          cVertsUsed(Vec3UIs const & tris,Vec3Ds const & verts);
bool            hasUnusedVerts(Vec3UIs const & tris,Vec3Fs const & verts);
// Returned array is 1-1 with 'verts' and contains the new index value if the vert is used,
// or uint::max otherwise. Vertex ordering is preserved:
Uints           removeUnusedVertsRemap(Vec3UIs const & tris,Vec3Fs const & verts);
// note that this necessarily impacts any barycentric coordinates for the tri:
Vec3UIs         reverseWinding(Vec3UIs const & tris);       // [0,1,2] -> [0,2,1]
// reverse quad winding in such a way that the default triangulation (0123 -> 012,230) remains the same.
// note that this necessarily changes the tri-equivalent ordering as well as barycentric coordinates:
Vec4UIs         reverseWinding(Vec4UIs const & quads);      // [0,1,2,3] -> [0,3,2,1]
template<uint N>
NPolys<N>       reverseWinding(NPolys<N> const & ps) {return {reverseWinding(ps.vertInds),reverseWinding(ps.uvInds)}; }
SurfPoint       reverseWinding(SurfPoint const & bp,size_t numTris);    // update for reversed winding per above
SurfPointNames  reverseWinding(SurfPointNames const & sps,size_t numTris);  // "
Surf            reverseWinding(Surf const & surf);
// Returns alpha-mapped 1024 X 1024 wireframe image:
ImgRgba8        cUvWireframeImage(Vec2Fs const & uvs,Vec3UIs const & tris,Vec4UIs const & quads,Rgba8 wireColor);

struct      SurfsPoint
{
    size_t              surfIdx;
    SurfPoint           surfPoint;
};

SurfsPoint          findSurfsPoint(Surfs const & surfs,String const & name);
Vec3F               getSurfsPointPos(SurfsPoint sp,Surfs const & surfs,Vec3Fs const & verts);

struct  FacetNormals
{
    Vec3Fs              tri;        // Tri facet normals for a surface
    Vec3Fs              quad;       // Quad facet normals for a surface

    Vec3F               triEquiv(size_t idx) const
    {
        if (idx < tri.size())
            return tri[idx];
        idx -= tri.size();
        return quad[idx/2];
    }
};
typedef Svec<FacetNormals>      FacetNormalss;

// Returns normalized tri surface normal (CC winding), or {0,0,0} for degenerate tris:
template<class T>
Mat<T,3,1>      cTriNorm(Vec3UI const & tri,Svec<Mat<T,3,1>> const & verts)
{
    Arr<Mat<T,3,1>,3>   vs = mapIndex(tri.m,verts);
    Mat<T,3,1>          cross = crossProduct(vs[1]-vs[0],vs[2]-vs[0]);
    T                   ssv = cross.ssv();
    return (ssv == 0) ? Mat<T,3,1>{0} : cross / sqrt(ssv);
}
template<class T>
Svec<Mat<T,3,1>>    cTriNorms(Vec3UIs const & tris,Svec<Mat<T,3,1>> const & verts)
{
    auto                fn = [&verts](Vec3UI tri){return cTriNorm(tri,verts); };
    return mapCallT<Mat<T,3,1>>(tris,fn);
}
Vec3F           cQuadNorm(Vec4UI const & quad,Vec3Fs const & verts);    // least squares surface fit normal
Vec3Ds          cVertNorms(Vec3Ds const & verts,Vec3UIs const & tris);
struct      TriNorms
{
    Vec3Fs          faceNorms;      // one for each tri face. {0,0,0} if degenerate.
    Vec3Fs          vertNorms;      // one for each vertex. {0,0,0} if degenerate.
};
TriNorms            cTriNorms(Vec3UIs const & triInds,Vec3Ds const & verts);

struct  TriSurf
{
    Vec3Fs          verts;
    Vec3UIs         tris;

    bool            hasUnusedVerts() const {return Fg::hasUnusedVerts(tris,verts); }
};
typedef Svec<TriSurf>   TriSurfs;

struct      TriSurfD
{
    Vec3Ds                  shape;
    Vec3UIs                 tris;
};
typedef Svec<TriSurfD>  TriSurfDs;

inline TriSurf      reverseWinding(TriSurf const & ts) {return {ts.verts,reverseWinding(ts.tris)}; }
TriSurf             removeUnused(Vec3Fs const & verts,Vec3UIs const & tris);
inline TriSurf      removeUnused(TriSurf const & ts) {return removeUnused(ts.verts,ts.tris); }

struct      MeshNormals
{
    FacetNormalss       facet;       // Facet normals for each surface
    Vec3Fs              vert;        // Vertex normals.
};
typedef Svec<MeshNormals>       MeshNormalss;

MeshNormals         cNormals(Surfs const & surfs,Vec3Fs const & verts);

struct      TriSurfLms
{
    TriSurf         surf;
    Vec3Fs          landmarks;
};
typedef Svec<TriSurfLms>   TriSurfLmss;

struct      QuadSurf
{
    Vec3Fs          verts;
    Vec4UIs         quads;
};

}

#endif
