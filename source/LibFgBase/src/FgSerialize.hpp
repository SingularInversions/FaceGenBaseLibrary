//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Andrew Beatty
//
// NB. Binary serialize doesn't work for large map<>s (as of Boost 1.52)
//

#ifndef FGSERIALIZE_HPP
#define FGSERIALIZE_HPP

#include "FgBoostLibs.hpp"
#include "portable_binary_oarchive.hpp"
#include "portable_binary_iarchive.hpp"

#define FG_SERIAL_HELPER1(v1)                                                   \
    template<class Archive>                                                     \
    void serialize(Archive & ar, const unsigned int)                            \
    {ar & BOOST_SERIALIZATION_NVP(v1)
#define FG_SERIAL_HELPER2(v1,v2) FG_SERIAL_HELPER1(v1) & BOOST_SERIALIZATION_NVP(v2)
#define FG_SERIAL_HELPER3(v1,v2,v3) FG_SERIAL_HELPER2(v1,v2) & BOOST_SERIALIZATION_NVP(v3)
#define FG_SERIAL_HELPER4(v1,v2,v3,v4) FG_SERIAL_HELPER3(v1,v2,v3) & BOOST_SERIALIZATION_NVP(v4)
#define FG_SERIAL_HELPER5(v1,v2,v3,v4,v5) FG_SERIAL_HELPER4(v1,v2,v3,v4) & BOOST_SERIALIZATION_NVP(v5)
#define FG_SERIAL_HELPER6(v1,v2,v3,v4,v5,v6) FG_SERIAL_HELPER5(v1,v2,v3,v4,v5) & BOOST_SERIALIZATION_NVP(v6)
#define FG_SERIAL_HELPER7(v1,v2,v3,v4,v5,v6,v7) FG_SERIAL_HELPER6(v1,v2,v3,v4,v5,v6) & BOOST_SERIALIZATION_NVP(v7)
#define FG_SERIAL_HELPER8(v1,v2,v3,v4,v5,v6,v7,v8) FG_SERIAL_HELPER7(v1,v2,v3,v4,v5,v6,v7) & BOOST_SERIALIZATION_NVP(v8)
#define FG_SERIAL_HELPER9(v1,v2,v3,v4,v5,v6,v7,v8,v9) FG_SERIAL_HELPER8(v1,v2,v3,v4,v5,v6,v7,v8) & BOOST_SERIALIZATION_NVP(v9)
#define FG_SERIAL_HELPER10(v1,v2,v3,v4,v5,v6,v7,v8,v9,v10) FG_SERIAL_HELPER9(v1,v2,v3,v4,v5,v6,v7,v8,v9) & BOOST_SERIALIZATION_NVP(v10)
#define FG_SERIAL_HELPER11(v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11) FG_SERIAL_HELPER10(v1,v2,v3,v4,v5,v6,v7,v8,v9,v10) & BOOST_SERIALIZATION_NVP(v11)
#define FG_SERIAL_HELPER12(v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12) FG_SERIAL_HELPER11(v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11) & BOOST_SERIALIZATION_NVP(v12)

#define FG_SERIALIZE1(v1) FG_SERIAL_HELPER1(v1) ;}
#define FG_SERIALIZE2(v1,v2) FG_SERIAL_HELPER2(v1,v2) ;}
#define FG_SERIALIZE3(v1,v2,v3) FG_SERIAL_HELPER3(v1,v2,v3) ;}
#define FG_SERIALIZE4(v1,v2,v3,v4) FG_SERIAL_HELPER4(v1,v2,v3,v4) ;}
#define FG_SERIALIZE5(v1,v2,v3,v4,v5) FG_SERIAL_HELPER5(v1,v2,v3,v4,v5) ;}
#define FG_SERIALIZE6(v1,v2,v3,v4,v5,v6) FG_SERIAL_HELPER6(v1,v2,v3,v4,v5,v6) ;}
#define FG_SERIALIZE7(v1,v2,v3,v4,v5,v6,v7) FG_SERIAL_HELPER7(v1,v2,v3,v4,v5,v6,v7) ;}
#define FG_SERIALIZE8(v1,v2,v3,v4,v5,v6,v7,v8) FG_SERIAL_HELPER8(v1,v2,v3,v4,v5,v6,v7,v8) ;}
#define FG_SERIALIZE9(v1,v2,v3,v4,v5,v6,v7,v8,v9) FG_SERIAL_HELPER9(v1,v2,v3,v4,v5,v6,v7,v8,v9) ;}
#define FG_SERIALIZE10(v1,v2,v3,v4,v5,v6,v7,v8,v9,v10) FG_SERIAL_HELPER10(v1,v2,v3,v4,v5,v6,v7,v8,v9,v10) ;}
#define FG_SERIALIZE11(v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11) FG_SERIAL_HELPER11(v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11) ;}
#define FG_SERIALIZE12(v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12) FG_SERIAL_HELPER12(v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12) ;}

// Use these macros for migrating stored data to a new structure by having different
// members in the input and output instantiations (note this doesn't work for xml archives
// as those also store a code for the class name):
#define FG_SER_MIGRATE_HELPER1(Ar,v1)                                      \
    void serialize(Ar & ar, const unsigned int)                            \
    {ar & BOOST_SERIALIZATION_NVP(v1)
#define FG_SER_MIGRATE_HELPER2(Ar,v1,v2) FG_SER_MIGRATE_HELPER1(Ar,v1) & BOOST_SERIALIZATION_NVP(v2)
#define FG_SER_MIGRATE_HELPER3(Ar,v1,v2,v3) FG_SER_MIGRATE_HELPER2(Ar,v1,v2) & BOOST_SERIALIZATION_NVP(v3)
#define FG_SER_MIGRATE_HELPER4(Ar,v1,v2,v3,v4) FG_SER_MIGRATE_HELPER3(Ar,v1,v2,v3) & BOOST_SERIALIZATION_NVP(v4)
#define FG_SER_MIGRATE_HELPER5(Ar,v1,v2,v3,v4,v5) FG_SER_MIGRATE_HELPER4(Ar,v1,v2,v3,v4) & BOOST_SERIALIZATION_NVP(v5)
#define FG_SER_MIGRATE_HELPER6(Ar,v1,v2,v3,v4,v5,v6) FG_SER_MIGRATE_HELPER5(Ar,v1,v2,v3,v4,v5) & BOOST_SERIALIZATION_NVP(v6)
#define FG_SER_MIGRATE_HELPER7(Ar,v1,v2,v3,v4,v5,v6,v7) FG_SER_MIGRATE_HELPER6(Ar,v1,v2,v3,v4,v5,v6) & BOOST_SERIALIZATION_NVP(v7)
#define FG_SER_MIGRATE_HELPER8(Ar,v1,v2,v3,v4,v5,v6,v7,v8) FG_SER_MIGRATE_HELPER7(Ar,v1,v2,v3,v4,v5,v6,v7) & BOOST_SERIALIZATION_NVP(v8)

#define FG_SER_MIGRATE1(Ar,v1) FG_SER_MIGRATE_HELPER1(Ar,v1) ;}
#define FG_SER_MIGRATE2(Ar,v1,v2) FG_SER_MIGRATE_HELPER2(Ar,v1,v2) ;}
#define FG_SER_MIGRATE3(Ar,v1,v2,v3) FG_SER_MIGRATE_HELPER3(Ar,v1,v2,v3) ;}
#define FG_SER_MIGRATE4(Ar,v1,v2,v3,v4) FG_SER_MIGRATE_HELPER4(Ar,v1,v2,v3,v4) ;}
#define FG_SER_MIGRATE5(Ar,v1,v2,v3,v4,v5) FG_SER_MIGRATE_HELPER5(Ar,v1,v2,v3,v4,v5) ;}
#define FG_SER_MIGRATE6(Ar,v1,v2,v3,v4,v5,v6) FG_SER_MIGRATE_HELPER6(Ar,v1,v2,v3,v4,v5,v6) ;}
#define FG_SER_MIGRATE7(Ar,v1,v2,v3,v4,v5,v6,v7) FG_SER_MIGRATE_HELPER7(Ar,v1,v2,v3,v4,v5,v6,v7) ;}
#define FG_SER_MIGRATE8(Ar,v1,v2,v3,v4,v5,v6,v7,v8) FG_SER_MIGRATE_HELPER8(Ar,v1,v2,v3,v4,v5,v6,v7,v8) ;}

template<class T>
void
fgSerialize(const T & val,std::string & blob)
{
    std::ostringstream              os;
    boost::archive::binary_oarchive oa(os);
    oa << BOOST_SERIALIZATION_NVP(val);
    blob = os.str();
}

template<class T>
inline
std::string
fgSerialize(const T & val)
{
    std::string blob;
    fgSerialize(val,blob);
    return blob;
}

template<class T>
void
fgDeserialize(const std::string & blob,T & val)
{
    std::istringstream              is(blob);
    boost::archive::binary_iarchive ia(is);
    ia >> BOOST_SERIALIZATION_NVP(val);
}

template<class T>
void
fgSerializePort(const T & val,std::string & blob)
{
    std::ostringstream          os;
    portable_binary_oarchive    oa(os);
    oa << BOOST_SERIALIZATION_NVP(val);
    blob = os.str();
}

template<class T>
inline
std::string
fgSerializePort(const T & val)
{
    std::string blob;
    fgSerializePort(val,blob);
    return blob;
}

template<class T>
void
fgDeserializePort(const std::string & blob,T & val)
{
    std::istringstream          is(blob);
    portable_binary_iarchive    ia(is);
    ia >> BOOST_SERIALIZATION_NVP(val);
}

#endif
