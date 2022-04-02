//
// Coypright (c) 2022 Singular Inversions Inc. (facegen.com)
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

template<typename T,typename F,FG_ENABLE_IF(F,is_floating_point)>
T                   interpolate(        // barycentric coordinate interpolation
    Vec3UI              tri,            // triangle vertex indices into 'vals'
    Mat<F,3,1> const &  baryCoord,      // respective weights should sum to 1 (barycentric); not checked
    Svec<T> const &     vals)           // values to be interpolated
{
    return vals[tri[0]]*baryCoord[0] + vals[tri[1]]*baryCoord[1] + vals[tri[2]]*baryCoord[2];
}

// index into 'tris', then beyond that each quad is implicitly 2 tris; [012] and [230]
Vec3UI              getTriEquivalent(size_t triEquivIdx,Vec3UIs const & tris,Vec4UIs const & quads);

struct  BaryPoint
{
    uint            triEquivIdx;    // index into tris then quads where each quad counts as 2 tris
    Vec3F           weights;        // barycentric coordinate of point in triangle

    BaryPoint() {}
    BaryPoint(uint t,Vec3F const & w) : triEquivIdx(t), weights(w) {}

    Vec3F           pos(Vec3UIs const & tris,Vec4UIs const & quads,Vec3Fs const & verts) const;
    Vec3F           pos(Vec3UIs const & tris,Vec3Fs const & verts) const {return pos(tris,{},verts); }
};
typedef Svec<BaryPoint>     BaryPoints;

struct  SurfPoint
{
    BaryPoint       point;
    // for left/right surface points on saggitally symmetric meshes, this label should end in
    // the character L or R, which will be automatically swapped if the mesh is mirrored:
    String          label;          // Can be empty

    SurfPoint() {}
    SurfPoint(BaryPoint const & p,String const & l) : point{p}, label{l} {}
    SurfPoint(uint t,Vec3F const & w) : point{t,w} {}
    SurfPoint(uint t,Vec3F const & w,String const & l) : point{t,w}, label{l} {}

    bool        operator==(String const & rhs) const {return (label == rhs); }
};
typedef Svec<SurfPoint>     SurfPoints;

NameVec3Fs          toNameVecs(
    SurfPoints const &  sps,
    Vec3UIs const &     tris,
    Vec4UIs const &     quads,
    Vec3Fs const &      verts);

template<uint N>
struct  NPolys
{
    typedef Mat<uint,N,1>       Ind;        // CC winding unless otherwise specified
    Svec<Ind>                   vertInds;
    Svec<Ind>                   uvInds;     // must be empty or same size as 'vertInds'

    NPolys() {}
    explicit NPolys(Svec<Ind> const & vs) : vertInds (vs) {}
    NPolys(Svec<Ind> const & vtInds, const Svec<Ind> & uvIds) : vertInds(vtInds), uvInds(uvIds) {}

    bool            valid() const {return ((uvInds.size() == 0) || (uvInds.size() == vertInds.size())); }
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
typedef NPolys<3>           Tris;
typedef NPolys<4>           Quads;          // treated as 2 tris for many operations; i0-i1-i2 and i2-i3-i0
typedef Svec<Tris>          Triss;
typedef Svec<Triss>         Trisss;

// UVs must be discarded if one of the arguments has position indices but no UV indices:
template<uint N>
NPolys<N>       concat(NPolys<N> const & l,NPolys<N> const & r)
{
    // UVs must be discarded if both do not have, to preserve validity:
    if (l.hasUvs() && r.hasUvs())
        return NPolys<N>{cat(l.vertInds,r.vertInds),cat(l.uvInds,r.uvInds)};
    else
        return NPolys<N>{cat(l.vertInds,r.vertInds)};
}
Vec3UIs         asTris(Vec4UIs const & quads);      // [i0,i1,i2,i3] -> [i0,i1,i2],[i2,i3,i0]
inline Tris     asTris(Quads const & quads) {return Tris{asTris(quads.vertInds),asTris(quads.uvInds)}; }

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

struct  Material
{
    bool                        shiny = false;  // Ignored if 'specularMap' below is non-empty
    Sptr<ImgRgba8>              albedoMap;      // Can be nullptr but should not be the empty image
    Sptr<ImgRgba8>              specularMap;    // TODO: Change to greyscale
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
    Surf(String8 const & n,Tris const & ts,SurfPoints const & sps,Material const & m)
        : name(n), tris(ts), surfPoints(sps), material(m) {}
    Surf(Tris const & t,Quads const & q) : tris{t}, quads{q} {}
    Surf(String8 const & n,Tris const & t,Quads const & q) : name(n), tris(t), quads(q) {}
    Surf(String8 const & n,Tris const & t,Quads const & q,SurfPoints const & s,Material const & m)
        : name(n), tris(t), quads(q), surfPoints(s), material(m) {}

    bool            empty() const {return (tris.empty() && quads.empty()); }
    uint            numTris() const {return uint(tris.size()); }
    uint            numQuads() const {return uint(quads.size()); }
    uint            numPolys() const {return (numTris() + numQuads()); }
    uint            numTriEquivs() const {return numTris() + 2*numQuads(); }
    uint            vertIdxMax() const;
    std::set<uint>  vertsUsed() const;
    Vec3UI          getTriEquivUvInds(size_t idx) const {return getTriEquivalent(idx,tris.uvInds,quads.uvInds); }
    Vec3UI          getTriEquivVertInds(size_t idx) const {return getTriEquivalent(idx,tris.vertInds,quads.vertInds); }
    Tris            getTriEquivs() const {return concat(tris,Fg::asTris(quads)); }
    bool            hasUvIndices() const {return !(tris.uvInds.empty() && quads.uvInds.empty()); }
    Vec3F           surfPointPos(Vec3Fs const & verts,BaryPoint const & bp) const;
    Vec3F           surfPointPos(Vec3Fs const & verts,size_t surfPointIdx) const;
    Vec3F           surfPointPos(Vec3Fs const & verts,String const & label) const;
    Vec3Fs          surfPointPositions(Vec3Fs const & verts) const;
    NameVec3Fs      surfPointsAsNameVecs(Vec3Fs const & verts) const
                    {return toNameVecs(surfPoints,tris.vertInds,quads.vertInds,verts); }
    Surf            convertToTris() const {return Surf {name,getTriEquivs(),surfPoints,material}; }
    void            merge(Tris const &,Quads const &,SurfPoints const &);
    void            merge(Surf const & s) {merge(s.tris,s.quads,s.surfPoints); }
    void            checkMeshConsistency(uint maxCoordIdx,uint maxUvIdx) const;
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

void            fgReadp(std::istream &,Surf &);
void            fgWritep(std::ostream &,Surf const &);

std::ostream& operator<<(std::ostream&,Surf const&);

typedef Svec<Surf>      Surfs;
typedef Svec<Surfs>     Surfss;

NameVec3Fs      surfPointsToNameVecs(Surfs const & surfs,Vec3Fs const & verts);
// Only preserves name and polygons. Splits into <name>-## surfaces for each occupied UV domain and
// modifies the UVs to be in [0,1]. If domains are not used, just returns the input surface:
Surfs           splitByUvDomain_(Surf const & surf,Vec2Fs & uvs);
Surf            removeDuplicateFacets(Surf const &);
Surf            mergeSurfaces(Surfs const & surfs);     // Retains name & material of first surface
// Split a surface into its (one or more) discontiguous (by vertex index) surfaces:
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
BaryPoint       reverseWinding(BaryPoint const & bp,size_t numTris);    // update for reversed winding per above
SurfPoints      reverseWinding(SurfPoints const & sps,size_t numTris);  // "
Surf            reverseWinding(Surf const & surf);
// Returns alpha-mapped 1024 X 1024 wireframe image:
ImgRgba8        cUvWireframeImage(Vec2Fs const & uvs,Vec3UIs const & tris,Vec4UIs const & quads,Rgba8 wireColor);

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

struct  MeshNormals
{
    FacetNormalss       facet;       // Facet normals for each surface
    Vec3Fs              vert;        // Vertex normals.
};
typedef Svec<MeshNormals>       MeshNormalss;

// CC winding. Norm is 0 for degenerate tris:
template<typename T>
Mat<T,3,1>      cTriNorm(Vec3UI const & tri,Svec<Mat<T,3,1>> const & verts)
{
    Mat<T,3,1>          v0 = verts[tri[0]],
                        v1 = verts[tri[1]],
                        v2 = verts[tri[2]],
                        cross = crossProduct(v1-v0,v2-v0);      // CC winding
    double              mag = cMag(cross);
    return (mag == 0.0) ? Mat<T,3,1>{0} : cross * (1.0 / sqrt(mag));
}
Vec3F           cQuadNorm(Vec4UI const & quad,Vec3Fs const & verts);    // least squares surface fit normal
Vec3Ds          cVertNorms(Vec3Ds const & verts,Vec3UIs const & tris);

struct  TriSurf
{
    Vec3Fs          verts;
    Vec3UIs         tris;

    bool            hasUnusedVerts() const {return Fg::hasUnusedVerts(tris,verts); }
};
struct      TriSurfD
{
    Vec3Ds                  shape;
    Vec3UIs                 tris;
};

inline TriSurf      reverseWinding(TriSurf const & ts) {return {ts.verts,reverseWinding(ts.tris)}; }
TriSurf             removeUnusedVerts(Vec3Fs const & verts,Vec3UIs const & tris);
inline TriSurf      removeUnusedVerts(TriSurf const & ts) {return removeUnusedVerts(ts.verts,ts.tris); }
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
