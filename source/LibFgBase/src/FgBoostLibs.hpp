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
// qualifier applied to function type has no meaning in path.hpp:
#pragma warning(disable:4180)
// boost function: nonstandard extension used : formal parameter 'function_ptr'
// was previously defined as a type:
#pragma warning(disable:4224)
// VS2010 has problems with boost archive converting from std::streamsize to size_t for 32 bit:
#pragma warning(disable:4244)
// boost/archive/basic_binary_oprimitive.hpp: warning C4310: cast truncates constant value
// (occurs only with vs2010 debug 32):
#pragma warning(disable:4310)
// Assignment operator could not be generated:
#pragma warning(disable:4512)

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
#include <boost/bind.hpp>
#include <boost/config.hpp>
#include <boost/detail/endian.hpp>
#include <boost/function.hpp>
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
// Only include the necessary parts of boost/thread.hpp to avoid warning C4913 (VS2010):
#include <boost/thread/barrier.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/once.hpp>
#include <boost/thread/thread.hpp>

#endif  // FGBOOSTLIBS_HPP
