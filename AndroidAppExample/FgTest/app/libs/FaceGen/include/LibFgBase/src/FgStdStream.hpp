//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     June 28, 2007
//
// Cross-platform unicode filename fstreams and other conveniences.
//
// NOTES:
//
// * More convenient to select exception throwing on a per-call basis.
// * Always use binary mode due to complexity of text mode some of which is:
// * In C++ the '\n' character has value 0x0A (LF) on both windows and *nix.
// * When using text mode ostream on Windows this is converted to the 2 byte code CR LF
//   (0x0D 0x0A) or ('\r' '\n'). On *nix it is left unchanged.
// * When using text mode istream on either platform, any of the various EOL formats are converted to '\n'.
//

#ifndef FGSTDSTREAM_HPP
#define FGSTDSTREAM_HPP

#include "FgStdLibs.hpp"
#include "FgString.hpp"

using   std::istream;
using   std::ostream;
using   std::string;
using   std::vector;

struct  FgOfstream : public std::ofstream
{
    FgOfstream()
    {}

    explicit
    FgOfstream(
        const FgString &        fname,
        bool                    append = false,
        bool                    throwOnFail = true)
    {open(fname,append,throwOnFail); }

    bool
    open(
        const FgString &        fname,
        bool                    append = false,
        bool                    throwOnFail = true);

    template<class T>
    void
    writeb(const T & val)
    {write(reinterpret_cast<const char*>(&val),sizeof(val)); }
};

// 32/64 portable file format interface (boost::serialization tends to be incompatible with past versions).
// Just casts size_t to 32 bit and assumes little-endian native and IEEE floats:

template<class T>
void
fgWriteb(ostream & os,const T & val)    // Only use for builtins !
{os.write(reinterpret_cast<const char*>(&val),sizeof(val)); }

// Handle builtins:
inline void fgWritep(ostream & os,int32 val) {fgWriteb(os,val); }
inline void fgWritep(ostream & os,uint32 val) {fgWriteb(os,val); }
inline void fgWritep(ostream & os,int64 val) {fgWriteb(os,val); }
inline void fgWritep(ostream & os,uint64 val) {fgWriteb(os,val); }
inline void fgWritep(ostream & os,float val) {fgWriteb(os,val); }
inline void fgWritep(ostream & os,double val) {fgWriteb(os,val); }
inline void fgWritep(ostream & os,bool val) {fgWriteb(os,uchar(val)); }

inline void
fgWritep(ostream & os,const string & str)
{
    fgWritep(os,uint32(str.size()));
    if (!str.empty())
        os.write(&str[0],str.size());
}

// Has to be here since FgStdStream.hpp depends on FgString.hpp
inline void
fgWritep(std::ostream & os,const FgString & s)
{fgWritep(os,s.m_str); }

template<class T>
void
fgWritep(ostream & os,const vector<T> & vec)
{
    fgWritep(os,uint32(vec.size()));        // Always store size_t as 32 bit for 32/64 portability
    if (!vec.empty())
        for (size_t ii=0; ii<vec.size(); ++ii)
            fgWritep(os,vec[ii]);
}

// INPUT:

struct  FgIfstream : public std::ifstream
{
    FgIfstream()
    {}

    explicit
    FgIfstream(
        const FgString &        fname,
        bool                    throwOnFail=true)
    {open(fname,throwOnFail); }

    bool
    open(
        const FgString &        fname,
        bool                    throwOnFail=true);

    template<typename T>
    void
    readb(T & val)
    {read(reinterpret_cast<char*>(&val),sizeof(val)); }

    template<typename T>
    T
    readt()
    {
        T       ret;
        read(reinterpret_cast<char*>(&ret),sizeof(ret));
        return ret;
    }
};

template<class T>
void
fgReadb(istream & is,T & val)
{is.read(reinterpret_cast<char*>(&val),sizeof(T)); }

template<class T>
T
fgReadt(istream & is)
{
    T       ret;
    is.read(reinterpret_cast<char*>(&ret),sizeof(ret));
    return ret;
}

// Handle builtins:
inline void fgReadp(istream & is,int32 & val) {fgReadb(is,val); }
inline void fgReadp(istream & is,uint32 & val) {fgReadb(is,val); }
inline void fgReadp(istream & is,int64 & val) {fgReadb(is,val); }
inline void fgReadp(istream & is,uint64 & val) {fgReadb(is,val); }
inline void fgReadp(istream & is,float & val) {fgReadb(is,val); }
inline void fgReadp(istream & is,double & val) {fgReadb(is,val); }
inline void fgReadp(istream & is,bool & val) {val = bool(fgReadt<uchar>(is)); }

inline void
fgReadp(istream & is,string & str)
{
    str.resize(fgReadt<uint32>(is));
    if (!str.empty())
        is.read(&str[0],str.size());
}

inline void
fgReadp(istream & is,FgString & str)
{fgReadp(is,str.m_str); }

template<class T>
void
fgReadp(istream & is,vector<T> & vec)
{
    vec.resize(fgReadt<uint32>(is));
    for (size_t ii=0; ii<vec.size(); ++ii)
        fgReadp(is,vec[ii]);
}

template<class T>
T
fgReadpT(istream & is)
{
    T       ret;
    fgReadp(is,ret);
    return ret;
}

// Handy for open-write-close:
void
fgWriteFile(const FgString & fname,const std::string & data,bool append=true);

#endif

// */
