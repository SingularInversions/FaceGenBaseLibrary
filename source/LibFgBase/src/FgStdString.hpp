//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// std::string related functions
//

#ifndef FGSTDSTRING_HPP
#define FGSTDSTRING_HPP

#include "FgStdLibs.hpp"
#include "FgTypes.hpp"
#include "FgStdVector.hpp"
#include "FgOpt.hpp"

namespace Fg {

typedef std::string             String;
typedef Svec<String>            Strings;
typedef Svec<Strings>           Stringss;
typedef Svec<std::u32string>    FgStr32s;

// More general than std::to_string since it uses operator<< which can be defined for
// user-defined types as well. Also, to_string can cause ambiguous call errors.
template<class T>
String
toString(const T & val)
{
    std::ostringstream   msg;
    msg << val;
    return msg.str();
}
template<>
inline String
toString(const String & str)
{return str; }

// Default uses standard stream input "lexical conversions".
// Only valid strings for the given type are accepted, extra characters including whitespace are errors
// Define fully specialized versions where this behaviour is not desired:
template<class T>
Opt<T>
fgFromStr(const String & str)
{
    T                   val;
    std::istringstream  iss(str);
    iss >> val;
    if (iss.fail() || (!iss.eof()))
        return Opt<T>();
    return Opt<T>(val);
}
// Only valid integer representations within the range of int32 will return a valid value, whitespace invalid:
template<> Opt<int> fgFromStr<int>(const String &);
template<> Opt<uint> fgFromStr<uint>(const String &);

// Throws if the String is not formatted correctly:
template<class T>
T
fgFromString(const String & str)
{
    Opt<T>    oval = fgFromStr<T>(str);
    if (!oval.valid())
        throw FgException("Unable to convert String to type",String(typeid(T).name())+":"+str);
    return oval.val();
}

// Ensures a minimum number of digits are printed:
template<class T>
String
fgToStringDigits(T val,uint numDigits)
{
    std::ostringstream   oss;
    oss << std::setw(numDigits) << std::setfill('0') << val;
    return oss.str();
}

// Sets the desired total number of digits (precision):
template<class T>
String
fgToStringPrecision(T val,uint precision)
{
    std::ostringstream   oss;
    oss.precision(precision);
    oss << val;
    return oss.str();
}

// Set the number of digits beyond fixed point:
String
fgToFixed(double val,uint fractionalDigits=0);

// Multiply by 100 and put a percent sign after:
String
fgToPercent(double val,uint fractionalDigits=0);

String
fgToLower(const String & s);

String
fgToUpper(const String & s);

// Returned list of strings does NOT include separators but DOES include empty
// strings where there are consecutive separators:
Svec<String>
fgSplitAtSeparators(const String & str,char sep);

String
fgReplace(const String & str,char orig,char repl);

// Pad a String to desired len (does not truncate of longer):
String
fgPad(const String & str,size_t len,char ch=' ');

// Inspired by Python join():
String
cat(const Svec<String> & strings,const String & separator);

inline
String
cat(const String & s0,const String & s1)
{
    String      ret(s0);
    ret.append(s1);
    return ret;
}

inline
String
cat(const String & s0,const String & s1,const String & s2)
{
    String      ret(s0);
    ret.append(s1);
    ret.append(s2);
    return ret;
}

// C++98 doesn't support .back() for strings:
inline
char
fgBack(const String & s)
{
    FGASSERT(!s.empty());
    return *(--s.end());
}
inline
char &
fgBack(String & s)
{
    FGASSERT(!s.empty());
    return *(--s.end());
}

template<typename T>
bool
fgContains(const std::basic_string<T> & str,T ch)
{return (str.find(ch) != std::basic_string<T>::npos); }

template<typename T>
bool
fgContains(const std::basic_string<T> & str,const std::basic_string<T> & pattern)
{return (str.find(pattern) != std::basic_string<T>::npos); }

template<typename T>
bool
fgContains(const std::basic_string<T> & str,const T * pattern_c_str)
{return fgContains(str,std::basic_string<T>(pattern_c_str)); }

template<typename T>
std::basic_string<T>
fgHead(const std::basic_string<T> & str,size_t size)
{
    FGASSERT(size <= str.size());
    return std::basic_string<T>(str.begin(),str.begin()+size);
}

template<class T>
std::basic_string<T>
fgRest(const std::basic_string<T> & str,size_t start=1)
{
    FGASSERT(start <= str.size());      // Can be size zero
    return std::basic_string<T>(str.begin()+start,str.end());
}

template<class T>
std::basic_string<T>
fgSubstr(const std::basic_string<T> & str,size_t start,size_t size)
{
    FGASSERT(start+size <= str.size());
    return  std::basic_string<T>(str.begin()+start,str.begin()+start+size);
}

// Returns at least size 1, with 1 additional for each split element:
template<class T>
Svec<std::basic_string<T> >
fgSplit(const std::basic_string<T> & str,T ch)
{
    Svec<std::basic_string<T> >  ret;
    std::basic_string<T>                ss;
    for(T c : str) {
        if (c == ch) {
            ret.push_back(ss);
            ss.clear();
        }
        else
            ss.push_back(c);
    }
    ret.push_back(ss);
    return ret;
}

template<class T>
size_t
fgFindLastIdx(const std::basic_string<T> & str,T val)
{
    for (size_t ii=str.size(); ii!=0; --ii)
        if (str[ii-1] == val)
            return ii-1;
    return str.size();
}

template<class T>
bool
fgBeginsWith(const std::basic_string<T> & base,const std::basic_string<T> & pattern)
{
    if (pattern.size() > base.size())
        return false;
    for (size_t ii=0; ii<pattern.size(); ++ii)
        if (pattern[ii] != base[ii])
            return false;
    return true;
}

template<class T>
bool
fgBeginsWith(const std::basic_string<T> & base,const T * pattern_c_str)
{return fgBeginsWith(base,std::basic_string<T>(pattern_c_str)); }

template<class T>
bool
fgEndsWith(const std::basic_string<T> & str,const std::basic_string<T> & pattern)
{
    if (pattern.size() > str.size())
        return false;
    size_t      offset = str.size() - pattern.size();
    for (size_t ii=0; ii<pattern.size(); ++ii)
        if (pattern[ii] != str[ii+offset])
            return false;
    return true;
}

template<class T>
bool
fgEndsWith(const std::basic_string<T> & str,const T * pattern_c_str)
{return fgEndsWith(str,std::basic_string<T>(pattern_c_str)); }

}

#endif
