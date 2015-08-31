//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     June 28, 2007
//
// Unicode filename fstreams and other conveniences.
//
// NOTES:
//
// In C++ the '\n' character has value 0x0A (LF) on both windows and *nix.
//
// When using text mode ostream on Windows this is converted to the 2 byte code CR LF
// (0x0D 0x0A) or ('\r' '\n'). On *nix it is left unchanged.
//
// When using text mode istream on either platform, any of the various EOL formats are converted to '\n'.
//

#ifndef FGSTDSTREAM_HPP
#define FGSTDSTREAM_HPP

#include "FgStdLibs.hpp"
#include "FgString.hpp"

struct  FgOfstream : public std::ofstream
{
    FgOfstream()
    {}

    explicit
    FgOfstream(
        const FgString &        fname,
        bool                    throwOnFail=true,
        // ios::app     Append
        // ios::out     Default (specify to negate ios::binary)
        // ios::binary  Don't do obscure text conversions
        std::ios::openmode      mode = std::ios::binary)
    {open(fname,throwOnFail,mode); }

    bool
    open(
        const FgString &        fname,
        bool                    throwOnFail=true,
        // ios::app     Append
        // ios::out     Default (no need to specify)
        // ios::binary  Don't do obscure text conversions
        std::ios::openmode      mode = std::ios::binary);

    template<class T>
    void
    writeb(const T & val)
    {write(reinterpret_cast<const char*>(&val),sizeof(val)); }
};

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
fgRead(
    std::istream &  ii,
    T &             val)
{
    ii.read(reinterpret_cast<char*>(&val),sizeof(T));
}

template<class T>
void
fgWrite(
    std::ostream &  os,
    const T &       val)
{
    os.write(reinterpret_cast<const char*>(&val),sizeof(T));
}

// Useful for redirecting global overload of operator<< to member print() function:
#define FG_OSTREAM_PRINT(Class)                                                 \
inline std::ostream &                                                           \
operator<<(std::ostream & ss,const Class & obj)                                 \
{return obj.print(ss); }


#endif

// */
