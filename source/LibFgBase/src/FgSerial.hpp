//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Very simple basic binary serialization to/from std::string that:
//
// * Is 32/64 bit build portable on LLP64 and LP64 architectures
// * Is NOT endian independent (all supported architectures are little-endian)
// * Assumes 'char' is 8-bit, 'short' is 16-bit, 'int' is 32-bit
// * Assumes IEEE 754 encoding for float and double
// * Uses default construction followed by value population
// * Does not handle pointers
// * Doesn't use boost::serialization which has drawbacks:
//   1. version updates are not forward compatible
//   2. portable binary serialization is not directly supported but relies on an example file
//   3. class or field name changes can break compatibility
//
// NOTES:
// 
// * Will NOT work for size_t due to its changing nature so handle any size_t members manually
// * manually writing out the builtin versions of each function is required since enable_if results in
//   strange ambiguous template errors with Bytes.

#ifndef FGSERIAL_HPP
#define FGSERIAL_HPP

#include "FgString.hpp"

namespace Fg {

// Concatenates the arguments into a message then computes a deterministic 64 bit hash of the message which:
// * Always gives the same result on any platform
// * Is not cryptographically secure
uint64          treeHash(Uint64s const & hashes);

Bytes               stringToBytes(String const &);
String              bytesToString(Bytes const &);

// SERIALIZE / DESERIALIZE TO REFLECTION TREE
// which can also be used for text & json serialization / deserialization

struct      RflMember
{
    String          name;
    std::any        object;
};
struct      RflStruct
{
    Svec<RflMember> members;
};
struct      RflArray
{
    Svec<std::any>  elems;
};

// classes redirect to member function 'cReflect' defined by FG_RFL macros:
template<class T>
inline std::any     toReflect(T const & strct) {return strct.cReflect(); }
// like json, represent all numbers by 'double':
inline std::any     toReflect(bool v) {return v; }
inline std::any     toReflect(uchar val) {return scast<double>(val); }
inline std::any     toReflect(int val) {return scast<double>(val); }
inline std::any     toReflect(uint val) {return scast<double>(val); }
inline std::any     toReflect(long val) {return scast<double>(val); }
inline std::any     toReflect(unsigned long val) {return scast<double>(val); }
inline std::any     toReflect(long long val) {return scast<double>(val); }
inline std::any     toReflect(unsigned long long val) {return scast<double>(val); }
inline std::any     toReflect(float val) {return scast<double>(val); }
inline std::any     toReflect(double val) {return scast<double>(val); }
inline std::any     toReflect(String const & v) {return v; }
inline std::any     toReflect(String8 const & v) {return v.m_str; }
template<class T> std::any toReflect(Svec<T> const &);        // declare for Arr<Svec<...>...> case
template<class T,size_t S>
std::any            toReflect(Arr<T,S> const & a)
{
    RflArray            ret; ret.elems.reserve(S);
    for (T const & e : a)
        ret.elems.push_back(toReflect(e));
    return ret;
}
template<class T>
std::any            toReflect(Svec<T> const & v)
{
    RflArray            ret; ret.elems.reserve(v.size());
    for (T const & e : v)
        ret.elems.push_back(toReflect(e));
    return ret;
}
template<> inline std::any  toReflect(Bytes const & b) {return bytesToString(b); }

String              reflectToText(std::any const & reflectTree);
template<class T>
String              srlzText(T const & v) {return reflectToText(toReflect(v)); }

#define FG_RFL_BEG                  std::any cReflect() const {RflStruct ret;
#define FG_RFL_M1(A)                ret.members.push_back(RflMember{#A,toReflect(A)});
#define FG_RFL_M2(A,B)              FG_RFL_M1(A) FG_RFL_M1(B)
#define FG_RFL_M4(A,B,C,D)          FG_RFL_M2(A,B) FG_RFL_M2(C,D)
#define FG_RFL_END                  return ret; }

#define FG_RFL_1(A)                 FG_RFL_BEG FG_RFL_M1(A) FG_RFL_END
#define FG_RFL_2(A,B)               FG_RFL_BEG FG_RFL_M2(A,B) FG_RFL_END
#define FG_RFL_3(A,B,C)             FG_RFL_BEG FG_RFL_M2(A,B) FG_RFL_M1(C) FG_RFL_END
#define FG_RFL_4(A,B,C,D)           FG_RFL_BEG FG_RFL_M4(A,B,C,D) FG_RFL_END
#define FG_RFL_5(A,B,C,D,E)         FG_RFL_BEG FG_RFL_M4(A,B,C,D) FG_RFL_M1(E) FG_RFL_END
#define FG_RFL_6(A,B,C,D,E,F)       FG_RFL_BEG FG_RFL_M4(A,B,C,D) FG_RFL_M2(E,F) FG_RFL_END
#define FG_RFL_7(A,B,C,D,E,F,G)     FG_RFL_BEG FG_RFL_M4(A,B,C,D) FG_RFL_M2(E,F) FG_RFL_M1(G) FG_RFL_END
#define FG_RFL_8(A,B,C,D,E,F,G,H)   FG_RFL_BEG FG_RFL_M4(A,B,C,D) FG_RFL_M4(E,F,G,H) FG_RFL_END

template<class T>
inline void         fromReflect_(std::any const & node,T & strct) {strct.setRflct(node); }
inline void         fromReflect_(std::any const & node,bool & val) {val = std::any_cast<bool>(node); }
inline void         fromReflect_(std::any const & node,uchar & val) {val = scast<uchar>(std::any_cast<double>(node)); }
inline void         fromReflect_(std::any const & node,int & val) {val = scast<int>(std::any_cast<double>(node)); }
inline void         fromReflect_(std::any const & node,uint & val) {val = scast<uint>(std::any_cast<double>(node)); }
inline void         fromReflect_(std::any const & node,long & val) {val = scast<long>(std::any_cast<double>(node)); }
inline void         fromReflect_(std::any const & node,unsigned long & val) {val = scast<unsigned long>(std::any_cast<double>(node)); }
inline void         fromReflect_(std::any const & node,long long & val) {val = scast<long long>(std::any_cast<double>(node)); }
inline void         fromReflect_(std::any const & node,unsigned long long & val) {val = scast<unsigned long long>(std::any_cast<double>(node)); }
inline void         fromReflect_(std::any const & node,float & val) {val = scast<float>(std::any_cast<double>(node)); }
inline void         fromReflect_(std::any const & node,double & val) {val = std::any_cast<double>(node); }
inline void         fromReflect_(std::any const & node,String & str) {str = std::any_cast<String>(node); }
inline void         fromReflect_(std::any const & node,String8 & str) {str = std::any_cast<String>(node); }
// forward declaration to handle Arr<Svec<...>>:
template<class T> void fromReflect_(std::any const & node,Svec<T> & val);
template<class T,size_t S>
void                fromReflect_(std::any const & node,Arr<T,S> & val)
{
    RflArray            arr = std::any_cast<RflArray>(node);
    FGASSERT(arr.elems.size() == S);
    for (size_t ii=0; ii<S; ++ii)
        fromReflect_(arr.elems[ii],val[ii]);
}
template<class T>
void                fromReflect_(std::any const & node,Svec<T> & val)
{
    RflArray            arr = std::any_cast<RflArray>(node);
    size_t              S = arr.elems.size();
    val.resize(S);
    for (size_t ii=0; ii<S; ++ii)
        fromReflect_(arr.elems[ii],val[ii]);
}
template<> inline void fromReflect_(std::any const & node,Bytes & val) {val = stringToBytes(std::any_cast<String>(node)); }

std::any            textToReflect(String const & txt);
template<class T>
T                   dsrlzText(String const & txt)
{
    T                   ret;
    fromReflect_(textToReflect(txt),ret);
    return ret;
}

#define FG_DRF_BEG                  void setRflct(std::any const & node) {RflStruct strct = std::any_cast<RflStruct>(node); size_t cnt {0};
#define FG_DRF_M1(A)                fromReflect_(strct.members[cnt++].object,A);
#define FG_DRF_M2(A,B)              FG_DRF_M1(A) FG_DRF_M1(B)
#define FG_DRF_M4(A,B,C,D)          FG_DRF_M2(A,B) FG_DRF_M2(C,D)
#define FG_DRF_END                  }

#define FG_DRF_1(A)                 FG_DRF_BEG FG_DRF_M1(A) FG_DRF_END
#define FG_DRF_2(A,B)               FG_DRF_BEG FG_DRF_M2(A,B) FG_DRF_END
#define FG_DRF_3(A,B,C)             FG_DRF_BEG FG_DRF_M2(A,B) FG_DRF_M1(C) FG_DRF_END
#define FG_DRF_4(A,B,C,D)           FG_DRF_BEG FG_DRF_M4(A,B,C,D) FG_DRF_END
#define FG_DRF_5(A,B,C,D,E)         FG_DRF_BEG FG_DRF_M4(A,B,C,D) FG_DRF_M1(E) FG_DRF_END
#define FG_DRF_6(A,B,C,D,E,F)       FG_DRF_BEG FG_DRF_M4(A,B,C,D) FG_DRF_M2(E,F) FG_DRF_END
#define FG_DRF_7(A,B,C,D,E,F,G)     FG_DRF_BEG FG_DRF_M4(A,B,C,D) FG_DRF_M2(E,F) FG_DRF_M1(G) FG_DRF_END
#define FG_DRF_8(A,B,C,D,E,F,G,H)   FG_DRF_BEG FG_DRF_M4(A,B,C,D) FG_DRF_M4(E,F,G,H) FG_DRF_END

// COMPARISONS:

enum struct Cmp { lt=-1, eq=0, gt=1 };

// Default for compound types is to redirect to member operation:
template<typename T,FG_ENABLE_IF(T,is_compound)>
inline Cmp          cmp(T const & l,T const & r) {return l.cmp(r); }

// Base cases include all scalars:
template<typename T,FG_ENABLE_IF(T,is_scalar)>
Cmp                 cmp(T l,T r) {return (l<r) ? Cmp::lt : ((r<l) ? Cmp::gt : Cmp::eq); }

#define FG_CMP_M1(T,A)                                                                      \
    bool cmp(T const & r) const {return cmp(A,r.A); }

#define FG_LTE_M1(T,A)                                                                                  \
    bool operator<(T const & r) const {return (A < r.A); }                                              \
    bool operator==(T const & r) const {return (A == r.A); }

#define FG_LTE_M2(T,A,B)                                                                                \
    bool operator<(T const & r) const {                                                                 \
        if (A < r.A) return true; else if (r.A < A) return false; \
        else return (B < r.B); } \
    bool operator==(T const & r) const {return ((A == r.A) && (B == r.B)); }

#define FG_LTE_M3(T,A,B,C)                                        \
    bool operator<(T const & r) const {                           \
        if (A < r.A) return true; else if (r.A < A) return false; \
        if (B < r.B) return true; else if (r.B < B) return false; \
        else return (C < r.C); }                                                                             \
    bool operator==(T const & r) const                                                                  \
        {return ((A == r.A) && (B == r.B) && (C == r.C)); }

#define FG_LTE_M4(T,A,B,C,D)                                                                           \
    bool operator<(T const & r) const {                                                                 \
        if (A < r.A) return true; else if (r.A < A) return false; \
        else if (B < r.B) return true; else if (r.B < B) return false; \
        else if (C < r.C) return true; else if (r.C < C) return false; \
        else return (D < r.D); }                                                                             \
    bool operator==(T const & r) const                                                                  \
        {return ((A == r.A) && (B == r.B) && (C == r.C) && (D == r.D)); }

// When only equality is needed.
// As far as I can tell, 'decltype' can't infer T in the signature so must be explicit:
#define FG_EQ_M1(T,A) bool operator==(T const & r) const {return (A == r.A); }
#define FG_EQ_M2(T,A,B) bool operator==(T const & r) const {return ((A == r.A) && (B == r.B)); }
#define FG_EQ_M3(T,A,B,C) bool operator==(T const & r) const {return ((A == r.A) && (B == r.B) && (C == r.C)); }
#define FG_EQ_M4(T,A,B,C,D) bool operator==(T const & r) const {return ((A == r.A) && (B == r.B) && (C == r.C) && (D == r.D)); }
#define FG_EQ_M5(T,A,B,C,D,E) bool operator==(T const & r) const {return ((A==r.A)&&(B==r.B)&&(C==r.C)&&(D==r.D)&&(E==r.E)); }

// BINARY SERIALIZATION / DESERIALIATION helper functions:

// determines whether size_t is serialized to uint32 or uint64. Default true (uint64). Not threadsafe.
extern bool         g_useSize64;        // Never change this without a ScopeGuard to ensure it's reset to default

// Raw binary serialization (just copy the bytes). T must be fundamental type:
template<class T>
void                srlzRaw_(T val,Bytes & ser)
{
    // S is small so an inline [compiler-unrolled] loop is faster than calling memcpy:
    size_t constexpr    S = sizeof(val);
    std::byte const     *vPtr = reinterpret_cast<std::byte const *>(&val);
    // do NOT call ser.reserve() here ... pathological slow-down results in MSVC
    for (size_t ii=0; ii<S; ++ii)                           
        ser.push_back(vPtr[ii]);
}
void                srlzSizet_(size_t val,Bytes & ser);
template<class T>
void                srlzRawOverwrite_(T val,Bytes & ser,size_t pos)  // random access overwrite version
{
    size_t constexpr    S = sizeof(val);
    FGASSERT(pos+S <= ser.size());
    std::byte const     *vPtr = reinterpret_cast<std::byte const *>(&val);
    for (size_t ii=0; ii<S; ++ii)
        ser[pos+ii] = vPtr[ii];
}
template<class T>
void                dsrlzRaw_(Bytes const & ser,size_t & pos,T & val)
{
    size_t constexpr    S = sizeof(val);
    if (pos+S > ser.size())
        fgThrow("deserialze past end of data for type",typeid(T).name());
    val = *reinterpret_cast<T const *>(&ser[pos]);
    pos += S;
}
void                dsrlzSizet_(Bytes const & ser,size_t & pos,size_t & val);

// BINARY SERIALIZATION:
// Default case redirects serialization to class member serialization, which has a different name
// to avoid confusing errors (ie. when global can't be resolved it tries to call itself but has
// wrong number of arguments):
template<class T>
void                srlz_(T const & v,Bytes & s) {v.srlzm_(s); }
// full specializations for builtins and string:
void                srlz_(bool v,Bytes & s);                                       // Store as uchar
inline void         srlz_(uchar v,Bytes & s) {srlzRaw_(v,s); }                     // Assume always 8bit
inline void         srlz_(int v,Bytes & s) {srlzRaw_(v,s); }                       // Assume always 32bit
inline void         srlz_(uint v,Bytes & s) {srlzRaw_(v,s); }                      // "
inline void         srlz_(long v,Bytes & s) {srlzRaw_(int64(v),s); }               // LP64 / LLP64 interop
inline void         srlz_(unsigned long v,Bytes & s) {srlzRaw_(uint64(v),s); }     // "
inline void         srlz_(long long v,Bytes & s) {srlzRaw_(v,s); }                 // Assume always 64bit
inline void         srlz_(unsigned long long v,Bytes & s) {srlzRaw_(v,s); }        // "
inline void         srlz_(float v,Bytes & s) {srlzRaw_(v,s); }                     // Assume always IEEE 754
inline void         srlz_(double v,Bytes & s) {srlzRaw_(v,s); }                    // "
void                srlz_(String const & v,Bytes & s);
// can't be handled by base case above since String8 is defined BEFORE serialization:
inline void         srlz_(String8 const & s,Bytes & b) {srlz_(s.m_str,b); }
// partial specializations for std::array and std::vector:
template<typename T> void srlz_(Svec<T> const &,Bytes &);      // declaration for Arr<Svec<...>...>
template<typename T,size_t S>
void                srlz_(Arr<T,S> const & v,Bytes & s)
{
    for (T const & e : v)
        srlz_(e,s);
}
template<typename T>
void                srlz_(Svec<T> const & v,Bytes & s)
{
    srlzSizet_(v.size(),s);
    for (T const & e : v)
        srlz_(e,s);
}
template<> inline void srlz_(Bytes const & v,Bytes & s)
{
    srlz_(uint64(v.size()),s);
    cat_(s,v);
}

// BINARY DESERIALIZATION:
template<class T>
void                dsrlz_(Bytes const & s,size_t & p,T & v) {v.dsrlzm_(s,p); }
// full specializations:
void                dsrlz_(Bytes const & s,size_t & p,bool & v);
inline void         dsrlz_(Bytes const & s,size_t & p,uchar & v) {dsrlzRaw_(s,p,v); }
inline void         dsrlz_(Bytes const & s,size_t & p,int & v) {dsrlzRaw_(s,p,v); }
inline void         dsrlz_(Bytes const & s,size_t & p,uint & v) {dsrlzRaw_(s,p,v); }
void                dsrlz_(Bytes const & s,size_t & p,long & v);                   // interop w/ bounds checks
void                dsrlz_(Bytes const & s,size_t & p,unsigned long & v);          // "
inline void         dsrlz_(Bytes const & s,size_t & p,int64 & v) {dsrlzRaw_(s,p,v); }
inline void         dsrlz_(Bytes const & s,size_t & p,uint64 & v) {dsrlzRaw_(s,p,v); }
inline void         dsrlz_(Bytes const & s,size_t & p,float & v) {dsrlzRaw_(s,p,v); }
inline void         dsrlz_(Bytes const & s,size_t & p,double & v) {dsrlzRaw_(s,p,v); }
void                dsrlz_(Bytes const & s,size_t & p,String & v);
// can't be handled by base case above since String8 is defined BEFORE serialization:
inline void         dsrlz_(Bytes const & b,size_t & p,String8 & s) {dsrlz_(b,p,s.m_str); }
// partial specializations for std::array and std::vector:
template<typename T> void dsrlz_(Bytes const &,size_t &,Svec<T> &);    // declare for Arr<Svec<...>...>
template<typename T,size_t S>
void                dsrlz_(Bytes const & s,size_t & p,Arr<T,S> & v)
{
    for (T & e : v)
        dsrlz_(s,p,e);
}
template<typename T>
void                dsrlz_(Bytes const & s,size_t & p,Svec<T> & v)
{
    size_t              sz;
    dsrlzSizet_(s,p,sz);
    v.resize(sz);
    for (T & e : v)
        dsrlz_(s,p,e);
}
template<> inline void dsrlz_(Bytes const & s,size_t & p,Bytes & v)
{
    uint64              sz;
    dsrlz_(s,p,sz);
    v = cSubvec(s,p,sz);
    p += sz;
}
template<class T> T dsrlzT_(Bytes const & bytes,size_t & pos)
{
    T                   ret;
    dsrlz_(bytes,pos,ret);
    return ret;
}

// The type signature is uint64 value defined only on simple types, which is unique
// (modulo hash collisions) to a given ordered hierarchy of basic types and independent
// of member names and class/structure names. Using this signature ensures safe retrieval
// of serialized data after code modifications; The signature is unaffected by name changes
// but any change in ordering, number or type of structure members wil give a different
// signature (as it will also serialize differently).
//
// Implementation requires a dummy struct since the function takes no arguments, only a
// type template, and thus cannot be overloaded, and function templates do not support partial
// specialization which means we cannot support standard library partial template specializations:

// Base case handles struct with macro-defined static function 'typeSig()':
template<class T> struct TypeSig    {static uint64 typeSig() {return T::typeSig(); } };
// Builtin type (full) specializations (signatures chosen randomly):
template<> struct TypeSig<std::byte>{ static uint64 typeSig() {return 0x5E92C9783665A77AULL; } };
template<> struct TypeSig<uchar>    { static uint64 typeSig() {return 0x5E92C9783665A77AULL; } };   // equivalent to above
template<> struct TypeSig<int>      { static uint64 typeSig() {return 0x6E78D0CA3F3EE2EAULL; } };
template<> struct TypeSig<uint>     { static uint64 typeSig() {return 0x10609B64EE938647ULL; } };
template<> struct TypeSig<long>     { static uint64 typeSig() {return 0x72ED76724A8186F5ULL; } };
template<> struct TypeSig<unsigned long> { static uint64 typeSig() {return 0x502B9EB2981463C0ULL; } };
template<> struct TypeSig<long long> { static uint64 typeSig() {return 0x72ED76724A8186F5ULL; } };
template<> struct TypeSig<unsigned long long> { static uint64 typeSig() {return 0x502B9EB2981463C0ULL; } };
template<> struct TypeSig<float>    { static uint64 typeSig() {return 0x2692A71030495B97ULL; } };
template<> struct TypeSig<double>   { static uint64 typeSig() {return 0x75629296874859D5ULL; } };
template<> struct TypeSig<bool>     { static uint64 typeSig() {return 0xCF7ECE5FAEA76F77ULL; } };
// Library type (partial & full) specializations:
template<class T,size_t S> struct TypeSig<Arr<T,S>>
{
    static uint64 typeSig() {return treeHash({TypeSig<T>::typeSig(),scast<uint64>(S),0x7DA6DF3E1C1BF4BBULL}); }
};
template<class T> struct TypeSig<Svec<T>>
{
    static uint64 typeSig() {return treeHash({TypeSig<T>::typeSig(),0xC9D2B190616B768AULL}); }
};
template<> struct TypeSig<String> { static uint64 typeSig() {return 0x8EF74E84679AD006ULL; } };
template<> struct TypeSig<String8> { static uint64 typeSig() {return 0x8EF74E84679AD006ULL; } };    // String/String8 same typesig

// Binary serialization macros:
#define FG_SER_BEG                  void srlzm_(Bytes & s) const {
#define FG_SER_M1(A)                srlz_(A,s);
#define FG_SER_M2(A,B)              FG_SER_M1(A) FG_SER_M1(B)
#define FG_SER_M4(A,B,C,D)          FG_SER_M2(A,B) FG_SER_M2(C,D)

#define FG_SER_1(A)                 FG_SER_BEG FG_SER_M1(A) }
#define FG_SER_2(A,B)               FG_SER_BEG FG_SER_M2(A,B) }
#define FG_SER_3(A,B,C)             FG_SER_BEG FG_SER_M2(A,B) FG_SER_M1(C) }
#define FG_SER_4(A,B,C,D)           FG_SER_BEG FG_SER_M4(A,B,C,D) }
#define FG_SER_5(A,B,C,D,E)         FG_SER_BEG FG_SER_M4(A,B,C,D) FG_SER_M1(E) }
#define FG_SER_6(A,B,C,D,E,F)       FG_SER_BEG FG_SER_M4(A,B,C,D) FG_SER_M2(E,F) }
#define FG_SER_7(A,B,C,D,E,F,G)     FG_SER_BEG FG_SER_M4(A,B,C,D) FG_SER_M2(E,F) FG_SER_M1(G) }
#define FG_SER_8(A,B,C,D,E,F,G,H)   FG_SER_BEG FG_SER_M4(A,B,C,D) FG_SER_M4(E,F,G,H) }

// Binary deserialization macros:
#define FG_DSR_BEG                  void dsrlzm_(Bytes const & s,size_t & p) {
#define FG_DSR_M1(A)                dsrlz_(s,p,A);
#define FG_DSR_M2(A,B)              FG_DSR_M1(A) FG_DSR_M1(B)
#define FG_DSR_M4(A,B,C,D)          FG_DSR_M2(A,B) FG_DSR_M2(C,D)

#define FG_DSR_1(A)                 FG_DSR_BEG FG_DSR_M1(A) }
#define FG_DSR_2(A,B)               FG_DSR_BEG FG_DSR_M2(A,B) }
#define FG_DSR_3(A,B,C)             FG_DSR_BEG FG_DSR_M2(A,B) FG_DSR_M1(C) }
#define FG_DSR_4(A,B,C,D)           FG_DSR_BEG FG_DSR_M4(A,B,C,D) }
#define FG_DSR_5(A,B,C,D,E)         FG_DSR_BEG FG_DSR_M4(A,B,C,D) FG_DSR_M1(E) }
#define FG_DSR_6(A,B,C,D,E,F)       FG_DSR_BEG FG_DSR_M4(A,B,C,D) FG_DSR_M2(E,F) }
#define FG_DSR_7(A,B,C,D,E,F,G)     FG_DSR_BEG FG_DSR_M4(A,B,C,D) FG_DSR_M2(E,F) FG_DSR_M1(G) }
#define FG_DSR_8(A,B,C,D,E,F,G,H)   FG_DSR_BEG FG_DSR_M4(A,B,C,D) FG_DSR_M4(E,F,G,H) }

// Type hash signature macros:
#define FG_THS_BEG                  static uint64 typeSig() { return treeHash({
#define FG_THS_M1(A)                TypeSig<decltype(A)>::typeSig(),
#define FG_THS_M2(A,B)              FG_THS_M1(A) FG_THS_M1(B)
#define FG_THS_M4(A,B,C,D)          FG_THS_M2(A,B) FG_THS_M2(C,D)
#define FG_THS_END                  }); }

#define FG_THS_1(A)                 FG_THS_BEG FG_THS_M1(A) FG_THS_END
#define FG_THS_2(A,B)               FG_THS_BEG FG_THS_M2(A,B) FG_THS_END
#define FG_THS_3(A,B,C)             FG_THS_BEG FG_THS_M2(A,B) FG_THS_M1(C) FG_THS_END
#define FG_THS_4(A,B,C,D)           FG_THS_BEG FG_THS_M4(A,B,C,D) FG_THS_END
#define FG_THS_5(A,B,C,D,E)         FG_THS_BEG FG_THS_M4(A,B,C,D) FG_THS_M1(E) FG_THS_END
#define FG_THS_6(A,B,C,D,E,F)       FG_THS_BEG FG_THS_M4(A,B,C,D) FG_THS_M2(E,F) FG_THS_END
#define FG_THS_7(A,B,C,D,E,F,G)     FG_THS_BEG FG_THS_M4(A,B,C,D) FG_THS_M2(E,F) FG_THS_M1(G) FG_THS_END
#define FG_THS_8(A,B,C,D,E,F,G,H)   FG_THS_BEG FG_THS_M4(A,B,C,D) FG_THS_M4(E,F,G,H) FG_THS_END

// COMBINED MACROS:

#define FG_SER1(A)  FG_SER_1(A) FG_DSR_1(A) FG_THS_1(A) FG_RFL_1(A) FG_DRF_1(A)
#define FG_SER2(A,B) FG_SER_2(A,B) FG_DSR_2(A,B) FG_THS_2(A,B) FG_RFL_2(A,B) FG_DRF_2(A,B)
#define FG_SER3(A,B,C) FG_SER_3(A,B,C) FG_DSR_3(A,B,C) FG_THS_3(A,B,C) FG_RFL_3(A,B,C) FG_DRF_3(A,B,C)
#define FG_SER4(A,B,C,D) FG_SER_4(A,B,C,D) FG_DSR_4(A,B,C,D) FG_THS_4(A,B,C,D) FG_RFL_4(A,B,C,D) FG_DRF_4(A,B,C,D)
#define FG_SER5(A,B,C,D,E) FG_SER_5(A,B,C,D,E) FG_DSR_5(A,B,C,D,E) FG_THS_5(A,B,C,D,E) FG_RFL_5(A,B,C,D,E) FG_DRF_5(A,B,C,D,E)
#define FG_SER6(A,B,C,D,E,F) FG_SER_6(A,B,C,D,E,F) FG_DSR_6(A,B,C,D,E,F) FG_THS_6(A,B,C,D,E,F) FG_RFL_6(A,B,C,D,E,F) \
FG_DRF_6(A,B,C,D,E,F)
#define FG_SER7(A,B,C,D,E,F,G) FG_SER_7(A,B,C,D,E,F,G) FG_DSR_7(A,B,C,D,E,F,G) FG_THS_7(A,B,C,D,E,F,G) FG_RFL_7(A,B,C,D,E,F,G) \
FG_DRF_7(A,B,C,D,E,F,G)
#define FG_SER8(A,B,C,D,E,F,G,H) FG_SER_8(A,B,C,D,E,F,G,H) FG_DSR_8(A,B,C,D,E,F,G,H) FG_THS_8(A,B,C,D,E,F,G,H) FG_RFL_8(A,B,C,D,E,F,G,H) \
FG_DRF_8(A,B,C,D,E,F,G,H)

// Client direct usage of binary serialization (no message packaging):
template<class T>
Bytes               srlz(T const & v)
{
    Bytes               ret;
    srlz_(v,ret);
    return ret;
}
template<class T>
T                   dsrlz(Bytes const & s)
{
    T               ret;
    size_t          p = 0;
    dsrlz_(s,p,ret);
    return ret;
}
// A message is a uint64 type signature followed by a binary serialized simple object:
template<class T>
Bytes               toMessage(T const & v,uint64 typeSig)
{
    Bytes               msg;
    srlz_(typeSig,msg);
    srlz_(v,msg);
    return msg;
}
// Serialize to message using the default-defined tree hash type signature:
template<class T>
inline Bytes        toMessage(T const & v) {return toMessage(v,TypeSig<T>::typeSig()); }
// Serialize to message using the type signature manually-defined by T::typeID()
template<class T>
inline Bytes        toMessageExplicit(T const & v) {return toMessage(v,T::typeID()); }
template<class T>
void                fromMessage_(Bytes const & msg,uint64 typeSig,T & v)
{
    size_t          p {0};
    uint64          msgID {0};
    dsrlz_(msg,p,msgID);
    if (msgID != typeSig)
        fgThrow("message deserialization non-matching type signature",toStr(msgID)+"!="+toStr(typeSig));
    dsrlz_(msg,p,v);
}
// Deserialize from message validating the default tree hash type signature:
template<class T>
inline void         fromMessage_(Bytes const & msg,T & v) {fromMessage_(msg,TypeSig<T>::typeSig(),v); }
template<class T>
T                   fromMessage(Bytes const & s)
{
    T               ret;
    fromMessage_(s,ret);
    return ret;
}
// Deserialize from message validating the type signature manually defined by T::typeID()
template<class T>
inline void         fromMessageExplicit_(Bytes const & msg,T & v) {fromMessage_(msg,T::typeID(),v); }
template<class T>
T                   fromMessageExplicit(Bytes const & s)
{
    T               ret;
    fromMessageExplicit_(s,ret);
    return ret;
}
// Reflection utilities:
template<typename T>
void                typeNames_(Strings & str,T const & v)
{
    str.push_back(typeid(v).name());
}
template<typename T,typename... Ts>
void                typeNames_(Strings & str,T const & v,Ts... vs)
{
    str.push_back(typeid(v).name());
    typeNames_(str,vs...);
}

}

#endif
