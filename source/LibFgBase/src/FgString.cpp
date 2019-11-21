//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//


#include "stdafx.h"

#include "FgString.hpp"
#include "FgStdString.hpp"
#include "FgDiagnostics.hpp"
#include "FgStdVector.hpp"

using namespace std;

namespace Fg {

u32string
fgToUtf32(string const & str)
{
    // https://stackoverflow.com/questions/38688417/utf-conversion-functions-in-c11
#ifdef _MSC_VER
    if (str.empty())
        return u32string();
    wstring_convert<codecvt_utf8<int32_t>,int32_t>  convert;
    auto            asInt = convert.from_bytes(str);
    return u32string(reinterpret_cast<char32_t const *>(asInt.data()),asInt.size());
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

Ustring::Ustring(const wchar_t * s) : m_str(fgToUtf8(wstring(s)))
{}

Ustring::Ustring(const wstring & s) : m_str(fgToUtf8(s))
{}

wstring
Ustring::as_wstring() const
{
    return fgToUtf16(m_str);
}

#endif

Ustring&
Ustring::operator+=(Ustring const & s)
{
    m_str += s.m_str;
    return *this;
}

Ustring
Ustring::operator+(Ustring const& s) const
{
    return Ustring(m_str+s.m_str);
}

Ustring
Ustring::operator+(char c) const
{
    string      ret = m_str;
    FGASSERT(uchar(c) < 128);
    ret.push_back(c);
    return ret;
}

uint32
Ustring::operator[](size_t idx) const
{
    return uint32(fgToUtf32(m_str).at(idx));
}

size_t
Ustring::size() const
{
    return fgToUtf32(m_str).size();
}

bool
Ustring::is_ascii() const
{
    for (size_t ii=0; ii<m_str.size(); ++ii)
        if ((m_str[ii] & 0x80) != 0)
            return false;
    return true;
}

string const &
Ustring::ascii() const
{
    for (size_t ii=0; ii<m_str.size(); ++ii)
        if ((m_str[ii] & 0x80) != 0)
            fgThrow("Attempt to convert non-ascii utf-8 string to ascii",m_str);
    return m_str;
}

string
Ustring::as_ascii() const
{
    u32string       str(as_utf32());
    string          ret;
    for (size_t ii=0; ii<str.size(); ++ii)
        ret += char(str[ii] & 0x7F);
    return ret;
}

Ustring
Ustring::replace(char a, char b) const
{
    FGASSERT((uchar(a) < 128) && (uchar(b) < 128));
    u32string           str = fgToUtf32(m_str);
    char32_t            a32 = a,
                        b32 = b;
    for (char32_t & c : str)
        if (c == a32)
            c = b32;
    return Ustring(str);
}

Ustrings
Ustring::split(char ch) const
{
    FgStr32s            strs = fgSplit(fgToUtf32(m_str),char32_t(ch));
    Ustrings           ret;
    ret.reserve(strs.size());
    for (const u32string & str : strs)
        ret.push_back(Ustring(str));
    return ret;
}

bool
Ustring::beginsWith(Ustring const & s) const
{
    if(s.m_str.size() > m_str.size())
        return false;
    return (m_str.compare(0,s.m_str.size(),s.m_str) == 0);
}

// Can't put this inline without include file recursive dependency:
bool
Ustring::endsWith(Ustring const & str) const
{return fgEndsWith(as_utf32(),str.as_utf32()); }

Ustring
Ustring::toLower() const
{
    u32string       tmp = as_utf32();
    for (size_t ii=0; ii<tmp.size(); ++ii) {
        char32_t    ch = tmp[ii];
        if ((ch > 64) && (ch < 91))
            tmp[ii] = ch + 32;
    }
    return Ustring(tmp);
}

std::ostream& 
operator<<(std::ostream & os, Ustring const & s)
{
    return os << s.m_str;
}

std::istream&
operator>>(std::istream & is, Ustring & s)
{
    return is >> s.m_str;
}

Ustring
fgTr(string const & msg)
{
    // Just a stub for now:
    return msg;
}

Ustring
fgRemoveChars(Ustring const & str,uchar chr)
{
    FGASSERT(chr < 128);
    u32string       s32 = str.as_utf32(),
                    r32;
    for (size_t ii=0; ii<s32.size(); ++ii)
        if (s32[ii] != chr)
            r32.push_back(s32[ii]);
    return Ustring(r32);
}

Ustring
fgRemoveChars(Ustring const & str,Ustring chrs)
{
    u32string       s32 = str.as_utf32(),
                    c32 = chrs.as_utf32(),
                    r32;
    for (size_t ii=0; ii<s32.size(); ++ii)
        if (!fgContains(c32,s32[ii]))
            r32.push_back(s32[ii]);
    return Ustring(r32);
}

bool
fgGlobMatch(Ustring const & globStr,Ustring const & str)
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

Ustring
fgSubstring(Ustring const & str,size_t start,size_t size)
{
    Ustring        ret;
    u32string       s = str.as_utf32();
    s = fgSubstr(s,start,size);
    ret = Ustring(s);
    return ret;
}

Ustring
fgRest(Ustring const & str,size_t start)
{
    Ustring        ret;
    u32string       s = str.as_utf32();
    s = fgRest(s,start);
    ret = Ustring(s);
    return ret;
}

Ustring
cat(const Ustrings & strings,Ustring const & separator)
{
    Ustring        ret;
    for (size_t ii=0; ii<strings.size(); ++ii) {
        ret += strings[ii];
        if (ii < strings.size()-1)
            ret += separator;
    }
    return ret;
}

string
fgToVariableName(Ustring const & str)
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

Ustring
replaceCharWithString(Ustring const & in,char32_t from,Ustring const to)
{
    u32string       in32 = in.as_utf32(),
                    to32 = to.as_utf32(),
                    out;
    for (char32_t ch : in32) {
        if (ch == from)
            out += to32;
        else
            out += ch;
    }
    return Ustring(out);
}

}
