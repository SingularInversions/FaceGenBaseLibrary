//
// Coypright (c) 2022 Singular Inversions Inc. (facegen.com)
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

struct  String8
{
    String          m_str;      // UTF-8 unicode

    String8() {};
    String8(char const * utf8_c_string) : m_str(utf8_c_string) {};
    String8(String const & utf8_string) : m_str(utf8_string) {};
    explicit String8(char32_t utf32_char) : m_str(toUtf8(utf32_char)) {}
    explicit String8(String32 const & utf32) : m_str(toUtf8(utf32)) {}

    String8 &       operator+=(String8 const & rhs);
    String8 &       operator+=(char rhs);               // checks that 'rhs' is ASCII
    String8         operator+(String8 const&) const;
    String8         operator+(char const * utf8_c_str) {return String8(m_str + utf8_c_str); }
    String8         operator+(char c) const;

    // Use sparingly as this function is very inefficient for sequential use (due to UTF-8 encoding):
    uint32          operator[](size_t) const;
    // Returns number of unicode characters in string
    size_t          size() const;
    bool            empty() const {return m_str.empty(); }
    void            clear() {m_str.clear(); }
    bool            operator==(String8 const & rhs) const {return m_str == rhs.m_str; }
    bool            operator!=(String8 const & other) const {return !(*this == other); }
    bool            operator<(String8 const & other) const {return m_str < other.m_str; }
    int             compare(String8 const & rhs) const {return m_str.compare(rhs.m_str); }

    // The narrow-character stream operators do not do any character set conversion:
    friend          std::ostream& operator<<(std::ostream&, String8 const &);
    friend          std::istream& operator>>(std::istream&, String8 &);

    String const &  as_utf8_string() const {return m_str; }
    String32        as_utf32() const {return toUtf32(m_str); }

    // Return native unicode string (UTF-16 for Win, UTF-8 for Nix):
#ifdef _WIN32
    // Construct from a wstring. Since sizeof(wchar_t) is compiler/platform dependent,
    // encoding is utf16 for Windows, utf32 for gcc & XCode:
    String8(wchar_t const *);
    String8(std::wstring const &);

    // Encoded as UTF16 if wchar_t is 16-bit, UTF32 if wchar_t is 32-bit:
    std::wstring    as_wstring() const;
    std::wstring    ns() const {return as_wstring(); }
#else
    String          ns() const {return as_utf8_string(); }
#endif

    bool            is_ascii() const;
    // Throw if there are any non-ascii characters:
    const String &  ascii() const;
    // Mutilate any non-ASCII characters into ASCII:
    String          as_ascii() const;
    // Replace all occurrences of 'a' with 'b'. Slow due to utf32<->utf8 conversions.
    // 'a' and 'b' are considered as unsigned values when comparing with UTF code points:
    String8         replace(char a, char b) const;
    // Split into multiple strings based on split character which is not included in
    // results. At least one string is created and every split character creates an
    // additional string (empty or otherwise):
    Svec<String8>   split(char ch) const;
    bool            beginsWith(String8 const & s) const;
    bool            endsWith(String8 const & str) const;
    String8         toLower() const;        // Member func avoids ambiguity with toLower on string literals

    FG_SERIALIZE1(m_str)
};
typedef Svec<String8>   String8s;
typedef Svec<String8s>  String8ss;

String8s            toUstrings(Strings const & strs);

template<>
inline String       toStr(String8 const & str) {return str.m_str; }
template<class T>
void                fgThrow(String const & msg,T const & data)  {throw FgException(msg,toStr(data));  }
template<class T,class U>
void                fgThrow(String const & msg,T const data0,const U & data1) 
{
    throw FgException(msg,toStr(data0)+","+toStr(data1));
}

inline String8      toLower(String8 const & str) {return str.toLower(); }
inline String8      operator+(const String & lhs,String8 const & rhs) {return String8(lhs) + rhs; }

// Translate an English message into a UTF-8 string. If the message is
// not found, the untranslated message is returned.
// TODO: This should return const references eventually as they will
// be referenecs to something in a map:
String8             fgTr(const String & message);
// Remove all instances of a given character:
String8             removeChars(String8 const & str,uchar chr);
// Remove all instances of any of the given characters:
String8             removeChars(String8 const & str,String8 chrs);
// Very simple glob match. Only supports '*' character at beginning or end (but not both)
// or for whole glob string:
bool                isGlobMatch(String8 const & globStr,String8 const & str);
String8             cSubstr8(String8 const & str,size_t start,size_t size);
String8             cRest(String8 const & s,size_t start);
// Inspired by Python join():
String8             cat(String8s const & strings,String8 const & separator);
String8             catDeref(Ptrs<String8> const & stringPtrs,String8 const & separator);
// Changes all non-ASCII-alphanumeric characters to '_' and ensures the first charcter is non-numeric.
// Non-ASCII characters are projected down to ASCII to minimize ambiguities:
String              fgToVariableName(String8 const & str);
// Replace all instances of 'from' with 'to' in 'in':
String8             replaceCharWithString(String8 const & in,char32_t from,String8 const to);
// Print a title and indented list of items one per line:
void                printList(String const & title,Strings const & items,bool numbered);

}

#endif // INCLUDED_FGSTRING_HPP
