//
// Copyright (c) 2022 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Simple, slow, correct string tokenizing / parsing (functional multi-pass approach).
//

#ifndef FGPARSE_HPP
#define FGPARSE_HPP

#include "FgSerial.hpp"

namespace Fg {

inline bool         isDigit(char ch) {return std::isdigit(scast<uchar>(ch)); }
inline bool         isCrLf(uint ch) {return ((ch == '\r') || (ch == '\n')); }
// Returns true if empty:
bool                containsOnlyDigits(String const &);
// Returns a vector of tokens, treating all control codes and extended codes (high bit set) as whitespace,
// grouping all connected digit-letter-dash-underscore characters and considering any others as single-character
// tokens:
Strings             tokenize(String const &);
// Split a string into non-empty lines at CR/LF and remove all CR/LF characters.
// Use this instead of std::getline which leaves in CR characters on Windows.
// If 'commentFlag' is non-null, then any line starting with that character will be discarded.
Strings             splitLines(String const & src,char commentFlag=0);
String32s           splitLines(String32 const & src,char32_t commentFlag=0);
String8s            splitLinesUtf8(String const & utf8,char commentFlag=0);
// The exact UTF-8 string between commas will be taken as the value except for:
// * Newlines, which are interpreted as the start of a new record.
// * When a quote directly follows a comma, in which case everthing, including commas and newlines,
//   up until the next single-quote is taken as the value, and double-quotes are taken as single-quotes.
Stringss            loadCsv(
    String8 const &     fname,
    size_t              fieldsPerLine=0);   // If non-zero, non-empty lines must have this many fields.
// Each non-empty line in the CSV must have at least keyIdx+1 fields or an error will occur.
// The map value will contain all fields in original order (including key):
std::map<std::string,Strings> loadCsvToMap(
    String8 const &     fname,
    size_t              keyIdx,
    size_t              fieldsPerLine=0);   // If non-zero, non-empty lines must have this many fields.
// Quotes all fields and uses double-quotes to escape quote literals.
void                saveCsv(String8 const & fname,const Stringss & csvLines);
// Split up a string based on a seperator.
// Output does not include separators or (by default) empty strings.
Strings             splitChar(String const & str,char sep=' ',bool includeEmptyStrings=false);
// Breaks the given string into a vector of strings according to any whitespace, which is 
// removed. Quotation marks can be used to enclose symbols containing whitespace.
Strings             splitWhitespace(String const &);
// Map all non-ASCII characters that look like an ASCII character (homoglyphs) to that ASCII character,
// and all others to '?':
String              asciify(String const &);
String32            replaceAll(String32 const & str,char32_t a,char32_t b);     // Replace each 'a' with 'b'
String              noLeadingWhitespace(String const &);

}

#endif
