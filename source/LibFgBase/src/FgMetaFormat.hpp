//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
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

namespace Fg {

template<class Archive>     String fgArchiveString();
template<> inline           String fgArchiveString<boost::archive::text_iarchive>() {return "Text"; }
template<> inline           String fgArchiveString<boost::archive::xml_iarchive>() {return "XML"; }
template<> inline           String fgArchiveString<boost::archive::binary_iarchive>() {return "Binary"; }
template<> inline           String fgArchiveString<portable_binary_iarchive>() {return "PBin"; }

// Returns true unless throwOnFail==false and failure occured:
template<class Archive,class T>
bool
fgLoadDeserial(
    const Ustring &    filename,
    T &                 val,
    bool                throwOnFail)
{
    // If client doesn't want exceptions and the file doesn't exist, avoid an internal exception
    // polluting the debug log:
    if (!throwOnFail && !pathExists(filename))
        return false;
    try
    {
        Ifstream      ifs(filename);
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
            fgThrow("Error while deserializing "+String(typeid(T).name())+" from "+fgArchiveString<Archive>()+" file",filename);
        else
            return false;
    }
    return true;
}

template<class T>
inline bool
fgLoadText(const Ustring & filename,T & val,bool throwOnFail=true)
{return fgLoadDeserial<boost::archive::text_iarchive>(filename,val,throwOnFail); }

template<class T>
inline bool
fgLoadXml(const Ustring & filename,T & val,bool throwOnFail=true)
{return fgLoadDeserial<boost::archive::xml_iarchive>(filename,val,throwOnFail); }

template<class T>
inline bool
fgLoadBin(const Ustring & filename,T & val,bool throwOnFail=true)
{return fgLoadDeserial<boost::archive::binary_iarchive>(filename,val,throwOnFail); }

template<class T>
inline bool
fgLoadPBin(const Ustring & filename,T & val,bool throwOnFail=true)
{return fgLoadDeserial<portable_binary_iarchive>(filename,val,throwOnFail); }

template<class T>
T
fgLoadBinT(const Ustring & fname)
{
    T       ret;
    fgLoadBin(fname,ret,true);
    return ret;
}

template<class T>
T
fgLoadPBinT(const Ustring & fname)
{
    T       ret;
    fgLoadPBin(fname,ret,true);
    return ret;
}

template<class Archive,class T>
bool
fgSaveSerial(
    const Ustring &    filename,
    const T &           val,
    bool                throwOnFail)
{
    try
    {
        Ofstream      ofs(filename);
        Archive         oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(val);
    }
    catch(FgException & e)
    {
        if (throwOnFail)
            throw;
        fgout << fgnl << "ERROR (FG exception): " << e.tr_message();
        return false;
    }
    catch(std::exception & e)
    {
        if (throwOnFail)
            throw;
        fgout << fgnl << "ERROR (std::exception): " << e.what();
        return false;
    }
    catch(...)
    {
        if (throwOnFail)
            fgThrow("Error while serializing to file",filename);
        fgout << fgnl << "ERROR (unknown exception)";
        return false;
    }
    return true;
}

template<class T>
inline bool
fgSaveText(const Ustring & filename,const T & val,bool throwOnFail=true)
{return fgSaveSerial<boost::archive::text_oarchive>(filename,val,throwOnFail); }

template<class T>
inline bool
fgSaveXml(const Ustring & filename,const T & val,bool throwOnFail=true)
{return fgSaveSerial<boost::archive::xml_oarchive>(filename,val,throwOnFail); }

template<class T>
inline bool
fgSaveBin(const Ustring & filename,const T & val,bool throwOnFail=true)
{return fgSaveSerial<boost::archive::binary_oarchive>(filename,val,throwOnFail); }

template<class T>
inline bool
fgSavePBin(const Ustring & filename,const T & val,bool throwOnFail=true)
{return fgSaveSerial<portable_binary_oarchive>(filename,val,throwOnFail); }

}

#endif
