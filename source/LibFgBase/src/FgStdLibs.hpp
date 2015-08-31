//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Jan 26, 2007
//

#ifndef FGSTDLIBS_HPP
#define FGSTDLIBS_HPP

// In case client includes Microsoft libraries before this:
#undef max
#undef min

// C standard libraries. We use the 'c' prefixed versions which put the original
// versions into the 'std::' namespace. Note that they also define 'using' for each
// name so we don't have to use the namespace prefix (or 'using namespace std') -
// however it can be used to avoid conflicts:
#include <cctype>
#include <cfloat>
#include <climits>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ctime>

// C++ standard libraries:

#ifdef _MSC_VER
// Some MSVC standard libraries trigger this warning after release optimizations:
#pragma warning (disable: 4702)
#endif

#include <vector>
#include <algorithm>
#include <exception>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <map>
#include <set>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string>

#endif

// */
