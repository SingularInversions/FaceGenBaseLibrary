//
// Copyright (c) 2025 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Simple, slow, correct string tokenizing / parsing (functional multi-pass approach).
//

#ifndef FGPARSE_HPP
#define FGPARSE_HPP

#include "FgSerial.hpp"
#include "FgAny.hpp"

namespace Fg {

inline bool         isDigit(char ch) {return std::isdigit(scast<uchar>(ch)); }
inline bool         isCrLf(uint ch) {return ((ch == '\r') || (ch == '\n')); }
inline bool         isWhitespace(char c) {return (c < 0x21); }
inline bool         isLetter(char ch) {return isalpha(scast<uchar>(ch)); }
inline bool         isDigitLetterDashUnderscore(char c) {return isDigit(c) || isLetter(c) || (c == '-') || (c == '_'); }
inline bool         isWhitespaceOrInvalid(char c) {return ((c < 0x21) || (c > 0x7E)); }
inline bool         isCrLf32(char32_t ch) {return ((ch == 0x0A) || (ch == 0x0D)); }

// Returns true if empty:
bool                containsOnlyDigits(String const &);
// Returns a vector of tokens, treating all control codes and extended codes (high bit set) as whitespace,
// grouping all connected digit-letter-dash-underscore characters and considering any others as single-character
// tokens:
Strings             tokenize(String const &);
// Split a string into non-empty lines at CR/LF and remove all CR/LF characters.
// Use this instead of std::getline which leaves in CR characters on Windows.
// If 'commentFlag' is non-null, then any line starting with that character will be discarded, even if 'includeEmptyLines' is true:
Strings             splitLines(String const & src,char commentFlag=0);
Str32s              splitLines(Str32 const & src,char32_t commentFlag=0,bool includeEmptyLines=false);
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
Str32s              splitChar(Str32 const & str,char32_t splitToken,bool includeEmptyStrings=false);
// Breaks the given string into a vector of strings according to any whitespace, which is 
// removed. Quotation marks can be used to enclose symbols containing whitespace.
Strings             splitWhitespace(String const &);
// Map all non-ASCII characters that look like an ASCII character (homoglyphs) to that ASCII character,
// and all others to '?':
String              asciify(String const &);
Str32               replaceAll(Str32 const & str,char32_t a,char32_t b);     // Replace each 'a' with 'b'
String              noLeadingWhitespace(String const &);
// converts space, quote, percent, colon and non-ASCII to hex escape codes. Results is all ASCII:
String              encodeUrl(String8 const & str);
// does the reverse; result may not be ASCII:
String8             decodeUrl(String const & ascii);
// world's simplest JSON parser:
// * only handles ASCII; assumes all non-ascii (eg. UTF-8) and special chars (space, %, ", :) are URL encoded
// * A node can be one of:
//     - A literal: string, double, bool or null (JsonNUll)
//     - An array of nodes
//     - An object (array of JsonPair):
struct          JsonNull {};
struct          JsonPair
{
    String          name;
    Any             val;
    JsonPair(String const & n,Any const & v) : name{n}, val{v} {}

    bool            operator==(String const & n) const {return (n==name); }
};
typedef Svec<JsonPair>  JsonObject;     // order not important

Any                 parseJson_(String const & json,size_t & pos);
inline Any          parseJson(String const & json) {size_t pos{0}; return parseJson_(json,pos); }
String              writeJson(Any const & node);

}

#endif
