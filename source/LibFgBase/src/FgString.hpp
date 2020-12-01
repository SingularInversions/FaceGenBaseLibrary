//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

// A UTF-8 string, typed in order to make explicit this is not ASCII.

#ifndef INCLUDED_FGSTRING_HPP
#define INCLUDED_FGSTRING_HPP

#include "FgStdLibs.hpp"
#include "FgException.hpp"
#include "FgStdString.hpp"
#include "FgTypes.hpp"
#include "FgSerialize.hpp"

namespace Fg {

std::u32string
toUtf32(const std::string & utf8);

std::string
toUtf8(const char32_t & utf32_char);

std::string
toUtf8(const std::u32string & utf32);

// The following 2 functions are only needed by Windows and don't work on *nix due to
// different sizeof(wchar_t):
#ifdef _WIN32

std::wstring
toUtf16(const std::string & utf8);

std::string
toUtf8(const std::wstring & utf16);

#endif

struct  Ustring
{
    std::string     m_str;      // UTF-8 unicode

    Ustring() {};

    Ustring(const char * utf8_c_string) : m_str(utf8_c_string) {};

    Ustring(const std::string & utf8_string) : m_str(utf8_string) {};

    explicit Ustring(char32_t utf32_char) : m_str(toUtf8(utf32_char)) {}

    explicit
    Ustring(const std::u32string & utf32) : m_str(toUtf8(utf32)) {}

    Ustring &
    operator+=(Ustring const&);

    Ustring
    operator+(Ustring const&) const;

    Ustring
    operator+(char const * utf8_c_str)
    {return Ustring(m_str + utf8_c_str); }

    Ustring
    operator+(char c) const;

    // Use sparingly as this function is very inefficient in UTF-8 encoding:
    uint32
    operator[](size_t) const;

    // Returns number of unicode characters in string
    size_t
    size() const;

    bool
    empty() const
    {return m_str.empty(); }

    void
    clear()
    {m_str.clear(); }

    bool operator==(Ustring const & rhs) const
    {return m_str == rhs.m_str; }

    bool operator!=(Ustring const & other) const
    {return !(*this == other); }

    bool operator<(Ustring const & other) const
    {return m_str < other.m_str; }

    int compare(Ustring const & rhs) const
    {return m_str.compare(rhs.m_str); }

    // The narrow-character stream operators do *not* do any
    // character set conversion:
    friend 
    std::ostream& operator<<(std::ostream&, Ustring const &);

    friend
    std::istream& operator>>(std::istream&, Ustring &);

    std::string const &
    as_utf8_string() const
    {return m_str; }

    std::u32string
    as_utf32() const
    {return toUtf32(m_str); }

    // Return native unicode string (UTF-16 for Win, UTF-8 for Nix):
#ifdef _WIN32
    // Construct from a wstring. Since sizeof(wchar_t) is compiler/platform dependent,
    // encoding is utf16 for Windows, utf32 for gcc & XCode:
    Ustring(const wchar_t *);
    Ustring(const std::wstring &);

    // Encoded as UTF16 if wchar_t is 16-bit, UTF32 if wchar_t is 32-bit:
    std::wstring
    as_wstring() const;

    std::wstring
    ns() const
    {return as_wstring(); }
#else
    std::string
    ns() const
    {return as_utf8_string(); }
#endif

    bool
    is_ascii() const;

    // Throw if there are any non-ascii characters:
    const std::string &
    ascii() const;

    // Mutilate any non-ASCII characters into ASCII:
    std::string
    as_ascii() const;

    // Replace all occurrences of 'a' with 'b'. Slow due to utf32<->utf8 conversions.
    // 'a' and 'b' are considered as unsigned values when comparing with UTF code points:
    Ustring
    replace(char a, char b) const;

    // Split into multiple strings based on split character which is not included in
    // results. At least one string is created and every split character creates an
    // additional string (empty or otherwise):
    Svec<Ustring>
    split(char ch) const;

    bool
    beginsWith(Ustring const & s) const;

    bool
    endsWith(Ustring const & str) const;

    uint
    maxWidth(char ch) const;

    Ustring
    toLower() const;        // Member func avoids ambiguity with toLower on string literals

    FG_SERIALIZE1(m_str)
};

typedef Svec<Ustring>   Ustrings;

Ustrings
toUstrings(Strings const & strs);

template<>
inline std::string
toStr(Ustring const & str)
{return str.m_str; }

template<class T>
void fgThrow(const std::string & msg,T const & data) 
{throw FgException(msg,toStr(data));  }

template<class T,class U>
void fgThrow(const std::string & msg,T const data0,const U & data1) 
{throw FgException(msg,toStr(data0)+","+toStr(data1)); }

inline
Ustring
toLower(Ustring const & str)
{return str.toLower(); }

inline
Ustring
operator+(const std::string & lhs,Ustring const & rhs)
{return Ustring(lhs) + rhs; }

// Translate an English message into a UTF-8 string. If the message is
// not found, the untranslated message is returned.
// TODO: This should return const references eventually as they will
// be referenecs to something in a map:
Ustring
fgTr(const std::string & message);

// Remove all instances of a given character:
Ustring
removeChars(Ustring const & str,uchar chr);

// Remove all instances of any of the given characters:
Ustring
removeChars(Ustring const & str,Ustring chrs);

// Very simple glob match. Only supports '*' character at beginning or end (but not both)
// or for whole glob string:
bool
isGlobMatch(Ustring const & globStr,Ustring const & str);

Ustring
cSubstr(Ustring const & str,size_t start,size_t size);

Ustring
cRest(Ustring const & s,size_t start);

// Inspired by Python join():
Ustring
cat(Ustrings const & strings,Ustring const & separator);

// Changes all non-ASCII-alphanumeric characters to '_' and ensures the first charcter is non-numeric.
// Non-ASCII characters are projected down to ASCII to minimize ambiguities:
std::string
fgToVariableName(Ustring const & str);

// Replace all instances of 'from' with 'to' in 'in':
Ustring
replaceCharWithString(Ustring const & in,char32_t from,Ustring const to);

}

#endif // INCLUDED_FGSTRING_HPP
