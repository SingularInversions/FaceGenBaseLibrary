//
// Copyright (c) 2017 Singular Inversions Inc.
//

//
// Ultra-simple binary serialization that:
//
// * Is 32/64 bit build portable on LLP64 and LP64 architectures
// * Is NOT endian independent
// * Does automatic type check but doesn't care about class or field names, will accept anything
//   that serializes as the same basic types in the same order.
// * Doesn't use boost::serialization which has drawbacks:
//   1. version updates are not forward compatible
//   2. portable binary serialization is not directly supported but relies on an example file
//   3. class or field name changes can break compatibility

#ifndef FGSERIAL_HPP
#define FGSERIAL_HPP

#include "FgStdString.hpp"
#include "FgStdVector.hpp"

namespace Fg {

// TYPE SIGNATURE:

template<class T> inline uint64 fgSerSig();

template<> inline uint64 fgSerSig<char>() {return 0xCB3906CB6D694578ULL; }
template<> inline uint64 fgSerSig<int>() {return 0xD02AB62D5BBA9F92ULL; }
template<> inline uint64 fgSerSig<unsigned int>() {return 0x60265052875F1A51ULL; }
template<> inline uint64 fgSerSig<long>() {return 0xAD962DA037970441ULL; }
template<> inline uint64 fgSerSig<unsigned long>() {return 0xDA4814A2F7700559ULL; }
template<> inline uint64 fgSerSig<long long>() {return 0xAABC2A5FDA8BA6CCULL; }
template<> inline uint64 fgSerSig<unsigned long long>() {return 0x7E08A77439218116ULL; }
// Not sure why overload with templated std::basic_string didn't work so do full specialization:
template<> inline uint64 fgSerSig<String>() {return 0x5B37748C96AB3A76ULL; }

uint64 fgHash(uint64 k0,uint64 k1);
uint64 fgHash(uint64 k0,uint64 k1,uint64 k2);

// Partial specialization of functions is not allowed so this approach won't work ... 
//template<class T>
//uint64 fgSerSig<Svec<T> >() {return fgHash(fgSerSig<T>(),0x9A77AEB690E81D6EULL); }

// SERIALIZE

template<class T>
inline
String
fgSerBuiltin(T v)
{return String((char*)&v,sizeof(T)); }

#define FG_SER_BI(T) inline String fgSer(T v) {return fgSerBuiltin(v); }

FG_SER_BI(int)
FG_SER_BI(unsigned int)
FG_SER_BI(long long)
FG_SER_BI(unsigned long long)

// Handle the LLP64 - LP64 difference:
inline String fgSer(unsigned long v) {return fgSerBuiltin(uint64(v)); }
inline String fgSer(long v) {return fgSerBuiltin(int64(v)); }

inline
String
fgSer(const String & str)
{return cat(fgSer(uint64(str.size())),str); }

template<class T>
String
fgSer(const Svec<T> & v)
{
    String      ret = fgSer(uint64(v.size()));
    for (size_t ii=0; ii<v.size(); ++ii)
        ret.append(fgSer(v[ii]));
    return ret;
}

// DESERIALIZE

template<class T>
inline
void
fgDsrBuiltin(const char * & ptr,const char * end,T & v)
{
    // Who knew differences between pointers were signed ints:
    FGASSERT(end-ptr >= int64(sizeof(T)));
    v = *((T*)ptr);
    ptr += sizeof(T);
}

#define FG_DSR_BT(T) inline void fgDsr(const char * & ptr,const char * end,T & v) {fgDsrBuiltin(ptr,end,v); }

FG_DSR_BT(int)
FG_DSR_BT(unsigned int)
FG_DSR_BT(long long)
FG_DSR_BT(unsigned long long)

// Handle the LLP64 - LP64 difference:
void fgDsr(const char * & ptr,const char * end,long & val);
void fgDsr(const char * & ptr,const char * end,unsigned long & val);

inline
void
fgDsr(const char * & ptr,const char * end,String & str)
{
    uint64          sz;
    fgDsr(ptr,end,sz);
    FGASSERT(uint64(end)-uint64(ptr) >= sz);
    FGASSERT(sz <= std::numeric_limits<size_t>::max());
    str = String(ptr,sz);
    ptr += sz;
}

template<class T>
void
fgDsr(const char * & ptr,const char * end,Svec<T> & v)
{
    uint64          sz;
    fgDsr(ptr,end,sz);
    FGASSERT(uint64(end)-uint64(ptr) >= sz);
    FGASSERT(sz <= std::numeric_limits<size_t>::max());
    v.resize(size_t(sz));
    for (size_t ii=0; ii<v.size(); ++ii)
        fgDsr(ptr,end,v[ii]);
}

}

#endif
