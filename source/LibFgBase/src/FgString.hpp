//
// Copyright (c) 2025 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// A UTF-8 string, typed in order to make explicit this is not ASCII.

#ifndef INCLUDED_FGSTRING_HPP
#define INCLUDED_FGSTRING_HPP

#include "FgStdExtensions.hpp"

namespace Fg {

Str32               toUtf32(String const & utf8);
String              toUtf8(char32_t utf32_char);
String              toUtf8(Str32 const & utf32);
// The following 2 functions are only needed by Windows and don't work on *nix due to
// different sizeof(wchar_t):
#ifdef _WIN32
std::wstring        toUtf16(String const & utf8);
String              toUtf8(std::wstring const & utf16);
#endif

// Default uses standard stream input "lexical conversions".
// Only valid strings for the given type are accepted, extra characters including whitespace are errors
// Define fully specialized versions where this behaviour is not desired:
// old versions of GCC give maybe-uninitialized warning for the 'return {}' below and there is no way to avoid it:
//#if defined(__GNUC__) && !defined(__clang__)
//#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
//#endif
template<class T>
Opt<T>              fromStr(String const & str)
{
    T                   val;
    std::istringstream  iss {str};
    iss >> val;
    if (iss.fail())
        return {};
    else
        return val;
}
// Only valid integer representations within the range of int32 will return a valid value.
// Whitespace returns invalid:
template<> Opt<int> fromStr<int>(String const &);
template<> Opt<uint> fromStr<uint>(String const &);

// Use this version to throw an informative error if the String cannot be converted to the type.
// require explicit type template:
template<class T>
T                   strTo(String const & str)
{
    Opt<T>              oval = fromStr<T>(str);
    if (!oval.has_value())
        throw FgException("Unable to convert string to type",str+":"+typeid(T).name());
    return oval.value();
}

// Ensures a minimum number of digits are printed:
template<class T>
String              toStrDigits(T val,size_t minDigits)
{
    std::ostringstream   oss;
    oss << std::setw(minDigits) << std::setfill('0') << val;
    return oss.str();
}

// Sets the desired total number of digits (precision):
template<class T>
String              toStrPrec(T val,uint numDigits)
{
    std::ostringstream   oss;
    oss.precision(numDigits);
    oss << val;
    return oss.str();
}

// Set the number of digits beyond fixed point:
String              toStrFixed(double val,uint fractionalDigits=0);
// Multiply by 100 and put a percent sign after:
String              toStrPercent(double val,uint fractionalDigits=0);
String              toStrRelPercents(double v0,double v1,uint fractionalDigits=0);  // compare two values by relative percents
String              toLower(String const & s);
String              toUpper(String const & s);
// Returned list of strings does NOT include separators but DOES include empty
// strings where there are consecutive separators:
Strings             splitAtSeparators(String const & str,char sep);
String              replaceAll(String const & str,char orig,char repl);
String              replaceFirst(String const & str,String const & substr,String const & replacement);
// returns 'true' if any replacement was done, result in 'ret'. Otherwise leaves 'ret' empty:
bool                replaceAll_(String const & str,String const & find,String const & repl,String & ret);
// Pad a String to desired len (does not truncate of longer):
String              padToLen(String const & str,size_t len,char ch=' ');
// concatenate strings with a separator between them (but not at beginning or end) like Python join():
String              cat(Strings const & strings,String const & separator);
String              catDeref(Ptrs<String> const & stringPtrs,String const & separator);

String              cRest(String const & str,size_t start);         // start must be <= str.length()
Str32               cRest(Str32 const & str,size_t start);       // "

// Returns the strings <prefix># from 0 through num-1, all with the same number of digits (as required):
Strings             numberedStrings(String const & prefix,size_t num);

struct  String8
{
    String          m_str;      // UTF-8 unicode

    String8() {};
    String8(char const * utf8_c_string) : m_str(utf8_c_string) {};
    String8(String const & utf8_string) : m_str(utf8_string) {};
    explicit String8(char32_t utf32_char) : m_str(toUtf8(utf32_char)) {}
    explicit String8(Str32 const & utf32) : m_str(toUtf8(utf32)) {}

    String8 &       operator+=(String8 const & rhs);
    String8 &       operator+=(char rhs);
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
    Str32        as_utf32() const {return toUtf32(m_str); }

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
};
typedef Svec<String8>   String8s;
typedef Svec<String8s>  String8ss;

String8s            toUstrings(Strings const & strs);

template<>
inline String       toStr(String8 const & str) {return str.m_str; }
template<class T>
inline void         fgThrow(String const & msg,T const & data)
{
    throw FgException(msg,toStr(data));
}
template<class T,class U>
inline void         fgThrow(String const & msg,T const data0,const U & data1) 
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

// Hex functions useful to create strings directly rather than going through ostringstream with 'std::hex'
// and 'std::uppercase' and uint-casting for uchar.
// Big-endian (ie highest order digits printed first)
// Signed and unsigned are treated identically (hex representation of bit pattern)
// Returns hex encoding in string of length 2:
String              toHexString(uchar c);
// Returns hex encoding in string of length 4:
String              toHexString(uint16 val);
inline String       toHexString(int16 val) {return toHexString(scast<uint16>(val)); }
// Returns hex encoding in string of length 8
String              toHexString(uint32 val);
inline String       toHexString(int32 val) {return toHexString(scast<uint32>(val)); }
// Returns hex encoding in string of length 16:
String              toHexString(uint64 val);
inline String       toHexString(int64 val) {return toHexString(scast<uint64>(val)); }
String              bytesToHexString(const uchar *arr,uint numBytes);
// from hex to value (ignores non-hex characters and accepts letters O and I as 0 and 1 resp.:
Valid<uint>         fromHex4(char ch);              // other characters than above yield invalid
uint64              fromHex64(String const & hex);
// Separate into 4 digit (16 bit word) chunks separated by dashes, and add a 4 digit CRC at end:
String              toHex64Crc(uint64 serialNum);
// Returns 0 if not a valid hex string or the CRC doesn't match:
uint64              fromHex64Crc(String const & serialString);
// Use to auto create labels with numbers appended:
Strings             cNumberedLabels(String const & baseLabel,size_t num);
String8s            cNumberedLabels(String8 const & baseLabel,size_t num);

}

#endif // INCLUDED_FGSTRING_HPP
