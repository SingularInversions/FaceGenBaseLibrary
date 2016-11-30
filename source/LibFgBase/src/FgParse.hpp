//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     April 21, 2005
//
// Simple, slow, correct string parsing (multi-pass approach).
//

#ifndef FGPARSE_HPP
#define FGPARSE_HPP

#include "FgStdVector.hpp"
#include "FgStdString.hpp"
#include "FgString.hpp"

// Split a string into non-empty lines at CR/LF and remove all CR/LF characters.
// Respects backslash+CR/LF line continuation.
// Use this instead of useless std::getline which leaves in CR characters on Windows.
vector<string>
fgSplitLines(const string & src);

vector<vector<uint32> >
fgSplitLines(const vector<uint32> & src,bool includeEmptyLines=false);

FgStrings
fgSplitLinesUtf8(const string & utf8,bool includeEmptyLines=false);

// Quotations not currently supported. White space is kept. CR/LFs removed.
vector<vector<string> >
fgReadCsvFile(const FgString & fname);

// Split up a string based on a seperator.
// Output does not include separators or (by default) empty strings.
// More convenient than boost::split
vector<string>
fgSplitChar(const string & str,char sep=' ',bool includeEmptyStrings=false);

// Breaks the given string into a vector of strings according to any whitespace, which is 
// removed. Quotation marks can be used to enclose symbols containing whitespace.
vector<string>
fgWhiteBreak(const string &);

#endif
