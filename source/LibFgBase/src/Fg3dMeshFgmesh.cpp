//
// Copyright (c) 2025 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "Fg3dMeshIo.hpp"
#include "FgFileSystem.hpp"
#include "FgSerial.hpp"
#include "FgCommand.hpp"
#include "FgImageIo.hpp"
#include "FgScopeGuard.hpp"
#include "FgTestUtils.hpp"

using namespace std;

namespace Fg {

namespace {

// This is a public file format so we can't use serialization (in case a struct changes) and instead
// have the ser/deser function hierarchy mirror the format. Also necessary because this format uses
// 32-bit size_t wheras FG serialization uses 64-bit.
// This approach also allows for more optimization as each function knows the size and structure of
// it's children. Not a big factor in practice though, SSD access is quite variable and sometimes takes
// much longer than the deseralization:

size_t              dsrSizet(Bytes const & data,size_t & idx)
{
        uint32              sz;
        dsrlzRaw_(data,idx,sz);
        return scast<size_t>(sz);
}
template<class T,class C>
void                dsrSvec_(Bytes const & data,size_t & idx,Svec<T> & ret,C const & fn)
{
    ret.resize(dsrSizet(data,idx));
    for (T & e : ret)
        fn(data,idx,e);
}
void                dsrStr_(Bytes const & data,size_t & idx,String & ret)
{
    size_t              S = dsrSizet(data,idx);
    FGASSERT(data.size()-idx>=S);
    ret.resize(S);
    memcpy(&ret[0],&data[idx],S);
    idx += S;
}
inline void         dsrStr_(Bytes const & data,size_t & idx,String8 & ret) {dsrStr_(data,idx,ret.m_str); }
template<size_t D>
void                dsrArrs_(Bytes const & data,size_t & idx,Svec<Arr<uint,D>> & ret)
{
    size_t              N = dsrSizet(data,idx);
    size_t constexpr    S = sizeof(uint) * D;
    FGASSERT(data.size()-idx >= N*S);
    ret.resize(N);
    // we can't read in all verts in one go since there's no guarantee the compiler hasn't added padding
    // to the Arr<> struct:
    for (size_t nn=0; nn<N; ++nn)
        memcpy(&ret[nn][0],&data[idx+nn*S],S);
    idx += N*S;
}
template<size_t D>
void                dsrVecs_(Bytes const & data,size_t & idx,Svec<Mat<float,D,1>> & ret)
{
    size_t              N = dsrSizet(data,idx);
    size_t constexpr    S = sizeof(float) * D;
    FGASSERT(data.size()-idx >= N * S);
    ret.resize(N);
    // we can't read in all verts in one go since there's no guarantee the compiler hasn't added padding
    // to the Mat<> struct:
    for (size_t nn=0; nn<N; ++nn)
        memcpy(&ret[nn].m[0],&data[idx+nn*S],S);
    idx += N*S;
}

template<size_t N>
void                dsrNPolys_(Bytes const & data,size_t & idx,NPolys<N> & ret)
{
    dsrArrs_(data,idx,ret.vertInds);
    dsrArrs_(data,idx,ret.uvInds);
}
void                dsrSP_(Bytes const & data,size_t & idx,SurfPoint & ret)
{
    dsrlz_(data,idx,ret.triEquivIdx);
    dsrlz_(data,idx,ret.weights);
}
void                dsrSPN_(Bytes const & data,size_t & idx,SurfPointName & ret)
{
    dsrSP_(data,idx,ret.point);
    dsrStr_(data,idx,ret.label);
}
void                dsrSurf_(Bytes const & data,size_t & idx,Surf & ret)
{
    dsrStr_(data,idx,ret.name);
    dsrNPolys_(data,idx,ret.tris);
    dsrNPolys_(data,idx,ret.quads);
    dsrSvec_(data,idx,ret.surfPoints,dsrSPN_);
}
void                dsrDM_(Bytes const & data,size_t & idx,DirectMorph & ret)
{
    dsrStr_(data,idx,ret.name);
    dsrVecs_(data,idx,ret.verts);
}
void                dsrIV_(Bytes const & data,size_t & idx,IdxVec3F & ret) {dsrlz_(data,idx,ret); }
void                dsrIM_(Bytes const & data,size_t & idx,IndexedMorph & ret)
{
    dsrStr_(data,idx,ret.name);
    dsrSvec_(data,idx,ret.ivs,dsrIV_);
}
void                dsrMV_(Bytes const & data,size_t & idx,MarkedVert & ret)
{
    dsrlz_(data,idx,ret.idx);
    dsrStr_(data,idx,ret.label);
}
void                dsrSW_(Bytes const & data,size_t & idx,SkinWeight & ret) {dsrlz_(data,idx,ret); }
void                dsrJoint_(Bytes const & data,size_t & idx,Joint & ret)
{
    dsrStr_(data,idx,ret.name);
    dsrlz_(data,idx,ret.parentIdx);
    dsrlz_(data,idx,ret.pos);
    dsrSvec_(data,idx,ret.skin,dsrSW_);
}

}

Mesh                loadFgmesh(String8 const & fname)
{
    Bytes               data = loadRaw(fname);
    if (data.size() < 16)
        fgThrow("Too short to be a valid .fgmesh file",fname);
    size_t              idx {0};
    String              header;
    dsrStr_(data,idx,header);
    if (cHead(header,6) != "FgMesh")
        fgThrow("Invalid header for .fgmesh file",fname);
    Mesh                ret;
    if (cRest(header,6) == "01") {
        try {
            dsrVecs_(data,idx,ret.verts);
            dsrVecs_(data,idx,ret.uvs);
            dsrSvec_(data,idx,ret.surfaces,dsrSurf_);
            dsrSvec_(data,idx,ret.deltaMorphs,dsrDM_);
            dsrSvec_(data,idx,ret.targetMorphs,dsrIM_);
            dsrSvec_(data,idx,ret.markedVerts,dsrMV_);
            if (idx<data.size())                 // ver 1.1
                dsrSvec_(data,idx,ret.joints,dsrJoint_);
        }
        catch (FgException & e) {e.contexts.emplace_back("invalid .fgmesh V01 file",fname.m_str); }
        catch (exception const & e) {fgThrow("invalid .fgmesh V01 file",fname.m_str,e.what()); }
    }
    else
        fgThrow("Unrecognized version of .fgmesh file, update to the latest version of this software",fname);
    return ret;
}

namespace {

void                srlSizet_(size_t sz,Bytes & data)
{
    FGASSERT(sz <= lims<uint32>::max());
    srlz_(scast<uint32>(sz),data);
}

template<class T,class C>
void                srlSvec_(Svec<T> const & obj,C const & fn,Bytes & data)
{
    srlSizet_(obj.size(),data);
    for (T const & e : obj)
        fn(e,data);
}
void                srlStr_(String const & obj,Bytes & data)
{
    size_t              S = obj.size();
    srlSizet_(S,data);
    if (S > 0) {
        size_t              B = data.size();
        data.resize(B+S);
        memcpy(&data[B],&obj[0],S);
    }
}
void                srlStr_(String8 const & obj,Bytes & data) {srlStr_(obj.m_str,data); }
template<size_t D>
void                srlArrs_(Svec<Arr<uint,D>> const & obj,Bytes & data)
{
    size_t constexpr    S = sizeof(uint) * D;
    size_t              N = obj.size();
    srlSizet_(N,data);
    // we can't write out all verts in one go since there's no guarantee the compiler hasn't added padding
    // to the Arr<> struct:
    size_t              B = data.size();
    data.resize(B+N*S);
    for (size_t nn=0; nn<N; ++nn)
        memcpy(&data[B+nn*S],&obj[nn][0],S);
}
template<size_t D>
void                srlVecs_(Svec<Mat<float,D,1>> const & obj,Bytes & data)
{
    size_t constexpr    S = sizeof(float) * D;
    size_t              N = obj.size();
    srlSizet_(N,data);
    // we can't write out all verts in one go since there's no guarantee the compiler hasn't added padding
    // to the Arr<> struct:
    size_t              B = data.size();
    data.resize(B+N*S);
    for (size_t nn=0; nn<N; ++nn)
        memcpy(&data[B+nn*S],&obj[nn].m[0],S);
}

template<size_t N>
void                srlNPolys_(NPolys<N> const & obj,Bytes & data)
{
    srlArrs_(obj.vertInds,data);
    srlArrs_(obj.uvInds,data);
}
void                srlSP_(SurfPoint const & obj,Bytes & data)
{
    srlz_(obj.triEquivIdx,data);
    srlz_(obj.weights,data);
}
void                srlSPN_(SurfPointName const & obj,Bytes & data)
{
    srlSP_(obj.point,data);
    srlStr_(obj.label,data);
}
void                srlSurf_(Surf const & obj,Bytes & data)
{
    srlStr_(obj.name,data);
    srlNPolys_(obj.tris,data);
    srlNPolys_(obj.quads,data);
    srlSvec_(obj.surfPoints,srlSPN_,data);
}
void                srlDM_(DirectMorph const & obj,Bytes & data)
{
    srlStr_(obj.name,data);
    srlVecs_(obj.verts,data);
}
void                srlIV_(IdxVec3F const & obj,Bytes & data) {srlz_(obj,data); }
void                srlIM_(IndexedMorph const & obj,Bytes & data)
{
    srlStr_(obj.name,data);
    srlSvec_(obj.ivs,srlIV_,data);
}
void                srlMV_(MarkedVert const & obj,Bytes & data)
{
    srlz_(obj.idx,data);
    srlStr_(obj.label,data);
}
void                srlSW_(SkinWeight const & obj,Bytes & data) {srlz_(obj,data); }
void                srlJoint_(Joint const & obj,Bytes & data)
{
    srlStr_(obj.name,data);
    srlz_(obj.parentIdx,data);
    srlz_(obj.pos,data);
    srlSvec_(obj.skin,srlSW_,data);
}

}

void                saveFgmesh(String8 const & fname,Mesh const & mesh)
{
    Bytes               data;
    srlStr_(String{"FgMesh01"},data);
    srlVecs_(mesh.verts,data);
    srlVecs_(mesh.uvs,data);
    srlSvec_(mesh.surfaces,srlSurf_,data);
    srlSvec_(mesh.deltaMorphs,srlDM_,data);
    srlSvec_(mesh.targetMorphs,srlIM_,data);
    srlSvec_(mesh.markedVerts,srlMV_,data);
    srlSvec_(mesh.joints,srlJoint_,data);
    saveRaw(data,fname,false);
}

}
