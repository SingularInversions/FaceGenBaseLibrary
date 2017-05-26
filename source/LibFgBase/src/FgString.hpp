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
#include "FgTypes.hpp"
#include "FgSerialize.hpp"

struct  FgString
{
    std::string     m_str;      // UTF-8 unicode

    FgString() {};

    FgString(const char * utf8_c_string)
        : m_str(utf8_c_string) {};

    FgString(const std::string & utf8_string)
        : m_str(utf8_string) {};

    // Construct from a wstring. Since sizeof(wchar_t) is compiler/platform dependent,
    // encoding is utf16 for Windows, utf32 for gcc & XCode:
    FgString(const wchar_t *);
    FgString(const std::wstring &);

    explicit
    FgString(const std::vector<uint32> & utf32_string);

    FgString &
    operator+=(const FgString&);

    FgString
    operator+(const FgString&) const;

    FgString
    operator+(char c) const;

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

    // Encoded as UTF16 if wstring is 16-bit chars, UTF32 if wstring is 32-bit chars:
    std::wstring
    as_wstring() const;

    std::string const &
    as_utf8_string() const
    {return m_str; }

    std::vector<uint32>
    as_utf32() const;

    // Return native unicode string (UTF-16 for Win, UTF-8 for Nix):
#ifdef _MSC_VER
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

    // Replace all occurrences of a in this string with b.  Note
    // that this algorithm is incredibly slow due to utf32<->utf8
    // conversions. Also, the input characters are expected to be
    // ASCII encoded, or at-least their integral value is the
    // expected unicode code point.
    FgString
    replace(char a, char b) const;

    // Split into multiple strings based on split character which is not included in
    // results. At least one string is created and every split character creates an
    // additional string (empty or otherwise):
    std::vector<FgString>
    split(char ch) const;

    uint
    count(char ch) const;

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

std::vector<uint>
fgUtf8ToUtf32(const std::string &);

std::string
fgUtf32ToUtf8(const std::vector<uint> &);

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
