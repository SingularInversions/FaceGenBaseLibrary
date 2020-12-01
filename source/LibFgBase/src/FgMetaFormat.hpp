//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Handy way to serialize to/from files using boost serialization / archive
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
loadBsa(
    Ustring const &     filename,
    T &                 val,
    bool                throwOnFail)
{
    // If client doesn't want exceptions and the file doesn't exist, avoid an internal exception
    // polluting the debug log:
    if (!throwOnFail && !pathExists(filename))
        return false;
    try
    {
        Ifstream        ifs(filename);
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
loadBsaText(Ustring const & filename,T & val,bool throwOnFail=true)
{return loadBsa<boost::archive::text_iarchive>(filename,val,throwOnFail); }

template<class T>
inline bool
loadBsaXml(Ustring const & filename,T & val,bool throwOnFail=true)
{return loadBsa<boost::archive::xml_iarchive>(filename,val,throwOnFail); }

template<class T>
inline bool
loadBsaBin(Ustring const & filename,T & val,bool throwOnFail=true)
{return loadBsa<boost::archive::binary_iarchive>(filename,val,throwOnFail); }

template<class T>
inline bool
loadBsaPBin(Ustring const & filename,T & val,bool throwOnFail=true)
{return loadBsa<portable_binary_iarchive>(filename,val,throwOnFail); }

template<class T>
T
loadBsaXml(Ustring const & filename)
{
    T           ret;
    loadBsaXml(filename,ret,true);
    return ret;
}

template<class T>
T
loadBsaBin(Ustring const & fname)
{
    T       ret;
    loadBsaBin(fname,ret,true);
    return ret;
}

template<class T>
T
loadBsaPBin(Ustring const & fname)
{
    T       ret;
    loadBsaPBin(fname,ret,true);
    return ret;
}

template<class Archive,class T>
bool
saveBsa(
    Ustring const &     filename,
    T const &           val,
    bool                throwOnFail)
{
    try
    {
        Ofstream        ofs(filename);
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
saveBsaText(Ustring const & filename,T const & val,bool throwOnFail=true)
{return saveBsa<boost::archive::text_oarchive>(filename,val,throwOnFail); }

template<class T>
inline bool
saveBsaXml(Ustring const & filename,T const & val,bool throwOnFail=true)
{return saveBsa<boost::archive::xml_oarchive>(filename,val,throwOnFail); }

template<class T>
inline bool
saveBsaBin(Ustring const & filename,T const & val,bool throwOnFail=true)
{return saveBsa<boost::archive::binary_oarchive>(filename,val,throwOnFail); }

template<class T>
inline bool
saveBsaPBin(Ustring const & filename,T const & val,bool throwOnFail=true)
{return saveBsa<portable_binary_oarchive>(filename,val,throwOnFail); }

}

#endif
