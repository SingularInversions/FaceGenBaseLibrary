//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
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
// USE:
// 
// * Will NOT work for size_t due to its changing nature so handle any size_t members manually

#ifndef FGSERIAL_HPP
#define FGSERIAL_HPP

#include "FgStdExtensions.hpp"
#include "FgHash.hpp"

namespace Fg {

// Default case redirects serialization to class member serialization, which has a different name
// to avoid confusing errors (ie. when global can't be resolved it tries to call itself but has
// wrong number of arguments):
template<class T>
void srlz_(T const & v,String & s)
{v.srlzm_(s); }
template<class T>
void dsrlz_(String const & s,size_t & p,T & v)
{v.dsrlzm_(s,p); }

// Direct binary serialization (no transformation). T must be simple type
template<class T> void srlzRaw_(T v,String & s)
{
    s.append(reinterpret_cast<char const *>(&v),sizeof(v));
}
template<class T> void srlzRaw_(Svec<T> const & v,String & s)
{
    if (!v.empty())
        s.append(reinterpret_cast<char const *>(&v[0]),sizeof(T)*v.size());
}
template<class T> void srlzRawOverwrite_(T v,String & s,size_t pos)  // random access overwrite version
{
    size_t          len = sizeof(v);
    FGASSERT(pos+len <= s.size());
    char const *    src = reinterpret_cast<char const *>(&v);
    for (size_t ii=0; ii<len; ++ii)
        s[pos+ii] = src[ii];
}
template<class T> void dsrlzRaw_(String const & s,size_t & p,T & v)
{
    size_t          sz = sizeof(v);
    FGASSERT(p+sz <= s.size());
    v = *reinterpret_cast<T const *>(&s[p]);
    p += sz;
}

// Base cases for builtins:
void srlz_(bool v,String & s);                                              // Store as uchar
inline void srlz_(uchar v,String & s) {srlzRaw_(v,s); }                     // Assume always 8bit
inline void srlz_(int v,String & s) {srlzRaw_(v,s); }                       // Assume always 32bit
inline void srlz_(uint v,String & s) {srlzRaw_(v,s); }                      // "
inline void srlz_(long v,String & s) {srlzRaw_(int64(v),s); }               // LP64 / LLP64 interop
inline void srlz_(unsigned long v,String & s) {srlzRaw_(uint64(v),s); }     // "
inline void srlz_(long long v,String & s) {srlzRaw_(v,s); }                 // Assume always 64bit
inline void srlz_(unsigned long long v,String & s) {srlzRaw_(v,s); }        // "
inline void srlz_(float v,String & s) {srlzRaw_(v,s); }                     // Assume always IEEE 754
inline void srlz_(double v,String & s) {srlzRaw_(v,s); }                    // "

void dsrlz_(String const & s,size_t & p,bool & v);
inline void dsrlz_(String const & s,size_t & p,uchar & v) {dsrlzRaw_(s,p,v); }
inline void dsrlz_(String const & s,size_t & p,int & v) {dsrlzRaw_(s,p,v); }
inline void dsrlz_(String const & s,size_t & p,uint & v) {dsrlzRaw_(s,p,v); }
void dsrlz_(String const & s,size_t & p,long & v);                          // interop w/ bounds checks
void dsrlz_(String const & s,size_t & p,unsigned long & v);                 // "
inline void dsrlz_(String const & s,size_t & p,int64 & v) {dsrlzRaw_(s,p,v); }
inline void dsrlz_(String const & s,size_t & p,uint64 & v) {dsrlzRaw_(s,p,v); }
inline void dsrlz_(String const & s,size_t & p,float & v) {dsrlzRaw_(s,p,v); }
inline void dsrlz_(String const & s,size_t & p,double & v) {dsrlzRaw_(s,p,v); }

// Base cases for std lib classes:
template<typename T,size_t S>
void srlz_(Arr<T,S> const & v,String & s)
{
    for (T const & e : v)
        srlz_(e,s);
}
template<typename T,size_t S>
void dsrlz_(String const & s,size_t & p,Arr<T,S> & v)
{
    for (T & e : v)
        dsrlz_(s,p,e);
}
void srlz_(String const & v,String & s);
void dsrlz_(String const & s,size_t & p,String & v);
template<typename T>
void srlz_(Svec<T> const & v,String &s)
{
    srlz_(uint64(v.size()),s);
    for (T const & e : v)
        srlz_(e,s);
}
template<typename T>
void dsrlz_(String const & s,size_t & p,Svec<T> & v)
{
    uint64              sz;
    dsrlz_(s,p,sz);
    FGASSERT(sz <= std::numeric_limits<size_t>::max());
    v.resize(size_t(sz));
    for (T & e : v)
        dsrlz_(s,p,e);
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
template<class T>
struct TypeSig
{
    static uint64 typeSig() {return T::typeSig(); }
};

// Builtin type (full) specializations (signatures chosen randomly):
template<> struct TypeSig<int>    { static uint64 typeSig() {return 0x6E78D0CA3F3EE2EAULL; } };
template<> struct TypeSig<uint>   { static uint64 typeSig() {return 0x10609B64EE938647ULL; } };
template<> struct TypeSig<int64>  { static uint64 typeSig() {return 0x72ED76724A8186F5ULL; } };
template<> struct TypeSig<uint64> { static uint64 typeSig() {return 0x502B9EB2981463C0ULL; } };
template<> struct TypeSig<float>  { static uint64 typeSig() {return 0x2692A71030495B97ULL; } };
template<> struct TypeSig<double> { static uint64 typeSig() {return 0x75629296874859D5ULL; } };
template<> struct TypeSig<bool>   { static uint64 typeSig() {return 0xCF7ECE5FAEA76F77ULL; } };

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

// Binary serialization macros:
#define FG_SER_S_BEG void srlzm_(String & s) const {
#define FG_SER_S1(A) srlz_(A,s);
#define FG_SER_S2(A,B) FG_SER_S1(A) FG_SER_S1(B)
#define FG_SER_S4(A,B,C,D) FG_SER_S2(A,B) FG_SER_S2(C,D)

// Binary deserialization macros:
#define FG_SER_D_BEG void dsrlzm_(String const & s,size_t & p) {
#define FG_SER_D1(A) dsrlz_(s,p,A);
#define FG_SER_D2(A,B) FG_SER_D1(A) FG_SER_D1(B)
#define FG_SER_D4(A,B,C,D) FG_SER_D2(A,B) FG_SER_D2(C,D)

// Type signature macros:
#define FG_SER_T_BEG static uint64 typeSig() { return treeHash({
#define FG_SER_T_END }); }
#define FG_SER_T1(A) TypeSig<decltype(A)>::typeSig(),
#define FG_SER_T2(A,B) FG_SER_T1(A) FG_SER_T1(B)
#define FG_SER_T4(A,B,C,D) FG_SER_T2(A,B) FG_SER_T2(C,D)

// Text tree serialization
struct                  SerNode
{
    virtual                 ~SerNode() {}
    virtual void            print(std::ostream &) const = 0;
    virtual bool            fitsInLine() const = 0;
};
typedef Sptr<SerNode>   SerPtr;
typedef Svec<SerPtr>    SerPtrs;
struct                  SerNodeVal : SerNode
{
    String                  textVal;                        // text serialized value (for builtins)
    explicit SerNodeVal(String const & s) : textVal{s} {}   // make_shared cannot use default agg ctr
    virtual void            print(std::ostream & os) const {os << textVal; }
    virtual bool            fitsInLine() const {return true; }
};
struct                  SerMember
{
    String                  name;
    SerPtr                  value;
};
typedef Svec<SerMember> SerMembers;
struct                  SerNodeStruct : SerNode
{
    SerMembers              membs;
    explicit SerNodeStruct(SerMembers const & s) : membs{s} {}
    virtual void            print(std::ostream & os) const
    {
        for (SerMember const & m : membs) {
            os << fgnl << m.name << ": ";
            if (!m.value->fitsInLine())
                os << fgpush;
            m.value->print(os);
            if (!m.value->fitsInLine())
                os << fgpop;
        }
    }
    virtual bool            fitsInLine() const {return false; }
};
struct                  SerNodeArr : SerNode
{
    SerPtrs                 vals;
    explicit SerNodeArr(SerPtrs const & s) : vals(s) {}
    virtual void            print(std::ostream & os) const
    {
        os << "[";
        if (fitsInLine()) {
            for (SerPtr const & v : vals) {
                v->print(os);
                os << " ";
            }
        }
        else {
            for (size_t ii=0; ii<vals.size(); ++ii) {
                SerPtr const &      v = vals[ii];
                os << fgnl << ii << ": ";
                if (!v->fitsInLine())
                    os << fgpush;
                v->print(os);
                if (!v->fitsInLine())
                    os << fgpop;
            }
            os << fgnl;
        }
        os << "]";
    }
    virtual bool            fitsInLine() const
    {
        if (vals.empty())
            return true;
        return ((vals.size() < 8) && vals[0]->fitsInLine());
    }
};
inline std::ostream & operator<<(std::ostream & os,SerPtr const & sp) {sp->print(os); return os; }
inline SerPtr cSerVal(String const & v) {return std::make_shared<SerNodeVal>(v); }
template<class T>  SerPtr tsrlz(T const & v) {return v.tsrlzm(); }          // base case redirects to member
inline SerPtr tsrlz(bool v) {return cSerVal(v ? "true" : "false"); }
inline SerPtr tsrlz(char v) {return cSerVal(String{v}); }
inline SerPtr tsrlz(uchar v) {return cSerVal(toStr(v)); }
inline SerPtr tsrlz(int16 v) {return cSerVal(toStr(v)); }
inline SerPtr tsrlz(uint16 v) {return cSerVal(toStr(v)); }
inline SerPtr tsrlz(int v) {return cSerVal(toStr(v)); }                     // overloads for builtin & lib types
inline SerPtr tsrlz(uint v) {return cSerVal(toStr(v)); }
inline SerPtr tsrlz(int64 v) {return cSerVal(toStr(v)); }
inline SerPtr tsrlz(uint64 v) {return cSerVal(toStr(v)); }
inline SerPtr tsrlz(float v) {return cSerVal(toStr(v)); }
inline SerPtr tsrlz(double v) {return cSerVal(toStrDigits(v,6)); }
inline SerPtr tsrlz(String const & s) {return cSerVal(s); }
#ifdef _WIN32       // wstring == String32 on nix:
inline SerPtr tsrlz(std::wstring const & s) {return cSerVal(toUtf8(s)); }
#endif
inline SerPtr tsrlz(String32 const & s) {return cSerVal(toUtf8(s)); }
template<class T,size_t S> SerPtr tsrlz(std::array<T,S> const & v)
{
    SerPtrs                 ret; ret.reserve(S);
    for (T const & e : v)
        ret.push_back(tsrlz(e));
    return std::make_shared<SerNodeArr>(ret);
}
template<class T> SerPtr tsrlz(std::vector<T> const & v)
{
    SerPtrs                 ret; ret.reserve(v.size());
    for (T const & e : v)
        ret.push_back(tsrlz(e));
    return std::make_shared<SerNodeArr>(ret);
}

#define FG_SER_O_BEG SerPtr tsrlzm() const { return std::make_shared<SerNodeStruct>(SerMembers{
#define FG_SER_O_END }); }
#define FG_SER_O1(A) { #A , tsrlz(A) },
#define FG_SER_O2(A,B) FG_SER_O1(A) FG_SER_O1(B)
#define FG_SER_O4(A,B,C,D) FG_SER_O2(A,B) FG_SER_O2(C,D)

// Combined macros:

#define FG_SER1(A)                                                                  \
FG_SER_S_BEG FG_SER_S1(A) }                                                         \
FG_SER_D_BEG FG_SER_D1(A) }                                                         \
FG_SER_O_BEG FG_SER_O1(A) FG_SER_O_END \
FG_SER_T_BEG FG_SER_T1(A) FG_SER_T_END

#define FG_SER2(A,B)                                                                \
FG_SER_S_BEG FG_SER_S2(A,B) }                                                       \
FG_SER_D_BEG FG_SER_D2(A,B) }                                                       \
FG_SER_O_BEG FG_SER_O2(A,B) FG_SER_O_END \
FG_SER_T_BEG FG_SER_T2(A,B) FG_SER_T_END

#define FG_SER3(A,B,C)                                                              \
FG_SER_S_BEG FG_SER_S2(A,B) FG_SER_S1(C) }                                          \
FG_SER_D_BEG FG_SER_D2(A,B) FG_SER_D1(C) }                                          \
FG_SER_O_BEG FG_SER_O2(A,B) FG_SER_O1(C) FG_SER_O_END \
FG_SER_T_BEG FG_SER_T2(A,B) FG_SER_T1(C) FG_SER_T_END

#define FG_SER4(A,B,C,D)                                                            \
FG_SER_S_BEG FG_SER_S4(A,B,C,D) }                                                   \
FG_SER_D_BEG FG_SER_D4(A,B,C,D) }                                                   \
FG_SER_O_BEG FG_SER_O4(A,B,C,D) FG_SER_O_END \
FG_SER_T_BEG FG_SER_T4(A,B,C,D) FG_SER_T_END

#define FG_SER5(A,B,C,D,E)                                                          \
FG_SER_S_BEG FG_SER_S4(A,B,C,D) FG_SER_S1(E) }                                      \
FG_SER_D_BEG FG_SER_D4(A,B,C,D) FG_SER_D1(E) }                                      \
FG_SER_O_BEG FG_SER_O4(A,B,C,D) FG_SER_O1(E) FG_SER_O_END \
FG_SER_T_BEG FG_SER_T4(A,B,C,D) FG_SER_T1(E) FG_SER_T_END

#define FG_SER6(A,B,C,D,E,F)                                                        \
FG_SER_S_BEG FG_SER_S4(A,B,C,D) FG_SER_S2(E,F) }                                    \
FG_SER_D_BEG FG_SER_D4(A,B,C,D) FG_SER_D2(E,F) }                                    \
FG_SER_O_BEG FG_SER_O4(A,B,C,D) FG_SER_O2(E,F) FG_SER_O_END \
FG_SER_T_BEG FG_SER_T4(A,B,C,D) FG_SER_T2(E,F) FG_SER_T_END

#define FG_SER7(A,B,C,D,E,F,G)                                                      \
FG_SER_S_BEG FG_SER_S4(A,B,C,D) FG_SER_S2(E,F) FG_SER_S1(G) }                       \
FG_SER_D_BEG FG_SER_D4(A,B,C,D) FG_SER_D2(E,F) FG_SER_D1(G) }                       \
FG_SER_O_BEG FG_SER_O4(A,B,C,D) FG_SER_O2(E,F) FG_SER_O1(G) FG_SER_O_END \
FG_SER_T_BEG FG_SER_T4(A,B,C,D) FG_SER_T2(E,F) FG_SER_T1(G) FG_SER_T_END

#define FG_SER8(A,B,C,D,E,F,G,H)                                                    \
FG_SER_S_BEG FG_SER_S4(A,B,C,D) FG_SER_S4(E,F,G,H) }                                \
FG_SER_D_BEG FG_SER_D4(A,B,C,D) FG_SER_D4(E,F,G,H) }                                \
FG_SER_O_BEG FG_SER_O4(A,B,C,D) FG_SER_O4(E,F,G,H) FG_SER_O_END \
FG_SER_T_BEG FG_SER_T4(A,B,C,D) FG_SER_T4(E,F,G,H) FG_SER_T_END

// Client direct usage of serialization (no message packaging):
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

// A message is a uint64 type signature followed by a binary serialized simple object:
template<class T>
String
toMessage(T const & v,uint64 typeSig)
{
    String          msg;
    srlz_(typeSig,msg);
    srlz_(v,msg);
    return msg;
}

// Serialize to message using the default-defined tree hash type signature:
template<class T>
inline String toMessage(T const & v) {return toMessage(v,T::typeSig()); }

// Serialize to message using the type signature manually-defined by T::typeID()
template<class T>
inline String toMessageExplicit(T const & v) {return toMessage(v,T::typeID()); }

template<class T>
void
fromMessage_(String const & msg,uint64 typeSig,T & v)
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
inline void fromMessage_(String const & msg,T & v) {fromMessage_(msg,T::typeSig(),v); }

template<class T>
T
fromMessage(String const & s)
{
    T               ret;
    fromMessage_(s,ret);
    return ret;
}

// Deserialize from message validating the type signature manually defined by T::typeID()
template<class T>
inline void fromMessageExplicit_(String const & msg,T & v) {fromMessage_(msg,T::typeID(),v); }

template<class T>
T
fromMessageExplicit(String const & s)
{
    T               ret;
    fromMessageExplicit_(s,ret);
    return ret;
}

// Reflection utilities:
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

}

#endif
