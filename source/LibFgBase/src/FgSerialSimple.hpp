//
// Copyright (c) 2017 Singular Inversions Inc.
//
// Authors: Andrew Beatty
// Created: March 10, 2017
//
// Ultra-simple binary serialization that:
//
// * Doesn't create compatibility difficulties whenever boost is updated
// * Is 32/64 bit build portable on LLP64 and LP64 architectures
// * Allows for manual versioning via serializing to a 'message' with a magic number
//

#ifndef FGSERIALSIMPLE_HPP
#define FGSERIALSIMPLE_HPP

#include "FgStdString.hpp"
#include "FgStdVector.hpp"

// SERIALIZE

template<class T>
inline
string
fgSerBuiltin(T v)
{return string((char*)&v,sizeof(T)); }

// Default is to use the member serialization function, but override this below for all builtins and
// standard library types:
template<class T>
string fgSer(const T & v) {return v.ser(v); }

#define FG_SER_BI(T) inline string fgSer(T v) {return fgSerBuiltin(v); }

FG_SER_BI(int)
FG_SER_BI(unsigned int)
FG_SER_BI(long long)
FG_SER_BI(unsigned long long)

// Handle the LLP64 - LP64 difference:
inline string fgSer(unsigned long v) {return fgSerBuiltin(uint64(v)); }
inline string fgSer(long v) {return fgSerBuiltin(int64(v)); }

inline
string
fgSer(const string & str)
{return fgCat(fgSer(uint64(str.size())),str); }

template<class T>
string
fgSer(const vector<T> & v)
{
    string      ret = fgSer(uint64(v.size()));
    for (size_t ii=0; ii<v.size(); ++ii)
        fgAppend(ret,fgSer(v[ii]));
    return ret;
}

// DESERIALIZE

// Default is to use the member serialization function, but override this below for all builtins and
// standard library types:
template<class T>
void fgDsr(const char * & ptr,const char * end,T & v) {v.dsr(ptr,end); }

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
inline void fgDsr(const char * & ptr,const char * end,long & val)
{
    FGASSERT(end-ptr >= int64(sizeof(int64)));
    int64           tmp;
    fgDsrBuiltin(ptr,end,tmp);
    FGASSERT((tmp <= std::numeric_limits<long>::max()) && (tmp >= std::numeric_limits<long>::min()));
    val = long(tmp);
}
inline void fgDsr(const char * & ptr,const char * end,unsigned long & val)
{
    FGASSERT(end-ptr >= int64(sizeof(uint64)));
    uint64          tmp;
    fgDsrBuiltin(ptr,end,tmp);
    FGASSERT(tmp <= std::numeric_limits<unsigned long>::max());
    val = long(tmp);
}

inline
void
fgDsr(const char * & ptr,const char * end,string & str)
{
    uint64          sz;
    fgDsr(ptr,end,sz);
    FGASSERT(uint64(end)-uint64(ptr) >= sz);
    FGASSERT(sz <= std::numeric_limits<size_t>::max());
    str = string(ptr,sz);
    ptr += sz;
}

template<class T>
void
fgDsr(const char * & ptr,const char * end,vector<T> & v)
{
    uint64          sz;
    fgDsr(ptr,end,sz);
    FGASSERT(uint64(end)-uint64(ptr) >= sz);
    FGASSERT(sz <= std::numeric_limits<size_t>::max());
    v.resize(size_t(sz));
    for (size_t ii=0; ii<v.size(); ++ii)
        fgDsr(ptr,end,v[ii]);
}

// HANDY MACROS:

#define FG_SER_MBR_BEG string ser() const {string ret;
#define FG_SER_MBR2(A,B) FG_SER_MBR_BEG ret.append(fgSer(A)); ret.append(fgSer(B)); return ret; }
#define FG_SER_MBR3(A,B,C) FG_SER_MBR_BEG ret.append(fgSer(A)); ret.append(fgSer(B)); ret.append(fgSer(C)); return ret; }

#define FG_DSR_MBR_BEG void dsr(const char * & ptr,const char * end) {
#define FG_DSR_MBR2(A,B) FG_DSR_MBR_BEG fgDsr(ptr,end,A); fgDsr(ptr,end,B); }
#define FG_DSR_MBR3(A,B,C) FG_DSR_MBR_BEG fgDsr(ptr,end,A); fgDsr(ptr,end,B); fgDsr(ptr,end,C); }

#define FG_SER2(A,B) FG_SER_MBR2(A,B) FG_DSR_MBR2(A,B)
#define FG_SER3(A,B,C) FG_SER_MBR3(A,B,C) FG_DSR_MBR3(A,B,C)

// Serialize to a message, which is just a magic number followed by simple serialization.
// Note that the magic number must be a 64 bit unsigned literal:
#define FG_SER_MSG(HEX64) string serMsg() const {string ret = fgSer(HEX64); ret.append(ser()); return ret; } \
    void dsrMsg(const string & msg) {const char *ptr = &msg[0],*end = ptr+msg.size(); uint64 ver; \
    fgDsr(ptr,end,ver); FGASSERT(ver == HEX64); dsr(ptr,end); }

#endif
