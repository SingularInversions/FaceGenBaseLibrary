//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//


#include "stdafx.h"

#include "FgString.hpp"
#include "FgMath.hpp"

#ifdef _MSC_VER
#pragma warning(disable:4996)   // C11 UTF 8-16-32 conversion functions are deprecated but there is no replacement, so keep using them !
#endif

using namespace std;

namespace Fg {

Str32            toUtf32(String const & str)
{
    // https://stackoverflow.com/questions/38688417/utf-conversion-functions-in-c11
#ifdef _MSC_VER
    if (str.empty())
        return Str32();
    wstring_convert<codecvt_utf8<int32_t>,int32_t>  convert;
    auto            asInt = convert.from_bytes(str);
    return Str32(reinterpret_cast<char32_t const *>(asInt.data()),asInt.size());
#else
    wstring_convert<codecvt_utf8<char32_t>,char32_t> convert;
    return convert.from_bytes(str);
#endif
}

String              toUtf8(Str32 const & in)
{
    // https://stackoverflow.com/questions/38688417/utf-conversion-functions-in-c11
#ifdef _MSC_VER
    if (in.empty())
        return {};
    wstring_convert<codecvt_utf8<int32_t>,int32_t>    convert;
    auto            ptr = reinterpret_cast<const int32_t *>(in.data());
    return convert.to_bytes(ptr,ptr+in.size());
#else
    wstring_convert<codecvt_utf8<char32_t>,char32_t>    convert;
    return convert.to_bytes(in);
#endif
}

String              toUtf8(char32_t utf32_char) {return toUtf8(Str32(1,utf32_char)); }

wstring             toUtf16(String const & utf8)
{
    wstring_convert<codecvt_utf8_utf16<wchar_t> >   converter;
    return converter.from_bytes(utf8);
}

String              toUtf8(wstring const & utf16)
{
    // codecvt_utf8_utf16 inherits from codecvt
    wstring_convert<codecvt_utf8_utf16<wchar_t> >   converter;
    return converter.to_bytes(utf16);
}

String              toStrFixed(double val,uint fractionalDigits)
{
    ostringstream   os;
    os << std::fixed << std::setprecision(fractionalDigits) << val;
    return os.str();
}

String              toStrPercent(double val,uint fractionalDigits)
{
    return toStrFixed(val*100.0,fractionalDigits) + "%";
}

String              toStrRelPercents(double v0,double v1,uint fractionalDigits)
{
    double              tot = v0 + v1;
    return toStrFixed(100*v0/tot,fractionalDigits) + "-" + toStrFixed(100*v1/tot,fractionalDigits) + "%";
}

String              toLower(const String & s)
{
    String  retval;
    retval.reserve(s.size());
	// Can't use std::transform since 'tolower' causes warning by converting int->char
    // std::transform(s.begin(),s.end(),retval.begin(),::tolower);
	for (char ch : s)
		retval.push_back(char(std::tolower(ch)));
    return retval;
}

String              toUpper(const String & s)
{
    String  retval;
    retval.reserve(s.size());
	// Can't use std::transform since 'tolower' causes warning by converting int->char
	// std::transform(s.begin(),s.end(),retval.begin(),::toupper);
	for (char ch : s)
		retval.push_back(char(std::toupper(ch)));
	return retval;
}

Strings             splitAtSeparators(String const & str,char sep)
{
    Strings             ret;
    String              tmp;
    auto                it = str.begin();
    while (it != str.end()) {
        if (*it == sep) {
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

String              replaceAll(String const & str,char orig,char repl)
{
    String              ret; ret.reserve(str.size());
    for (char ch : str) {
        if (ch == orig)
            ret.push_back(repl);
        else
            ret.push_back(ch);
    }
    return ret;
}

String              replaceFirst(String const & str,String const & sub,String const & rep)
{
    size_t              idx = str.find(sub);
    if (idx == String::npos)
        return str;
    else
        return cHead(str,idx) + rep + cRest(str,idx+sub.size());
}

bool                replaceAll_(String const & str,String const & fnd,String const & rpl,String & ret)
{
    FGASSERT(!fnd.empty());
    size_t              beg {0},
                        end;
    while ((end=str.find(fnd,beg)) != String::npos) {
        ret += str.substr(beg,end-beg) + rpl;
        beg = end + fnd.size();     // don't want to find the same string again !
    }
    if (beg == 0)
        return false;
    ret += cRest(str,beg);
    return true;
}

String              padToLen(String const & str,size_t len,char ch)
{
    String  ret = str;
    if (len > str.size())
        ret.resize(len,ch);
    return ret;
}

String              cat(Strings const & strings,String const & separator)
{
    String      ret;
    for (size_t ii=0; ii<strings.size(); ++ii) {
        ret += strings[ii];
        if (ii < strings.size()-1)
            ret += separator;
    }
    return ret;
}

String              cat(set<String> const & strings,String const & separator)
{
    auto                it = strings.begin();
    if (it == strings.end())
        return String{};
    String              ret {*it};
    while (++it != strings.end())
        ret += separator + *it;
    return ret;
}

String              catDeref(Ptrs<String> const & stringPtrs,String const & separator)
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
Opt<int>            fromStr<int>(String const & str)
{
    Opt<int>              ret;
    if (str.empty())
        return ret;
    bool                    neg = false;
    String::const_iterator  it = str.begin();
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
Opt<uint>           fromStr<uint>(String const & str)
{
    Opt<uint>             ret;
    if (str.empty())
        return ret;
    String::const_iterator  it = str.begin();
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

String              cRest(String const & str,size_t start)
{
    FGASSERT1(start <= str.size(),str+"@"+toStr(start));      // Can be size zero
    return String{str.begin()+start,str.end()};
}

Str32            cRest(Str32 const & str,size_t start)
{
    FGASSERT1(start <= str.size(),toUtf8(str)+"@"+toStr(start));
    return Str32{str.begin()+start,str.end()};
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

// Following 2 functions are only needed by Windows and don't work on *nix due to
// different sizeof(wchar_t):
#ifdef _WIN32

String8::String8(const wchar_t * s) : m_str(toUtf8(wstring(s))) {}
String8::String8(const wstring & s) : m_str(toUtf8(s)) {}

wstring             String8::as_wstring() const {return toUtf16(m_str); }

#endif

String8 &           String8::operator+=(String8 const & rhs)
{
    m_str += rhs.m_str;
    return *this;
}

String8 &           String8::operator+=(char rhs)
{
    m_str += rhs;
    return *this;
}

String8             String8::operator+(String8 const& s) const {return String8(m_str+s.m_str); }

String8             String8::operator+(char c) const
{
    String      ret = m_str;
    FGASSERT(uchar(c) < 128);
    ret.push_back(c);
    return ret;
}

uint32              String8::operator[](size_t idx) const {return uint32(toUtf32(m_str).at(idx)); }

size_t              String8::size() const {return toUtf32(m_str).size(); }

bool                String8::is_ascii() const
{
    for (size_t ii=0; ii<m_str.size(); ++ii)
        if ((m_str[ii] & 0x80) != 0)
            return false;
    return true;
}

String const &      String8::ascii() const
{
    for (size_t ii=0; ii<m_str.size(); ++ii)
        if ((m_str[ii] & 0x80) != 0)
            fgThrow("Attempt to convert non-ascii utf-8 String to ascii",m_str);
    return m_str;
}

String              String8::as_ascii() const
{
    Str32       str(as_utf32());
    String          ret;
    for (size_t ii=0; ii<str.size(); ++ii)
        ret += char(str[ii] & 0x7F);
    return ret;
}

String8             String8::replace(char a, char b) const
{
    FGASSERT((uchar(a) < 128) && (uchar(b) < 128));
    Str32           str = toUtf32(m_str);
    char32_t            a32 = a,
                        b32 = b;
    for (char32_t & c : str)
        if (c == a32)
            c = b32;
    return String8(str);
}

String8s            String8::split(char ch) const
{
    Str32s            strs = splitAtChar(toUtf32(m_str),char32_t(ch));
    String8s           ret;
    ret.reserve(strs.size());
    for (Str32 const & str : strs)
        ret.push_back(String8(str));
    return ret;
}

bool                String8::beginsWith(String8 const & s) const
{
    if(s.m_str.size() > m_str.size())
        return false;
    return (m_str.compare(0,s.m_str.size(),s.m_str) == 0);
}

// Can't put this inline without include file recursive dependency:
bool                String8::endsWith(String8 const & str) const {return Fg::endsWith(as_utf32(),str.as_utf32()); }

String8             String8::toLower() const
{
    Str32       tmp = as_utf32();
    for (size_t ii=0; ii<tmp.size(); ++ii) {
        char32_t    ch = tmp[ii];
        if ((ch > 64) && (ch < 91))
            tmp[ii] = ch + 32;
    }
    return String8(tmp);
}

std::ostream &      operator<<(std::ostream & os, String8 const & s) {return os << s.m_str; }

std::istream &      operator>>(std::istream & is, String8 & s) {return is >> s.m_str; }

String8s            toUstrings(Strings const & strs)
{
    String8s        ret;
    ret.reserve(strs.size());
    for (String const & s : strs)
        ret.push_back(String8(s));
    return ret;
}

String8             fgTr(String const & msg) {return msg; } // Just a stub for now

String8             removeChars(String8 const & str,uchar chr)
{
    FGASSERT(chr < 128);
    Str32       s32 = str.as_utf32(),
                    r32;
    for (size_t ii=0; ii<s32.size(); ++ii)
        if (s32[ii] != chr)
            r32.push_back(s32[ii]);
    return String8(r32);
}

String8             removeChars(String8 const & str,String8 chrs)
{
    Str32       s32 = str.as_utf32(),
                    c32 = chrs.as_utf32(),
                    r32;
    for (size_t ii=0; ii<s32.size(); ++ii)
        if (!contains(c32,s32[ii]))
            r32.push_back(s32[ii]);
    return String8(r32);
}

bool                isGlobMatch(String8 const & globStr,String8 const & str)
{
    if (globStr.empty())
        return str.empty();
    if (globStr == "*")
        return true;
    Str32           gs = globStr.as_utf32(),
                        ts = str.as_utf32();
    if (gs[0] == '*')
        return endsWith(ts,cRest(gs,1));
    else if (gs.back() == '*')
        return beginsWith(ts,cHead(gs,gs.size()-1));
    return (str == globStr);
}

String8             cSubstr8(String8 const & str,size_t start,size_t size)
{
    String8        ret;
    Str32       s = str.as_utf32();
    s = cSubstr(s,start,size);
    ret = String8(s);
    return ret;
}

String8             cRest(String8 const & str,size_t start)
{
    String8        ret;
    Str32       s = str.as_utf32();
    s = cRest(s,start);
    ret = String8(s);
    return ret;
}

String8             cat(String8s const & strings,String8 const & separator)
{
    String8        ret;
    for (size_t ii=0; ii<strings.size(); ++ii) {
        ret += strings[ii];
        if (ii < strings.size()-1)
            ret += separator;
    }
    return ret;
}
String8             catDeref(Ptrs<String8> const & stringPtrs,String8 const & separator)
{
    String8             ret;
    size_t              sz = stringPtrs.size();
    for (size_t ii=0; ii<sz; ++ii) {
        ret.m_str.append(stringPtrs[ii]->m_str);
        if (ii+1 < sz)
            ret.m_str.append(separator.m_str);
    }
    return ret;
}

String              fgToVariableName(String8 const & str)
{
    String          ret;
    Str32    str32 = str.as_utf32();
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

String8             replaceCharWithString(String8 const & in,char32_t from,String8 const to)
{
    Str32       in32 = in.as_utf32(),
                    to32 = to.as_utf32(),
                    out;
    for (char32_t ch : in32) {
        if (ch == from)
            out += to32;
        else
            out += ch;
    }
    return String8(out);
}

void                printList(String const & title,Strings const & items,bool numbered)
{
    PushIndent      pind {title};
    for (size_t ii=0; ii<items.size(); ++ii) {
        fgout << fgnl;
        if (numbered)
            fgout << toStrDigits(ii,2);
        fgout << " " << items[ii];
    }
}

// Generate single hex char. Value must be < 16:
static char         toHexChar(uchar c4)
{
    FGASSERT(c4 < 16);
    static char const * digits = "0123456789ABCDEF";
    return digits[c4];
}
String              toHexString(uchar c)
{
    return
        String(1,toHexChar(c >> 4)) +
        String(1,toHexChar(c & 0xF));
}
String              toHexString(uint16 val)
{
    return
        toHexString(uchar(val >> 8)) +
        toHexString(uchar(val & 0xFF));
}
String              toHexString(uint32 val)
{
    return
        toHexString(uint16(val >> 16)) +
        toHexString(uint16(val & 0xFFFF));
}
String              toHexString(uint64 val)
{
    return
        toHexString(uint32(val >> 32)) +
        toHexString(uint32(val & 0xFFFFFFFF));
}
String              bytesToHexString(const uchar *arr,uint numBytes)
{
    String  ret;
    for (uint ii=0; ii<numBytes; ++ii)
        ret += toHexString(*arr++);
    return ret;
}
String              toHex64Crc(uint64 id)
{
    uint16          parts[4];
    parts[0] = uint16((id >> 48) & 0xFFFF);
    parts[1] = uint16((id >> 32) & 0xFFFF);
    parts[2] = uint16((id >> 16) & 0xFFFF);
    parts[3] = uint16(id & 0xFFFF);
    String          ret;
    uint16          crc = 0;
    for (uint ii=0; ii<4; ++ii) {
        ret += toHexString(parts[ii]) + "-";
        crc = crc ^ parts[ii];
    }
    ret += toHexString(crc);
    return  ret;
}
Valid<uint>         fromHex4(char ch)
{
    Valid<uint>       ret;
    if ((ch >= '0') && (ch <= '9'))
        ret = uint(ch) - uint('0');
    else if ((ch >= 'a') && (ch <= 'f'))
        ret = uint(ch) - uint('a') + 10;
    else if ((ch >= 'A') && (ch <= 'F'))
        ret = uint(ch) - uint('A') + 10;
    else if ((ch == 'O') || (ch == 'o'))    // User confused 0 with letter O:
        ret = 0U;
    else if ((ch == 'I') || (ch == 'l'))    // User confused 1 with letter I or l
        ret = 1U;
    return ret;
}
static Valid<uint>  get16(istringstream & iss)
{
    uint            val = 0;
    uint            cnt = 0;
    for (;;) {
        int                 chi = iss.get();
        FGASSERT(chi < 256);
        if (iss.eof())
            return Valid<uint>();
        Valid<uint>       chVal = fromHex4(scast<char>(chi));
        if (chVal.valid()) {
            val = (val << 4) + chVal.val();
            if (++cnt == 4)
                return Valid<uint>(val);
        }
    }
}
uint64              fromHex64(String const & hex)
{
    uint64          ret = 0;
    istringstream   iss(hex);
    for (uint ii=0; ii<4; ++ii) {
        Valid<uint>       val = get16(iss);
        ret = (ret << 16) + val.val();
    }
    return ret;
}
uint64              fromHex64Crc(String const & uk)
{
    uint64          ret = 0;
    uint16          crc = 0;
    istringstream   iss(uk);
    for (uint ii=0; ii<4; ++ii) {
        Valid<uint>       val = get16(iss);
        if (!val.valid())
            return 0;
        ret = (ret << 16) + val.val();
        crc = crc ^ static_cast<uint16>(val.val());
    }
    Valid<uint>   chk = get16(iss);
    if (chk.valid() && (chk.val() == crc))
        return ret;
    return 0;
}

Strings             cNumberedLabels(String const & baseLabel,size_t num)
{
    size_t              numDigits = cNumDigits(num);
    return genSvec<String>(num,[&,numDigits](size_t ii){return baseLabel + toStrDigits(ii,numDigits); });
}
String8s            cNumberedLabels(String8 const & baseLabel,size_t num)
{
    size_t              numDigits = cNumDigits(num);
    return genSvec<String8>(num,[&,numDigits](size_t ii){return baseLabel + toStrDigits(ii,numDigits); });
}

}
