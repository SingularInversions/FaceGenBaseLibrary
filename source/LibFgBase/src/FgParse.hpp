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
// Use this instead of useless std::getline which leaves in CR characters on Windows.
FgStrs
fgSplitLines(const string & src,bool backslashContinuation=true);

FgUintss
fgSplitLines(const FgUints & src,bool includeEmptyLines=false);

FgStrings
fgSplitLinesUtf8(const string & utf8,bool includeEmptyLines=false);

// The exact UTF-8 string between commas will be taken as the value except for:
// * Newlines, which are interpreted as the start of a new record.
// * When a quote directly follows a comma, in which case everthing, including commas and newlines,
//   up until the next single-quote is taken as the value, and double-quotes are taken as single-quotes.
FgStrss
fgLoadCsv(const FgString & fname);

// Quotes all fields and uses double-quotes to escape quote literals.
void
fgSaveCsv(const FgString & fname,const FgStrss & csvLines);

// Split up a string based on a seperator.
// Output does not include separators or (by default) empty strings.
// More convenient than boost::split
FgStrs
fgSplitChar(const string & str,char sep=' ',bool includeEmptyStrings=false);

// Breaks the given string into a vector of strings according to any whitespace, which is 
// removed. Quotation marks can be used to enclose symbols containing whitespace.
FgStrs
fgWhiteBreak(const string &);

// Map all non-ASCII characters that look like an ASCII character (homoglyphs) to that ASCII character,
// and all others to '?':
string
fgAsciify(const string &);

#endif
