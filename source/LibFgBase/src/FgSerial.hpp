//
// Copyright (c) 2025 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Simple by-value serialization and related functionality
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

// SERIALIZE / DESERIALIZE TO REFLECTION TREE:
//  * class/struct becomes RflStruct
//  * svec/array becomes RflArray
//  * builtin types stored as strings
// This can be used for serialization / deserialization of text formats (simple text, JSON, XML)

struct      RflMember
{
    String          name;
    std::any        object;
};
typedef Svec<RflMember>     RflMembers;
struct      RflStruct
{
    RflMembers      members;
};
struct      RflArray
{
    Svec<std::any>  elems;
};

// classes redirect to member function 'cReflect' defined by FG_RFL macros:
template<class T>
inline std::any     toReflect(T const & strct) {return strct.cReflect(); }
// all builtins are strings:
inline std::any     toReflect(bool v) {return v ? String{"true"} : String{"false"}; }   // prefer to default "0" : "1"
inline std::any     toReflect(uchar v) {return toStr(v); }
inline std::any     toReflect(int v) {return toStr(v); }
inline std::any     toReflect(uint v) {return toStr(v); }
inline std::any     toReflect(long v) {return toStr(v); }
inline std::any     toReflect(unsigned long v) {return toStr(v); }
inline std::any     toReflect(long long v) {return toStr(v); }
inline std::any     toReflect(unsigned long long v) {return toStr(v); }
inline std::any     toReflect(float v) {return toStr(v); }
inline std::any     toReflect(double v) {return toStr(v); }
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

template<class... Args>
std::any            toreflAggregate(Strings const & mnames,Args const & ... args)
{
    size_t              cnt {0};
    auto                fn = [&](auto arg) -> RflMember
    {
        return {mnames[cnt++],toReflect(arg)};
    };
    return RflStruct{RflMembers{fn(args) ... }};
}
Strings             splitCommas(String const & csvs);
#define FG_MNAMES(...)  static Strings memberNames() {static Strings ret = splitCommas( #__VA_ARGS__ ); return ret; }
#define FG_TOREFL(...)  std::any cReflect() const {return toreflAggregate(memberNames(), __VA_ARGS__ ); }

template<class T>
T                   fromAny(std::any const & a)
{
    if (a.type() != typeid(T))
        fgThrow("bad any_cast from,to",a.type().name(),typeid(T).name());
    return std::any_cast<T>(a);
}
template<class T>
T                   fromAnyStr(std::any const & a)
{
    String              str = fromAny<String>(a);
    T                   val;
    std::istringstream  iss {str};
    iss >> val;
    if (iss.fail())
        fgThrow("bad string deserial from,to",str,typeid(T).name());
    return val;
}

template<class T>
inline void         fromReflect_(std::any const & node,T & strct) {strct.setRflct(node); }
inline void         fromReflect_(std::any const & node,bool & val)
{
    String              str = fromAny<String>(node);
    if (str == "true")
        val = true;
    else if (str == "false")
        val = false;
    else
        fgThrow("bad string deserial to bool from",str);
}
inline void         fromReflect_(std::any const & node,uchar & val) {val = fromAnyStr<uchar>(node); }
inline void         fromReflect_(std::any const & node,int & val) {val = fromAnyStr<int>(node); }
inline void         fromReflect_(std::any const & node,uint & val) {val = fromAnyStr<uint>(node); }
inline void         fromReflect_(std::any const & node,long & val) {val = fromAnyStr<long>(node); }
inline void         fromReflect_(std::any const & node,unsigned long & val) {val = fromAnyStr<unsigned long>(node); }
inline void         fromReflect_(std::any const & node,long long & val) {val = fromAnyStr<long long>(node); }
inline void         fromReflect_(std::any const & node,unsigned long long & val) {val = fromAnyStr<unsigned long long>(node); }
inline void         fromReflect_(std::any const & node,float & val) {val = fromAnyStr<float>(node); }
inline void         fromReflect_(std::any const & node,double & val) {val = fromAnyStr<double>(node); }
inline void         fromReflect_(std::any const & node,String & str) {str = fromAny<String>(node); }
inline void         fromReflect_(std::any const & node,String8 & str) {str = fromAny<String>(node); }
// forward declaration to handle Arr<Svec<...>>:
template<class T> void fromReflect_(std::any const & node,Svec<T> & val);
template<class T,size_t S>
void                fromReflect_(std::any const & node,Arr<T,S> & val)
{
    RflArray            arr = fromAny<RflArray>(node);
    FGASSERT(arr.elems.size() == S);
    for (size_t ii=0; ii<S; ++ii)
        fromReflect_(arr.elems[ii],val[ii]);
}
template<class T>
void                fromReflect_(std::any const & node,Svec<T> & val)
{
    RflArray            arr = fromAny<RflArray>(node);
    size_t              S = arr.elems.size();
    val.resize(S);
    for (size_t ii=0; ii<S; ++ii)
        fromReflect_(arr.elems[ii],val[ii]);
}
template<> inline void fromReflect_(std::any const & node,Bytes & val) {val = stringToBytes(fromAny<String>(node)); }

std::any            textToReflect(String const & txt);
template<class T>
T                   dsrlzText(String const & txt)
{
    T                   ret;
    fromReflect_(textToReflect(txt),ret);
    return ret;
}

// TODO: use the following along with SFINAE (or concepts) to create a variadic template for comparison operators:
template<class... Args>
void                setrflAggregate(std::any const & n,Args & ... args)
{
    RflStruct           strct = fromAny<RflStruct>(n);
    size_t              cnt {0};
    auto                fn = [&](auto & a)
    {
        fromReflect_(strct.members[cnt++].object,a);
    };
    (fn(args), ...);
}
#define FG_FRMRFL(...)  void setRflct(std::any const & n) {setrflAggregate(n, __VA_ARGS__ ); }

// COMPARISONS:
template<class... Args>
auto                makeCrefTuple(Args const & ... args)
{
    return std::make_tuple(std::cref(args) ...);
}
#define FG_ASCTPL(...)  auto asCrefTuple() const {return makeCrefTuple(__VA_ARGS__); }

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
// * The default global function redirects serialization to class member serialization, which has a different name
// to avoid ambiguity. 
// * The member function redirects back to the global function for each member object
// * The global functions catch the builtin and std library types (vector, array)
// * this back-and-forth has 2 important impliciations:
//   1. the member functions to not have to be visible to the global functions so the recursion does not require
//      declaring all member functions before the globals.
//   2. you can override the member functions by creating a global function that accepts that structure
template<class T>
void                srlz_(T const & v,Bytes & s) {v.srlzm_(s); }
// full specializations for builtins and string:
void                srlz_(bool v,Bytes & s);                                       // Store as uchar
inline void         srlz_(uchar v,Bytes & s) {srlzRaw_(v,s); }                     // Assume always 8bit
inline void         srlz_(int16 v,Bytes & s) {srlzRaw_(v,s); }
inline void         srlz_(uint16 v,Bytes & s) {srlzRaw_(v,s); }
inline void         srlz_(int v,Bytes & s) {srlzRaw_(v,s); }                       // Assume always 32bit
inline void         srlz_(uint v,Bytes & s) {srlzRaw_(v,s); }                      // "
inline void         srlz_(long v,Bytes & s) {srlzRaw_(int64(v),s); }               // LP64 / LLP64 interop
inline void         srlz_(unsigned long v,Bytes & s) {srlzRaw_(uint64(v),s); }     // "
inline void         srlz_(long long v,Bytes & s) {srlzRaw_(v,s); }                 // Assume always 64bit
inline void         srlz_(unsigned long long v,Bytes & s) {srlzRaw_(v,s); }        // "
inline void         srlz_(float v,Bytes & s) {srlzRaw_(v,s); }                     // Assume always IEEE 754
inline void         srlz_(double v,Bytes & s) {srlzRaw_(v,s); }                    // "
inline void         srlzSizet_(size_t val,Bytes & ser) {srlz_(scast<uint64>(val),ser); }
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
inline void         dsrlz_(Bytes const & s,size_t & p,int16 & v) {dsrlzRaw_(s,p,v); }
inline void         dsrlz_(Bytes const & s,size_t & p,uint16 & v) {dsrlzRaw_(s,p,v); }
inline void         dsrlz_(Bytes const & s,size_t & p,int & v) {dsrlzRaw_(s,p,v); }
inline void         dsrlz_(Bytes const & s,size_t & p,uint & v) {dsrlzRaw_(s,p,v); }
void                dsrlz_(Bytes const & s,size_t & p,long & v);                   // interop w/ bounds checks
void                dsrlz_(Bytes const & s,size_t & p,unsigned long & v);          // "
inline void         dsrlz_(Bytes const & s,size_t & p,long long & v) {dsrlzRaw_(s,p,v); }
inline void         dsrlz_(Bytes const & s,size_t & p,unsigned long long & v) {dsrlzRaw_(s,p,v); }
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
// Implementation requires a dummy struct since the function takes no arguments, only a
// type template, and thus cannot be overloaded, and function templates do not support partial
// specialization which means we cannot support standard library partial template specializations:
template<class Struct> struct TS        // base case handles struct:
{
    static uint64   typeSig()
    {
        // get the type of the tuple of member variables:
        typedef std::invoke_result_t<decltype(&Struct::asTuple),Struct> Tuple;
        // dispatch to a function with an argument type reflecting the tuple size:
        return typsigAggregate<Tuple>(std::make_index_sequence<std::tuple_size_v<Tuple>>{});
    }
    template<class Tuple,size_t... Inds>
    static uint64   typsigAggregate(std::index_sequence<Inds...>)
    {
        // hash the initializer list of type signatures for each of the tuple types:
        return treeHash({TS<std::tuple_element_t<Inds,Tuple>>::typeSig() ... });
    }
};
// Builtin type (full) specializations (signatures chosen randomly):
template<> struct TS<std::byte>{ static uint64 typeSig() {return 0x5E92C9783665A77AULL; } };
template<> struct TS<uchar>    { static uint64 typeSig() {return 0x5E92C9783665A77AULL; } };   // equivalent to above
template<> struct TS<int>      { static uint64 typeSig() {return 0x6E78D0CA3F3EE2EAULL; } };
template<> struct TS<uint>     { static uint64 typeSig() {return 0x10609B64EE938647ULL; } };
template<> struct TS<long>     { static uint64 typeSig() {return 0x72ED76724A8186F5ULL; } };
template<> struct TS<unsigned long> { static uint64 typeSig() {return 0x502B9EB2981463C0ULL; } };
template<> struct TS<long long> { static uint64 typeSig() {return 0x72ED76724A8186F5ULL; } };
template<> struct TS<unsigned long long> { static uint64 typeSig() {return 0x502B9EB2981463C0ULL; } };
template<> struct TS<float>    { static uint64 typeSig() {return 0x2692A71030495B97ULL; } };
template<> struct TS<double>   { static uint64 typeSig() {return 0x75629296874859D5ULL; } };
template<> struct TS<bool>     { static uint64 typeSig() {return 0xCF7ECE5FAEA76F77ULL; } };
// Library type (partial & full) specializations:
template<class T,size_t S> struct TS<Arr<T,S>>
{
    static uint64 typeSig() {return treeHash({TS<T>::typeSig(),scast<uint64>(S),0x7DA6DF3E1C1BF4BBULL}); }
};
template<class T> struct TS<Svec<T>>
{
    static uint64 typeSig() {return treeHash({TS<T>::typeSig(),0xC9D2B190616B768AULL}); }
};
template<> struct TS<String> { static uint64 typeSig() {return 0x8EF74E84679AD006ULL; } };
template<> struct TS<String8> { static uint64 typeSig() {return 0x8EF74E84679AD006ULL; } };    // String/String8 same typesig
// macros cannot be used recursively so there is no way to annotate each element of a variadic macro;
// they only come as a single blob of text. So in order to make a static type sig function we need to
// have a member function returning a tuple of all the members and statically make use of that return
// value type:
#define FG_ASTUPL(...)  auto asTuple() const {return std::make_tuple( __VA_ARGS__ ); }

template<typename... Args>
void                srlzAggregate_(Bytes & s,Args const & ... args)
{
    (srlz_(args,s), ...);
}
template<typename... Args>
void                dsrlzAggregate_(Bytes const & s,size_t & p,Args & ... args)
{
    (dsrlz_(s,p,args), ...);
}
#define FG_BINSER(...)              void srlzm_(Bytes & s) const {srlzAggregate_(s, __VA_ARGS__ ); }
#define FG_BINDSR(...)              void dsrlzm_(Bytes const & s,size_t & p) {dsrlzAggregate_(s,p, __VA_ARGS__ ); }

// COMBINED MACROS:

#define FG_SER(...) FG_BINSER(__VA_ARGS__) FG_BINDSR(__VA_ARGS__) FG_MNAMES(__VA_ARGS__) FG_TOREFL(__VA_ARGS__) FG_FRMRFL(__VA_ARGS__) FG_ASTUPL(__VA_ARGS__)

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
inline Bytes        toMessage(T const & v) {return toMessage(v,TS<T>::typeSig()); }
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
inline void         fromMessage_(Bytes const & msg,T & v) {fromMessage_(msg,TS<T>::typeSig(),v); }
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
