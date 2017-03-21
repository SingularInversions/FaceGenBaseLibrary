//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Sohail Somani
//

#include "stdafx.h"

#include "FgString.hpp"
#include "FgDiagnostics.hpp"
#include "FgStdVector.hpp"

/// difference_type -> int warning. Not a problem unless we have to
/// handle UTF-8 strings with 2 billion characters.
#if defined(_MSC_VER)
#  pragma warning(push)
#  pragma warning(disable:4244)
#endif

#include "utf8.h"

#if defined(_MSC_VER)
#  pragma warning(pop)
#endif

using namespace std;

// Assume this means UTF-16/UCS-2
#if WCHAR_MAX == 65535

static
std::string
convert(std::wstring const & w)
{
    std::string s;
    utf8::utf16to8(w.begin(),w.end(),
                   std::back_inserter(s));
    return s;
}

static
std::wstring
convert(std::string const & s)
{
    std::wstring w;
    utf8::utf8to16(s.begin(),s.end(),
                   std::back_inserter(w));
    return w;
}

// Assume this means UTF-32/UCS-4
#elif WCHAR_MAX == 2147483647

static
std::string
convert(std::wstring const & w)
{
    std::string s;
    utf8::utf32to8(w.begin(),w.end(),
                   std::back_inserter(s));
    return s;
}

static
std::wstring
convert(std::string const & s)
{
    std::wstring w;
    utf8::utf8to32(s.begin(),s.end(),
                   std::back_inserter(w));
    return w;
}

#else
# error Unknown value for WCHAR_MAX
#endif

static
std::string
convert(const vector<uint32> & v)
{
    std::string s;
    utf8::utf32to8(v.begin(),v.end(),std::back_inserter(s));
    return s;
}

FgString::FgString(const wchar_t * s)
    : m_str(convert(std::wstring(s)))
{}

FgString::FgString(const std::wstring & s)
    : m_str(convert(s))
{}

FgString::FgString(const vector<uint32> & utf32_string)
: m_str(convert(utf32_string))
{}

FgString&
FgString::operator+=(const FgString & s)
{
    m_str += s.m_str;
    return *this;
}

FgString
FgString::operator+(const FgString& s) const
{
    return FgString(m_str+s.m_str);
}

FgString
FgString::operator+(char c) const
{
    string      ret = m_str;
    FGASSERT(uchar(c) < 128);
    ret.push_back(c);
    return ret;
}

uint32
FgString::operator[](size_t idx) const
{
    utf8::iterator<std::string::const_iterator> it(m_str.begin(),
                                                   m_str.begin(),m_str.end());
    std::advance(it,idx);
    return *it;
}

size_t
FgString::length() const
{
    return utf8::distance(m_str.begin(),m_str.end());
}

std::wstring
FgString::as_wstring() const
{
    return convert(m_str);
}

std::vector<uint32>
FgString::as_utf32() const
{
    vector<uint32>  ret;
    utf8::utf8to32(m_str.begin(),m_str.end(),std::back_inserter(ret));
    return ret;
}

bool
FgString::is_ascii() const
{
    for (size_t ii=0; ii<m_str.size(); ++ii)
        if ((m_str[ii] & 0x80) != 0)
            return false;
    return true;
}

const std::string &
FgString::ascii() const
{
    for (size_t ii=0; ii<m_str.size(); ++ii)
        if ((m_str[ii] & 0x80) != 0)
            fgThrow("Attempt to convert non-ascii utf-8 string to ascii",m_str);
    return m_str;
}

std::string
FgString::as_ascii() const
{
    vector<uint32>  str(as_utf32());
    string          ret;
    for (size_t ii=0; ii<str.size(); ++ii)
        ret += char(str[ii] & 0x7F);
    return ret;
}

FgString
FgString::replace(char a, char b) const
{
    FGASSERT((uchar(a) < 128) && (uchar(b) < 128));
    FgString                ret(m_str);
    string::const_iterator  src = m_str.begin();
    string::iterator        dst = ret.m_str.begin();
    while (src != m_str.end())
    {
        if (*src == a)
            *dst = b;
        utf8::next(src,m_str.end());
        utf8::next(dst,ret.m_str.end());
    }
    return ret;
}

FgStrings
FgString::split(char ch) const
{
    FgStrings           ret;
    vector<uint32>      str = as_utf32();
    vector<uint32>      ss;
    for (size_t ii=0; ii<str.size(); ++ii) {
        if (str[ii] == uint32(ch)) {
            ret.push_back(FgString(ss));
            ss.clear();
        }
        else
            ss.push_back(str[ii]);
    }
    ret.push_back(FgString(ss));
    return ret;
}

uint
FgString::count(char ch) const
{
    uint    ret = 0;
    utf8::iterator<std::string::const_iterator>
        it(m_str.begin(),m_str.begin(),m_str.end()),
        end(m_str.end(),m_str.begin(),m_str.end());
    while (it != end)
    {
        if (*it == uint32(ch))
            ++ret;
        it++;
    }
    return ret;
}

bool
FgString::beginsWith(const FgString & s) const
{
    if(s.m_str.size() > m_str.size())
        return false;
    return (m_str.compare(0,s.m_str.size(),s.m_str) == 0);
}

// Can't put this inline without include file recursive dependency:
bool
FgString::endsWith(const FgString & str) const
{return fgEndsWith(as_utf32(),str.as_utf32()); }

FgString
FgString::toLower() const
{
    vector<uint32>  tmp = as_utf32();
    for (size_t ii=0; ii<tmp.size(); ++ii) {
        uint32      ch = tmp[ii];
        if ((ch > 64) && (ch < 91))
            tmp[ii] = ch + 32;
    }
    return FgString(tmp);
}

std::ostream& 
operator<<(std::ostream & os, const FgString & s)
{
    return os << s.m_str;
}

std::istream&
operator>>(std::istream & is, FgString & s)
{
    return is >> s.m_str;
}

FgString
fgTr(const std::string & msg)
{
    // Just a stub for now:
    return msg;
}

FgString
fgRemoveChars(const FgString & str,uchar chr)
{
    FGASSERT(chr < 128);
    vector<uint32>  s32 = str.as_utf32(),
                    r32;
    for (size_t ii=0; ii<s32.size(); ++ii)
        if (s32[ii] != chr)
            r32.push_back(s32[ii]);
    return FgString(r32);
}

FgString
fgRemoveChars(const FgString & str,FgString chrs)
{
    vector<uint32>  s32 = str.as_utf32(),
                    c32 = chrs.as_utf32(),
                    r32;
    for (size_t ii=0; ii<s32.size(); ++ii)
        if (!fgContains(c32,s32[ii]))
            r32.push_back(s32[ii]);
    return FgString(r32);
}

bool
fgGlobMatch(const FgString & globStr,const FgString & str)
{
    if (globStr.empty())
        return str.empty();
    if (globStr == "*")
        return true;
    vector<uint>        gs = globStr.as_utf32(),
                        ts = str.as_utf32();
    if (gs[0] == '*')
        return fgEndsWith(ts,fgRest(gs,1));
    else if (gs.back() == '*')
        return fgBeginsWith(ts,fgHead(gs,gs.size()-1));
    return (str == globStr);
}

FgString
fgSubstring(const FgString & str,size_t start,size_t size)
{
    FgString        ret;
    vector<uint>    s = str.as_utf32();
    s = fgSubvec(s,start,size);
    ret = FgString(s);
    return ret;
}

FgString
fgCat(const FgStrings & strings,const FgString & separator)
{
    FgString        ret;
    for (size_t ii=0; ii<strings.size(); ++ii) {
        ret += strings[ii];
        if (ii < strings.size()-1)
            ret += separator;
    }
    return ret;
}

string
fgToVariableName(const FgString & str)
{
    string          ret;
    vector<uint>    str32 = str.as_utf32();
    // First character must be alphabetical or underscore:
    if (isalpha(char(str32[0])))
        ret += char(str32[0]);
    else
        ret += '_';
    for (size_t ii=1; ii<str32.size(); ++ii) {
        if (isalnum(char(str32[ii])))
            ret += char(str32[ii]);
        else
            ret += '_';
    }
    return ret;
}
