//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

// Only the 'libs' and 'boost' subdirectories actually used are included from boost
// (which still adds up to quite a few due to dependencies).
//
// When updating to a newer version of boost, copy the 'libs/serialization/example/portable*'
// source files to 'LibFgBase' (if they've changed) and add in the '#include "stdfx.h"'.
// The serialization lib also needs a couple of redundant .cpp files removed.

#ifndef FGBOOSTLIBS_HPP
#define FGBOOSTLIBS_HPP

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
#include <boost/filesystem/operations.hpp>
// These includes are required for VS13 not to break with boost::serialize:
#include <boost/functional.hpp>
#include <boost/functional/hash.hpp>
// Somehow prevents boost from enabling VS warning 4242 which causes loads of warnings in this code:
#include <boost/lexical_cast.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/item_version_type.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/array.hpp> 
#include <boost/serialization/map.hpp> 
#include <boost/serialization/string.hpp>
#include <boost/serialization/throw_exception.hpp>
#include <boost/serialization/vector.hpp>

#endif  // FGBOOSTLIBS_HPP

