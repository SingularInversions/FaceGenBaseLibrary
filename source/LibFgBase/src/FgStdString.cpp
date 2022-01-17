//
// Coypright (c) 2022 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgStdString.hpp"
#include "FgTypes.hpp"
#include "FgString.hpp"

using namespace std;

namespace Fg {

String32
toUtf32(string const & str)
{
    // https://stackoverflow.com/questions/38688417/utf-conversion-functions-in-c11
#ifdef _MSC_VER
    if (str.empty())
        return String32();
    wstring_convert<codecvt_utf8<int32_t>,int32_t>  convert;
    auto            asInt = convert.from_bytes(str);
    return String32(reinterpret_cast<char32_t const *>(asInt.data()),asInt.size());
#else
    wstring_convert<codecvt_utf8<char32_t>,char32_t> convert;
    return convert.from_bytes(str);
#endif
}

String32
toUtf32(char const * str)
{
    // https://stackoverflow.com/questions/38688417/utf-conversion-functions-in-c11
#ifdef _MSC_VER
    if (str[0] == 0)
        return String32{};
    wstring_convert<codecvt_utf8<int32_t>,int32_t>  convert;
    auto            asInt = convert.from_bytes(str);
    return String32(reinterpret_cast<char32_t const *>(asInt.data()),asInt.size());
#else
    wstring_convert<codecvt_utf8<char32_t>,char32_t> convert;
    return convert.from_bytes(str);
#endif
}

string
toUtf8(String32 const & in)
{
    // https://stackoverflow.com/questions/38688417/utf-conversion-functions-in-c11
#ifdef _MSC_VER
    if (in.empty())
        return string();
    wstring_convert<codecvt_utf8<int32_t>,int32_t>    convert;
    auto            ptr = reinterpret_cast<const int32_t *>(in.data());
    return convert.to_bytes(ptr,ptr+in.size());
#else
    wstring_convert<codecvt_utf8<char32_t>,char32_t>    convert;
    return convert.to_bytes(in);
#endif
}

string
toUtf8(const char32_t & utf32_char)
{return toUtf8(String32(1,utf32_char)); }

std::wstring
toUtf16(const std::string & utf8)
{
    wstring_convert<codecvt_utf8_utf16<wchar_t> >   converter;
    return converter.from_bytes(utf8);
}

std::string
toUtf8(const std::wstring & utf16)
{
    // codecvt_utf8_utf16 inherits from codecvt
    wstring_convert<codecvt_utf8_utf16<wchar_t> >   converter;
    return converter.to_bytes(utf16);
}

string
toStrFixed(double val,uint fractionalDigits)
{
    ostringstream   os;
    os << std::fixed << std::setprecision(fractionalDigits) << val;
    return os.str();
}

string
toStrPercent(double val,uint fractionalDigits)
{return toStrFixed(val*100.0,fractionalDigits) + "%"; }

std::string
toLower(const std::string & s)
{
    string  retval;
    retval.reserve(s.size());
	// Can't use std::transform since 'tolower' causes warning by converting int->char
    // std::transform(s.begin(),s.end(),retval.begin(),::tolower);
	for (char ch : s)
		retval.push_back(char(std::tolower(ch)));
    return retval;
}

std::string
toUpper(const std::string & s)
{
    string  retval;
    retval.reserve(s.size());
	// Can't use std::transform since 'tolower' causes warning by converting int->char
	// std::transform(s.begin(),s.end(),retval.begin(),::toupper);
	for (char ch : s)
		retval.push_back(char(std::toupper(ch)));
	return retval;
}

std::vector<string>
splitAtSeparators(
    const std::string & str,
    char                sep)
{
    vector<string>  ret;
    string          tmp;
    string::const_iterator  it = str.begin();
    while (it != str.end())
    {
        if (*it == sep)
        {
            ret.push_back(tmp);
            tmp.clear();
            it++;
        }
        else
            tmp += *it++;
    }
    if (!tmp.empty())
        ret.push_back(tmp);
    return ret;
}

std::string
replaceAll(
    const std::string & str,
    char                orig,
    char                repl)
{
    std::string     ret = str;
    for (size_t ii=0; ii<ret.size(); ++ii)
        if (ret[ii] == orig)
            ret[ii] = repl;
    return ret;
}

string
padToLen(string const & str,size_t len,char ch)
{
    string  ret = str;
    if (len > str.size())
        ret.resize(len,ch);
    return ret;
}

string
cat(Strings const & strings,string const & separator)
{
    string      ret;
    for (size_t ii=0; ii<strings.size(); ++ii) {
        ret += strings[ii];
        if (ii < strings.size()-1)
            ret += separator;
    }
    return ret;
}

string
cat(set<string> const & strings,string const & separator)
{
    auto                it = strings.begin();
    if (it == strings.end())
        return string{};
    string              ret {*it};
    while (++it != strings.end())
        ret += separator + *it;
    return ret;
}

String          catDeref(Ptrs<String> const & stringPtrs,String const & separator)
{
    String              ret;
    size_t              sz = stringPtrs.size();
    for (size_t ii=0; ii<sz; ++ii) {
        ret.append(*stringPtrs[ii]);
        if (ii+1 < sz)
            ret.append(separator);
    }
    return ret;
}

template<>
Opt<int>
fromStr(string const & str)
{
    Opt<int>              ret;
    if (str.empty())
        return ret;
    bool                    neg = false;
    string::const_iterator  it = str.begin();
    if (*it == '-') {
        neg = true;
        ++it;
    }
    else if (*it == '+')
        ++it;
    int64                   acc = 0;
    while (it != str.end()) {
        int                 digit = int(*it++) - int('0');
        if ((digit < 0) || (digit > 9))
            return ret;
        acc = 10 * acc + digit;
        if (acc > numeric_limits<int>::max())   // Negative mag can be one greater but who cares.
            return ret;
    }
    acc = neg ? -acc : acc;
    ret = int(acc);
    return ret;
}

template<>
Opt<uint>
fromStr<uint>(string const & str)
{
    Opt<uint>             ret;
    if (str.empty())
        return ret;
    string::const_iterator  it = str.begin();
    if (*it == '+')
        ++it;
    uint64                  acc = 0;
    while (it != str.end()) {
        int                 digit = int(*it++) - int('0');
        if ((digit < 0) || (digit > 9))
            return ret;
        acc = 10 * acc + uint64(digit);
        if (acc > numeric_limits<uint>::max())
            return ret;
    }
    ret = uint(acc);
    return ret;
}

String
cRest(String const & str,size_t start)
{
    FGASSERT1(start <= str.size(),str+"@"+toStr(start));      // Can be size zero
    return String{str.begin()+start,str.end()};
}

String32
cRest(String32 const & str,size_t start)
{
    FGASSERT1(start <= str.size(),toUtf8(str)+"@"+toStr(start));
    return String32{str.begin()+start,str.end()};
}

Strings             numberedStrings(String const & prefix,size_t const num)
{
    Strings             ret; ret.reserve(num);
    uint                numDigits = 1;
    size_t              maxVal = 10;
    while (num > maxVal) {
        ++numDigits;
        maxVal *= 10;
    }
    for (size_t ii=0; ii<num; ++ii)
        ret.push_back(prefix+toStrDigits(ii,numDigits));
    return ret;
}

}

// */
