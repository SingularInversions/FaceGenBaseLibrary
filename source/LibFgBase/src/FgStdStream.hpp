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

// Handy for open-write-close. Binary mode of course:
void
fgWriteFile(const FgString & fname,const std::string & data,bool append=true);

#endif

// */
