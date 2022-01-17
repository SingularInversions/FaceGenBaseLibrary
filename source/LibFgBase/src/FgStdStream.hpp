//
// Coypright (c) 2022 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Cross-platform unicode filename fstreams and other conveniences.
//
// NOTES:
//
// * More convenient to select exception throwing on a per-call basis.
// * Always use binary mode due to complexity of text mode some of which is:
// * In C++ the '\n' character has value 0x0A (LF) on both windows and *nix.
// * When using text mode std::ostream on Windows this is converted to the 2 byte code CR LF
//   (0x0D 0x0A) or ('\r' '\n'). On *nix it is left unchanged.
// * When using text mode std::istream on either platform, any of the various EOL formats are converted to '\n'.
//

#ifndef FGSTDSTREAM_HPP
#define FGSTDSTREAM_HPP

#include "FgStdLibs.hpp"
#include "FgString.hpp"

namespace Fg {

struct  Ofstream : public std::ofstream
{
    Ofstream()
    {}

    explicit
    Ofstream(
        String8 const &         fname,
        bool                    appendFile = false,
        bool                    throwOnFail = true)
    {open(fname,appendFile,throwOnFail); }

    bool
    open(
        String8 const &         fname,
        bool                    appendFile = false,
        bool                    throwOnFail = true);

    // raw binary output of simple data types:
    template<class T>
    void
    writeb(T const & val)
    {write(reinterpret_cast<char const*>(&val),sizeof(val)); }

    // raw binary output from string:
    void
    writeBytes(String const & s)
    {
        if (!s.empty())
            write(s.data(),s.size());
    }
};

// 32/64 portable file format interface (boost::serialization tends to be incompatible with past versions).
// Just casts size_t to 32 bit and assumes little-endian native and IEEE floats:

template<class T>
void
fgWriteb(std::ostream & os,T const & val)    // Only use for builtins !
{os.write(reinterpret_cast<char const*>(&val),sizeof(val)); }

// Handle builtins:
inline void fgWritep(std::ostream & os,int32 val) {fgWriteb(os,val); }
inline void fgWritep(std::ostream & os,uint32 val) {fgWriteb(os,val); }
inline void fgWritep(std::ostream & os,int64 val) {fgWriteb(os,val); }
inline void fgWritep(std::ostream & os,uint64 val) {fgWriteb(os,val); }
inline void fgWritep(std::ostream & os,float val) {fgWriteb(os,val); }
inline void fgWritep(std::ostream & os,double val) {fgWriteb(os,val); }
inline void fgWritep(std::ostream & os,bool val) {fgWriteb(os,uchar(val)); }

inline void
fgWritep(std::ostream & os,String const & str)
{
    fgWritep(os,uint32(str.size()));
    if (!str.empty())
        os.write(&str[0],str.size());
}

// Has to be here since FgStdStream.hpp depends on FgString.hpp
inline void
fgWritep(std::ostream & os,String8 const & s)
{fgWritep(os,s.m_str); }

template<class T>
void
fgWritep(std::ostream & os,Svec<T> const & vec)
{
    fgWritep(os,uint32(vec.size()));        // Always store size_t as 32 bit for 32/64 portability
    if (!vec.empty())
        for (size_t ii=0; ii<vec.size(); ++ii)
            fgWritep(os,vec[ii]);
}

// INPUT:

struct  Ifstream : public std::ifstream
{
    Ifstream()
    {}

    explicit
    Ifstream(
        String8 const &        fname,
        bool                    throwOnFail=true)
    {open(fname,throwOnFail); }

    bool
    open(
        String8 const &        fname,
        bool                    throwOnFail=true);

    template<typename T>
    void
    readb_(T & val)
    {read(reinterpret_cast<char*>(&val),sizeof(val)); }

    template<typename T>
    T
    readb()
    {
        T       ret;
        read(reinterpret_cast<char*>(&ret),sizeof(ret));
        return ret;
    }

    String      readChars(size_t num)
    {
        String          ret(num,' ');
        if (num > 0)
            read(&ret[0],num);
        return ret;
    }
};

template<class T>
void
readb(std::istream & is,T & val)
{is.read(reinterpret_cast<char*>(&val),sizeof(T)); }

template<class T>
T
fgReadt(std::istream & is)
{
    T       ret;
    is.read(reinterpret_cast<char*>(&ret),sizeof(ret));
    return ret;
}

// Handle builtins:
inline void fgReadp(std::istream & is,int32 & val) {readb(is,val); }
inline void fgReadp(std::istream & is,uint32 & val) {readb(is,val); }
inline void fgReadp(std::istream & is,int64 & val) {readb(is,val); }
inline void fgReadp(std::istream & is,uint64 & val) {readb(is,val); }
inline void fgReadp(std::istream & is,float & val) {readb(is,val); }
inline void fgReadp(std::istream & is,double & val) {readb(is,val); }
inline void fgReadp(std::istream & is,bool & val) {val = bool(fgReadt<uchar>(is)); }
// MSVC and Android do not consider size_t to be its own type but others do:
#ifdef _MSC_VER
#elif defined(__ANDROID__)
#else
inline void fgWritep(std::ostream & os,size_t val) {fgWriteb(os,uint64(val)); }
inline void fgReadp(std::istream & is,size_t & val) {uint64 tmp; readb(is,tmp); val = size_t(tmp); }
#endif

inline void
fgReadp(std::istream & is,String & str)
{
    str.resize(fgReadt<uint32>(is));
    if (!str.empty())
        is.read(&str[0],str.size());
}

inline void
fgReadp(std::istream & is,String8 & str)
{fgReadp(is,str.m_str); }

template<class T>
void
fgReadp(std::istream & is,Svec<T> & vec)
{
    vec.resize(fgReadt<uint32>(is));
    for (size_t ii=0; ii<vec.size(); ++ii)
        fgReadp(is,vec[ii]);
}

template<class T>
T
fgReadpT(std::istream & is)
{
    T       ret;
    fgReadp(is,ret);
    return ret;
}

}

#endif

// */
