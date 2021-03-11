//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Very simple basic binary serialization to/from std::string that:
//
// * Is 32/64 bit build portable on LLP64 and LP64 architectures
// * Is NOT endian independent
// * Uses default construction followed by value population
// * Doesn't use boost::serialization which has drawbacks:
//   1. version updates are not forward compatible
//   2. portable binary serialization is not directly supported but relies on an example file
//   3. class or field name changes can break compatibility

#ifndef FGSERIAL_HPP
#define FGSERIAL_HPP

#include "FgStdString.hpp"
#include "FgStdVector.hpp"

namespace Fg {

// Used in macros to enable struct name reflection; default used for all external code
// and complete specializations made for each reflected struct:
template<class T> void reflectNames_(Strings &) {}

#define FG_REFLECT_SIG(S) template<> inline void reflectNames_<S>(Strings & s)
#define FG_REFLECT_1(A) s.push_back(#A);
#define FG_REFLECT_2(A,B) FG_REFLECT_1(A) FG_REFLECT_1(B)
#define FG_REFLECT_4(A,B,C,D) FG_REFLECT_2(A,B) FG_REFLECT_2(C,D)

#define FG_TRAVERSE_SIG(S) template<typename Op> void traverseMembers_(Op & op,S & s)
#define FG_TRAVERSE_1(A) traverseMembers_(op,s.A);
#define FG_TRAVERSE_2(A,B) FG_TRAVERSE_1(A) FG_TRAVERSE_1(B)
#define FG_TRAVERSE_4(A,B,C,D) FG_TRAVERSE_2(A,B) FG_TRAVERSE_2(C,D)

#define FG_SERIAL_1(S,A) FG_REFLECT_SIG(S) { FG_REFLECT_1(A) } FG_TRAVERSE_SIG(S) { FG_TRAVERSE_1(A) }
#define FG_SERIAL_2(S,A,B) FG_REFLECT_SIG(S) { FG_REFLECT_2(A,B) } FG_TRAVERSE_SIG(S) { FG_TRAVERSE_2(A,B) }

// Direct binary serialization (no transformation)
template<class T>   // T must be simple type
void
srlzBytes_(T v,String & s)
{
    s.append(reinterpret_cast<char const *>(&v),sizeof(v));
}
template<class T>   // T must be simple type
void
dsrlzBytes_(String const & s,size_t & p,T & v)
{
    size_t          sz = sizeof(v);
    FGASSERT(p+sz <= s.size());
    v = *reinterpret_cast<T const *>(&s[p]);
    p += sz;
}

inline void srlz_(int v,String & s) {srlzBytes_(v,s); }                     // Assume always 32bit
inline void srlz_(uint v,String & s) {srlzBytes_(v,s); }                    // "
inline void srlz_(long v,String & s) {srlzBytes_(int64(v),s); }             // LP64 / LLP64 interop
inline void srlz_(unsigned long v,String & s) {srlzBytes_(uint64(v),s); }   // "
inline void srlz_(long long v,String & s) {srlzBytes_(v,s); }               // Assume always 64bit
inline void srlz_(unsigned long long v,String & s) {srlzBytes_(v,s); }      // "

inline void dsrlz_(String const & s,size_t & p,int & v) {dsrlzBytes_(s,p,v); }
inline void dsrlz_(String const & s,size_t & p,uint & v) {dsrlzBytes_(s,p,v); }
void dsrlz_(String const & s,size_t & p,long & v);                          // interop w/ bounds checks
void dsrlz_(String const & s,size_t & p,unsigned long & v);                 // "
inline void dsrlz_(String const & s,size_t & p,long long & v) {dsrlzBytes_(s,p,v); }
inline void dsrlz_(String const & s,size_t & p,unsigned long long & v) {dsrlzBytes_(s,p,v); }

void srlz_(String const & v,String & s);
void dsrlz_(String const & s,size_t & p,String & v);

template<typename T>
void srlz_(Svec<T> const & v,String &s)
{
    srlz_(v.size(),s);
    for (T const & e : v)
        srlz_(e,s);
}
template<typename T>
void dsrlz_(String const & s,size_t & p,Svec<T> & v)
{
    size_t              sz;
    dsrlz_(s,p,sz);
    v.clear();
    v.reserve(sz);
    for (T & e : v)
        dsrlz_(s,p,e);
}

#define FG_SER_STRUCT1(T,A)                                                 \
inline void srlz_(T const & t,String & s)                                   \
{                                                                           \
    srlz_(t.A,s);                                                           \
}                                                                           \
inline void dsrlz_(String const & s,size_t & p,T & t)                       \
{                                                                           \
    dsrlz_(s,p,t.A);                                                        \
}
#define FG_SER_STRUCT2(T,A,B)                                               \
inline void srlz_(T const & t,String & s)                                   \
{                                                                           \
    srlz_(t.A,s);                                                           \
    srlz_(t.B,s);                                                           \
}                                                                           \
inline void dsrlz_(String const & s,size_t & p,T & t)                       \
{                                                                           \
    dsrlz_(s,p,t.A);                                                        \
    dsrlz_(s,p,t.B);                                                        \
}
#define FG_SER_STRUCT3(T,A,B,C)                                             \
inline void srlz_(T const & t,String & s)                                   \
{                                                                           \
    srlz_(t.A,s);                                                           \
    srlz_(t.B,s);                                                           \
    srlz_(t.C,s);                                                           \
}                                                                           \
inline void dsrlz_(String const & s,size_t & p,T & t)                       \
{                                                                           \
    dsrlz_(s,p,t.A);                                                        \
    dsrlz_(s,p,t.B);                                                        \
    dsrlz_(s,p,t.C);                                                        \
}

template<class T>
String
srlz(T const & v)
{
    String          ret;
    srlz_(v,ret);
    return ret;
}
template<class T>
T
dsrlz(String const & s)
{
    T               ret;
    size_t          p = 0;
    dsrlz_(s,p,ret);
    return ret;
}

// To create a message the type T must have static member typeID() defined:
template<class T>
String
toSerialMessage(T const & v)
{
    String          ret;
    srlz_(static_cast<uint64>(T::typeID()),ret);
    srlz_(v,ret);
    return ret;
}
template<class T>
void
fromSerialMessage_(String const & s,T & v)
{
    uint64          id = dsrlz<uint64>(s);
    FGASSERT(id == T::typeID());
    size_t          p {8};
    dsrlz_(s,p,v);
}
template<class T>
T
fromSerialMessage(String const & s)
{
    T               ret;
    fromSerialMessage_(s,ret);
    return ret;
}

template<typename T>
void
typeNames_(Strings & str,T const & v)
{
    str.push_back(typeid(v).name());
}
template<typename T,typename... Ts>
void
typeNames_(Strings & str,T const & v,Ts... vs)
{
    str.push_back(typeid(v).name());
    typeNames_(str,vs...);
}

}

#endif
