//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Sohail Somani, Andrew Beatty
//
// Only the 'libs' and 'boost' subdirectories actually used are included from boost
// (which still adds up to quite a few due to dependencies).
//
// When updating to a newer version of boost, copy the 'libs/serialization/example/portable*'
// source files to 'LibFgBase' (if they've changed) and add in the '#include "stdfx.h"'.
// The serialization lib also needs a couple of redundant .cpp files removed.

#ifndef FGBOOSTLIBS_HPP
#define FGBOOSTLIBS_HPP

#ifdef _MSC_VER
    #ifndef _WIN64
        // Make boost::bind also work with win32 __stdcall (__cdecl is default).
        // win64 uses a single calling convention so use of below would result in ambiguity.
        #define BOOST_MEM_FN_ENABLE_STDCALL
    #endif
    #pragma warning(push,0)
#endif

#include <boost/any.hpp>
#include <boost/archive/archive_exception.hpp>
#include <boost/archive/basic_archive.hpp>
#include <boost/archive/basic_binary_oprimitive.hpp>
#include <boost/archive/basic_binary_iprimitive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/array.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/config.hpp>
#include <boost/detail/endian.hpp>
#include <boost/functional.hpp>
#include <boost/functional/hash.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/item_version_type.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/map.hpp> 
#include <boost/serialization/string.hpp>
#include <boost/serialization/throw_exception.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/static_assert.hpp>
#include <boost/variant.hpp>

#ifdef _MSC_VER
    #pragma warning(pop)
    // Currently gets triggered by boost portable binary archive on VS2015 so hard to remove for now:
    #pragma warning(disable:4800)
#endif

#endif  // FGBOOSTLIBS_HPP
