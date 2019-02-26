//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Sohail Somani
//

#include "stdafx.h"

#include "FgString.hpp"
#include "FgStdString.hpp"
#include "FgDiagnostics.hpp"
#include "FgStdVector.hpp"

using namespace std;

u32string
fgToUtf32(const string & str)
{
    // https://stackoverflow.com/questions/38688417/utf-conversion-functions-in-c11
#ifdef _MSC_VER
    if (str.empty())
        return u32string();
    wstring_convert<codecvt_utf8<int32_t>,int32_t>  convert;
    auto            asInt = convert.from_bytes(str);
    return u32string(reinterpret_cast<char32_t const *>(asInt.data()),asInt.length());
#else
    wstring_convert<codecvt_utf8<char32_t>,char32_t> convert;
    return convert.from_bytes(str);
#endif
}

string
fgToUtf8(const u32string & in)
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
fgToUtf8(const char32_t & utf32_char)
{return fgToUtf8(u32string(1,utf32_char)); }

// Following 2 functions are only needed by Windows and don't work on *nix due to
// different sizeof(wchar_t):
#ifdef _WIN32

std::wstring
fgToUtf16(const std::string & utf8)
{
    wstring_convert<codecvt_utf8_utf16<wchar_t> >   converter;
    return converter.from_bytes(utf8);
}

std::string
fgToUtf8(const std::wstring & utf16)
{
    // codecvt_utf8_utf16 inherits from codecvt
    wstring_convert<codecvt_utf8_utf16<wchar_t> >   converter;
    return converter.to_bytes(utf16);
}

FgString::FgString(const wchar_t * s) : m_str(fgToUtf8(wstring(s)))
{}

FgString::FgString(const wstring & s) : m_str(fgToUtf8(s))
{}

wstring
FgString::as_wstring() const
{
    return fgToUtf16(m_str);
}

#endif

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
    return uint32(fgToUtf32(m_str).at(idx));
}

size_t
FgString::length() const
{
    return fgToUtf32(m_str).size();
}

bool
FgString::is_ascii() const
{
    for (size_t ii=0; ii<m_str.size(); ++ii)
        if ((m_str[ii] & 0x80) != 0)
            return false;
    return true;
}

const string &
FgString::ascii() const
{
    for (size_t ii=0; ii<m_str.size(); ++ii)
        if ((m_str[ii] & 0x80) != 0)
            fgThrow("Attempt to convert non-ascii utf-8 string to ascii",m_str);
    return m_str;
}

string
FgString::as_ascii() const
{
    u32string       str(as_utf32());
    string          ret;
    for (size_t ii=0; ii<str.size(); ++ii)
        ret += char(str[ii] & 0x7F);
    return ret;
}

FgString
FgString::replace(char a, char b) const
{
    FGASSERT((uchar(a) < 128) && (uchar(b) < 128));
    u32string           str = fgToUtf32(m_str);
    char32_t            a32 = a,
                        b32 = b;
    for (char32_t & c : str)
        if (c == a32)
            c = b32;
    return FgString(str);
}

FgStrings
FgString::split(char ch) const
{
    FgStr32s            strs = fgSplit(fgToUtf32(m_str),char32_t(ch));
    FgStrings           ret;
    ret.reserve(strs.size());
    for (const u32string & str : strs)
        ret.push_back(FgString(str));
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
    u32string       tmp = as_utf32();
    for (size_t ii=0; ii<tmp.size(); ++ii) {
        char32_t    ch = tmp[ii];
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
fgTr(const string & msg)
{
    // Just a stub for now:
    return msg;
}

FgString
fgRemoveChars(const FgString & str,uchar chr)
{
    FGASSERT(chr < 128);
    u32string       s32 = str.as_utf32(),
                    r32;
    for (size_t ii=0; ii<s32.size(); ++ii)
        if (s32[ii] != chr)
            r32.push_back(s32[ii]);
    return FgString(r32);
}

FgString
fgRemoveChars(const FgString & str,FgString chrs)
{
    u32string       s32 = str.as_utf32(),
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
    u32string           gs = globStr.as_utf32(),
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
    u32string       s = str.as_utf32();
    s = fgSubstr(s,start,size);
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
    u32string    str32 = str.as_utf32();
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
