//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     June 25, 2009
//
// Handy way to serialize to/from files
//
// * Terminology: load/save is used for whole files, read/write is used for streams.
// * All serialization formats can change between boost versions breaking application back-compatibility.
// * Binary (non-portable) serialization formats differ between 32/64 bit even when bit size of contents
//   is explicitly specified.

#ifndef FGMETAFORMAT_HPP
#define FGMETAFORMAT_HPP

#include "FgStdLibs.hpp"
#include "FgFileSystem.hpp"
#include "FgString.hpp"
#include "FgSerialize.hpp"
#include "FgStdStream.hpp"

// Returns true unless throwOnFail==false and failure occured:
template<class Archive,class T>
bool
fgLoadDeserial(
    const FgString &    filename,
    T &                 val,
    bool                throwOnFail)
{
    try
    {
        FgIfstream      ifs(filename);
        Archive         ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(val);
    }
    catch(FgException &)
    {
        if (throwOnFail)
            throw;
        else
            return false;
    }
    catch(...)
    {
        if (throwOnFail)
            fgThrow("Error while deserializing from file",filename);
        else
            return false;
    }
    return true;
}

template<class T>
inline bool
fgLoadText(const FgString & filename,T & val,bool throwOnFail=true)
{return fgLoadDeserial<boost::archive::text_iarchive>(filename,val,throwOnFail); }

template<class T>
inline bool
fgLoadXml(const FgString & filename,T & val,bool throwOnFail=true)
{return fgLoadDeserial<boost::archive::xml_iarchive>(filename,val,throwOnFail); }

template<class T>
inline bool
fgLoadBin(const FgString & filename,T & val,bool throwOnFail=true)
{return fgLoadDeserial<boost::archive::binary_iarchive>(filename,val,throwOnFail); }

template<class T>
inline bool
fgLoadPBin(const FgString & filename,T & val,bool throwOnFail=true)
{return fgLoadDeserial<portable_binary_iarchive>(filename,val,throwOnFail); }

template<class Archive,class T>
bool
fgSaveSerial(
    const FgString &    filename,
    const T &           val,
    bool                throwOnFail)
{
    try
    {
        FgOfstream      ofs(filename);
        Archive         oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(val);
    }
    catch(FgException & e)
    {
        if (throwOnFail)
            throw;
        fgout << "ERROR (FG exception): " << e.tr_message();
        return false;
    }
    catch(std::exception & e)
    {
        if (throwOnFail)
            throw;
        fgout << "ERROR (std::exception): " << e.what();
        return false;
    }
    catch(...)
    {
        if (throwOnFail)
            fgThrow("Error while serializing to file",filename);
        fgout << "ERROR (unknown exception)";
        return false;
    }
    return true;
}

template<class T>
inline bool
fgSaveText(const FgString & filename,const T & val,bool throwOnFail=true)
{return fgSaveSerial<boost::archive::text_oarchive>(filename,val,throwOnFail); }

template<class T>
inline bool
fgSaveXml(const FgString & filename,const T & val,bool throwOnFail=true)
{return fgSaveSerial<boost::archive::xml_oarchive>(filename,val,throwOnFail); }

template<class T>
inline bool
fgSaveBin(const FgString & filename,const T & val,bool throwOnFail=true)
{return fgSaveSerial<boost::archive::binary_oarchive>(filename,val,throwOnFail); }

template<class T>
inline bool
fgSavePBin(const FgString & filename,const T & val,bool throwOnFail=true)
{return fgSaveSerial<portable_binary_oarchive>(filename,val,throwOnFail); }

#endif
