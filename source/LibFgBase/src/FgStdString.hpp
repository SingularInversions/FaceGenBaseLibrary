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
#include "FgOpt.hpp"

using std::string;

typedef std::vector<std::string>    FgStrs;
typedef std::vector<FgStrs>         FgStrss;

// More general than std::to_string since it uses operator<< which can be defined for
// user-defined types as well. Also, to_string can cause ambiguous call errors.
// Defined here since this 'FgStdString' has more derived include dependencies and 
// we need to include specializations for 'string' as well as 'FgString':
template<class T>
std::string
fgToStr(const T & val)
{
    std::ostringstream   msg;
    msg << val;
    return msg.str();
}
template<>
inline std::string
fgToStr(const std::string & str)
{return str; }
template<>
inline std::string
fgToStr(const FgString & str)
{return str.m_str; }

// Default uses standard stream input "lexical conversions".
// Only valid strings for the given type are accepted, extra characters including whitespace are errors
// Define fully specialized versions where this behaviour is not desired:
template<class T>
FgOpt<T>
fgFromStr(const string & str)
{
    T                   val;
    std::istringstream  iss(str);
    iss >> val;
    if (iss.fail() || (!iss.eof()))
        return FgOpt<T>();
    return FgOpt<T>(val);
}
// Only valid integer representations within the range of int32 will return a valid value, whitespace invalid:
template<> FgOpt<int> fgFromStr<int>(const string &);
template<> FgOpt<uint> fgFromStr<uint>(const string &);

// Throws if the string is not formatted correctly:
template<class T>
T
fgFromString(const string & str)
{
    FgOpt<T>    oval = fgFromStr<T>(str);
    if (!oval.valid())
        fgThrow("Unable to convert string to " + string(typeid(T).name()),str);
    return oval.val();
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

// Multiply by 100 and put a percent sign after:
string
fgToPercent(double val,uint fractionalDigits=0);

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
fgCat(const vector<string> & strings,const string & separator);

inline
string
fgCat(const string & s0,const string & s1)
{
    string      ret(s0);
    ret.append(s1);
    return ret;
}

inline
string
fgCat(const string & s0,const string & s1,const string & s2)
{
    string      ret(s0);
    ret.append(s1);
    ret.append(s2);
    return ret;
}

// C++98 doesn't support .back() for strings:
inline
char
fgBack(const string & s)
{
    FGASSERT(!s.empty());
    return *(--s.end());
}
inline
char &
fgBack(string & s)
{
    FGASSERT(!s.empty());
    return *(--s.end());
}

bool
fgContains(const std::string & str,char c);

inline
bool
fgContains(const std::string & str,const std::string & pattern)
{return (str.find(pattern) != string::npos); }

#endif
