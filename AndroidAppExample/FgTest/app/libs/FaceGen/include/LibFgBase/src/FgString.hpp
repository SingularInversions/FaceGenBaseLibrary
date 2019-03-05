//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Sohail Somani, Andrew Beatty
//
// A UTF-8 string, typed in order to make explicit this is not ASCII.

#ifndef INCLUDED_FGSTRING_HPP
#define INCLUDED_FGSTRING_HPP

#include "FgStdLibs.hpp"
#include "FgException.hpp"
#include "FgStdString.hpp"
#include "FgTypes.hpp"
#include "FgSerialize.hpp"

std::u32string
fgToUtf32(const std::string & utf8);

std::string
fgToUtf8(const char32_t & utf32_char);

std::string
fgToUtf8(const std::u32string & utf32);

// The following 2 functions are only needed by Windows and don't work on *nix due to
// different sizeof(wchar_t):
#ifdef _WIN32

std::wstring
fgToUtf16(const std::string & utf8);

std::string
fgToUtf8(const std::wstring & utf16);

#endif

struct  FgString
{
    std::string     m_str;      // UTF-8 unicode

    FgString() {};

    FgString(const char * utf8_c_string) : m_str(utf8_c_string) {};

    FgString(const std::string & utf8_string) : m_str(utf8_string) {};

    explicit FgString(char32_t utf32_char) : m_str(fgToUtf8(utf32_char)) {}

    explicit
    FgString(const std::u32string & utf32) : m_str(fgToUtf8(utf32)) {}

    FgString &
    operator+=(const FgString&);

    FgString
    operator+(const FgString&) const;

    FgString
    operator+(char c) const;

    // Use sparingly as this function is very inefficient in UTF-8 encoding:
    uint32
    operator[](size_t) const;

    // Returns number of unicode characters in string
    size_t
    length() const;

    bool
    empty() const
    {return m_str.empty(); }

    void
    clear()
    {m_str.clear(); }

    bool operator==(const FgString & rhs) const
    {return m_str == rhs.m_str; }

    bool operator!=(const FgString & other) const
    {return !(*this == other); }

    bool operator<(const FgString & other) const
    {return m_str < other.m_str; }

    int compare(const FgString & rhs) const
    {return m_str.compare(rhs.m_str); }

    // The narrow-character stream operators do *not* do any
    // character set conversion:
    friend 
    std::ostream& operator<<(std::ostream&, const FgString &);

    friend
    std::istream& operator>>(std::istream&, FgString &);

    std::string const &
    as_utf8_string() const
    {return m_str; }

    std::u32string
    as_utf32() const
    {return fgToUtf32(m_str); }

    // Return native unicode string (UTF-16 for Win, UTF-8 for Nix):
#ifdef _WIN32
    // Construct from a wstring. Since sizeof(wchar_t) is compiler/platform dependent,
    // encoding is utf16 for Windows, utf32 for gcc & XCode:
    FgString(const wchar_t *);
    FgString(const std::wstring &);

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
    FgString
    replace(char a, char b) const;

    // Split into multiple strings based on split character which is not included in
    // results. At least one string is created and every split character creates an
    // additional string (empty or otherwise):
    std::vector<FgString>
    split(char ch) const;

    bool
    beginsWith(const FgString & s) const;

    bool
    endsWith(const FgString & str) const;

    uint
    maxWidth(char ch) const;

    FgString
    toLower() const;        // Member func avoids ambiguity with fgToLower on string literals

    FG_SERIALIZE1(m_str)
};

typedef std::vector<FgString>   FgStrings;

template<>
inline std::string
fgToStr(const FgString & str)
{return str.m_str; }

template<class T>
void fgThrow(const std::string & msg,const T & data) 
{throw FgException(msg,fgToStr(data));  }

template<class T,class U>
void fgThrow(const std::string & msg,const T data0,const U & data1) 
{throw FgException(msg,fgToStr(data0)+","+fgToStr(data1)); }

inline
FgString
fgToLower(const FgString & str)
{return str.toLower(); }

inline
FgString
operator+(const std::string & lhs,const FgString & rhs)
{return FgString(lhs) + rhs; }

// Translate an English message into a UTF-8 string. If the message is
// not found, the untranslated message is returned.
// TODO: This should return const references eventually as they will
// be referenecs to something in a map:
FgString
fgTr(const std::string & message);

// Remove all instances of a given character:
FgString
fgRemoveChars(const FgString & str,uchar chr);

// Remove all instances of any of the given characters:
FgString
fgRemoveChars(const FgString & str,FgString chrs);

// Very simple glob match. Only supports '*' character at beginning or end (but not both)
// or for whole glob string:
bool
fgGlobMatch(const FgString & globStr,const FgString & str);

FgString
fgSubstring(const FgString & str,size_t start,size_t size);

// Inspired by Python join():
FgString
fgCat(const FgStrings & strings,const FgString & separator);

// Changes all non-ASCII-alphanumeric characters to '_' and ensures the first charcter is non-numeric.
// Non-ASCII characters are projected down to ASCII to minimize ambiguities:
std::string
fgToVariableName(const FgString & str);

#endif // INCLUDED_FGSTRING_HPP
