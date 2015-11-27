//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Dec 29, 2004
//
// std::string related functions
//

#ifndef FGSTDSTRING_HPP
#define FGSTDSTRING_HPP

#include "FgStdLibs.hpp"
#include "FgTypes.hpp"
#include "FgStdVector.hpp"

using std::string;

// Breaks the given string into a vector of strings according to any whitespace, which is 
// removed. Quotation marks can be used to enclose symbols containing whitespace.
std::vector<string>
fgWhiteBreak(const string &);

// More general than std::to_string since it uses operator<< which can be defined for
// user-defined types as well. Also, to_string can cause ambiguous call errors:
template<class T>
string
fgToString(const T &val)
{
    std::ostringstream   msg;
    msg << val;
    return msg.str();
}

template<class T>
T
fgFromString(const string & str)
{
    std::istringstream  iss(str);
    T   ret;
    iss >> ret;
    return ret;
}

// Ensures a minimum number of digits are printed:
template<class T>
string
fgToStringDigits(T val,uint numDigits)
{
    std::ostringstream   oss;
    oss << std::setw(numDigits) << std::setfill('0') << val;
    return oss.str();
}

// Sets the desired total number of digits (precision):
template<class T>
string
fgToStringPrecision(T val,uint precision)
{
    std::ostringstream   oss;
    oss.precision(precision);
    oss << val;
    return oss.str();
}

// Set the number of digits beyond fixed point:
string
fgToFixed(double val,uint fractionalDigits=0);

string
fgToLower(const string & s);

string
fgToUpper(const string & s);

// Returned list of strings does NOT include separators but DOES include empty
// strings where there are consecutive separators:
std::vector<string>
fgSplitAtSeparators(
    const string & str,
    char                sep);

bool
fgStartsWith(
    const string & str,
    const string & pattern);

bool
fgEndsWith(
    const string & str,
    const string & pattern);

string
fgReplace(
    const string & str,
    char                orig,
    char                repl);

// Pad a string to desired len (does not truncate of longer):
string
fgPad(const string & str,size_t len,char ch=' ');

// Inspired by Python join():
string
fgConcat(const vector<string> & strings,const string & separator);

#endif
