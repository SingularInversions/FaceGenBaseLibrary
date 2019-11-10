//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Simple, slow, correct string tokenizing / parsing (functional multi-pass approach).
//

#ifndef FGPARSE_HPP
#define FGPARSE_HPP

#include "FgStdVector.hpp"
#include "FgStdString.hpp"
#include "FgString.hpp"

namespace Fg {

inline bool
fgIsCrLf(uint ch)
{return ((ch == '\r') || (ch == '\n')); }

inline bool
fgIsDigit(char c)
{return ((c >= '0') && (c <= '9')); }

inline bool
fgIsDigitOrMinus(char cc)
{return ((fgIsDigit(cc)) || (cc == '-')); }

inline bool
fgIsWhitespaceOrInvalid(char c)
{return ((c < 0x21) || (c > 0x7E)); }

inline bool
fgIsLetter(char c)
{return (((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z'))); }

inline bool
fgIsDigitOrLetter(char c)
{return fgIsDigit(c) || fgIsLetter(c); }

inline bool
fgIsDigitLetterDashUnderscore(char c)
{return fgIsDigit(c) || fgIsLetter(c) || (c == '-') || (c == '_'); }

inline
uint
fgAsciiToDigit(char cc)
{
    uint    ret = uint(cc) - uint('0');
    FGASSERT(ret < 10);
    return ret;
}

// Returns a vector of tokens, treating all control codes and extended codes (high bit set) as whitespace,
// grouping all connected digit-letter-dash-underscore characters and considering any others as single-character
// tokens:
Strings
fgTokenize(const String &);

// Split a string into non-empty lines at CR/LF and remove all CR/LF characters.
// Use this instead of useless std::getline which leaves in CR characters on Windows.
Strings
splitLines(String const & src);

FgStr32s
splitLines(const std::u32string & src,bool includeEmptyLines=false);

Ustrings
fgSplitLinesUtf8(const String & utf8,bool includeEmptyLines=false);

// The exact UTF-8 string between commas will be taken as the value except for:
// * Newlines, which are interpreted as the start of a new record.
// * When a quote directly follows a comma, in which case everthing, including commas and newlines,
//   up until the next single-quote is taken as the value, and double-quotes are taken as single-quotes.
Stringss
fgLoadCsv(
    const Ustring &    fname,
    size_t              fieldsPerLine=0);   // If non-zero, non-empty lines must have this many fields.

// Each non-empty line in the CSV must have at least keyIdx+1 fields or an error will occur.
// The map value will contain all fields in original order (including key):
std::map<std::string,Strings>
fgLoadCsvToMap(
    const Ustring &    fname,
    size_t              keyIdx,
    size_t              fieldsPerLine=0);   // If non-zero, non-empty lines must have this many fields.

// Quotes all fields and uses double-quotes to escape quote literals.
void
fgSaveCsv(const Ustring & fname,const Stringss & csvLines);

// Split up a string based on a seperator.
// Output does not include separators or (by default) empty strings.
// More convenient than boost::split
Strings
splitChar(const String & str,char sep=' ',bool includeEmptyStrings=false);

// Breaks the given string into a vector of strings according to any whitespace, which is 
// removed. Quotation marks can be used to enclose symbols containing whitespace.
Strings
fgWhiteBreak(const String &);

// Map all non-ASCII characters that look like an ASCII character (homoglyphs) to that ASCII character,
// and all others to '?':
String
fgAsciify(const String &);

std::u32string
fgReplace(const std::u32string & str,char32_t a,char32_t b);     // Replace each 'a' with 'b'

}

#endif
