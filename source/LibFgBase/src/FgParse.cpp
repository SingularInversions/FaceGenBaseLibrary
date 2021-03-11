//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgStdStream.hpp"
#include "FgTypes.hpp"
#include "FgException.hpp"
#include "FgParse.hpp"
#include "FgFileSystem.hpp"
#include "FgSyntax.hpp"

using namespace std;

namespace Fg {

static bool
isLetter(char c)
{return (((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z'))); }

static bool
isDigitLetterDashUnderscore(char c)
{return isDigit(c) || isLetter(c) || (c == '-') || (c == '_'); }

static bool
isWhitespaceOrInvalid(char c)
{return ((c < 0x21) || (c > 0x7E)); }

bool
containsOnlyDigits(String const & str)
{
    for (char c : str)
        if (!isDigit(c))
            return false;
    return true;
}

Strings
tokenize(string const & str)
{
    Strings      ret;
    string      acc;
    for (char c : str) {
        if (isWhitespaceOrInvalid(c)) {
            if (!acc.empty()) {
                ret.push_back(acc);
                acc.clear();
            }
        }
        else if (isDigitLetterDashUnderscore(c)) {
            if (acc.empty())
                acc.push_back(c);
            else {
                if (isDigitLetterDashUnderscore(acc.back()))
                    acc.push_back(c);
                else {
                    ret.push_back(acc);
                    acc.clear();
                    acc.push_back(c);
                }
            }
        }
    }
    if (!acc.empty())
        ret.push_back(acc);
    return ret;
}

static
bool
isCrOrLf(char c)
{
    return ((c == 0x0A) || (c == 0x0D));    // LF or CR resp.
}

Strings
splitLines(string const & src)
{
    Strings         ret;
    string          acc;
    for (size_t ii=0; ii<src.size(); ++ii) {
        if (isCrOrLf(src[ii])) {
            if (!acc.empty()) {
                ret.push_back(acc);
                acc.clear();
            }
        }
        else
            acc += src[ii];
    }
    if (!acc.empty())
        ret.push_back(acc);
    return ret;
}

String32s
splitLines(String32 const & src,bool incEmpty)
{
    String32s            ret;
    size_t              base = 0;
    for (size_t ii=0; ii<src.size(); ++ii) {
        if ((src[ii] == 0x0A) || (src[ii] == 0x0D)) {   // LF or CR resp.
            if ((ii > base) || incEmpty)
                ret.push_back(String32(src.begin()+base,src.begin()+ii));
            base = ii+1; } }
    if (base < src.size())
        ret.push_back(String32(src.begin()+base,src.end()));
    return ret;
}

String8s
splitLinesUtf8(string const & utf8,bool includeEmptyLines)
{
    String8s               ret;
    String32s       res = splitLines(String8(utf8).as_utf32(),includeEmptyLines);
    ret.resize(res.size());
    for (size_t ii=0; ii<res.size(); ++ii)
        ret[ii] = String8(res[ii]);
    return ret;
}

static
void
consumeCrLf(String32 const & in,size_t & idx)    // Current idx must point to CR or LF
{
    char32_t        ch0 = in[idx++];
    if (idx == in.size())
        return;
    char32_t        ch1 = in[idx];
    if (isCrLf(ch1) && (ch0 != ch1))            // Allow for both CR/LF (Windows) and LF/CR (RISC OS)
        ++idx;
    return;
}

static
string
csvGetField(String32 const & in,size_t & idx)    // idx must initially point to valid data but may point to end on return
{
    string      ret;
    if (in[idx] == '"') {               // Quoted field
        ++idx;
        for (;;) {
            if (idx == in.size())
                return ret;
            char32_t    ch = in[idx++];
            if (ch == '"') {
                if ((idx < in.size()) && (in[idx] == '"')) {    // Double quote inside quoted field
                    ret.push_back('"');
                    ++idx;
                }
                else
                    return ret;         // End of quoted field
            }
            else
                ret += toUtf8(ch);
        }
    }
    else {                             // Unquoted field
        for (;;) {
            if (idx == in.size())
                return ret;
            char32_t    ch = in[idx];
            if ((ch == ',') || (isCrLf(ch)))
                return ret;
            ret += toUtf8(ch);
            ++idx;
        }
    }
}

static
Strings
csvGetLine(
    String32 const & in,
    size_t &        idx)    // idx must initially point to valid data but may point to end on return
{
    Strings      ret;
    if (isCrLf(in[idx])) {  // Handle special case of empty line to avoid interpreting it as single empty field
        consumeCrLf(in,idx);
        return ret;
    }
    for (;;) {
        ret.push_back(csvGetField(in,idx));
        if (idx == in.size())
            return ret;
        if (isCrLf(in[idx])) {
            consumeCrLf(in,idx);
            return ret;
        }
        if (in[idx] == ',')
            ++idx;
        if (idx == in.size()) {
            ret.push_back(string());
            return ret;
        }
    }
}

Stringss
loadCsv(String8 const & fname,size_t fieldsPerLine)
{
    Stringss         ret;
    String32       data = toUtf32(loadRawString(fname));
    size_t          idx = 0;
    while (idx < data.size()) {
        Strings      line = csvGetLine(data,idx);
        if (!line.empty()) {                    // Ignore empty lines
            if ((fieldsPerLine > 0) && (line.size() != fieldsPerLine))
                fgThrow("CSV file contains a line with incorrect field width",fname);
            ret.push_back(line);
        }
    }
    return ret;
}

map<string,Strings>
loadCsvToMap(String8 const & fname,size_t keyIdx,size_t fieldsPerLine)
{
    FGASSERT(keyIdx < fieldsPerLine);
    map<string,Strings>  ret;
    String32           data = toUtf32(loadRawString(fname));
    size_t              idx = 0;
    while (idx < data.size()) {
        Strings          line = csvGetLine(data,idx);
        if ((fieldsPerLine > 0) && (line.size() != fieldsPerLine))
            fgThrow("CSV file contains a line with incorrect field width",fname);
        string const &  key = line[keyIdx];
        auto            it = ret.find(key);
        if (it == ret.end())
            ret[key] = line;
        else
            fgThrow("CSV file key is not unique",fname,key);
    }
    return ret;
}

static
string
csvField(string const & data)
{
    string          ret = "\"";
    String32       utf32 = toUtf32(data);
    for (char32_t ch32 : utf32) {
        if (ch32 == char32_t('"'))      // VS2013 doesn't support char32_t literal U
            ret += "\"\"";
        else
            ret += toUtf8(ch32);
    }
    ret += "\"";
    return ret;
}

void
saveCsv(String8 const & fname,const Stringss & csvLines)
{
    Ofstream      ofs(fname);
    for (Strings line : csvLines) {
        if (!line.empty()) {
            ofs << csvField(line[0]);
            for (size_t ii=1; ii<line.size(); ++ii) {
                ofs << "," << csvField(line[ii]);
            }
            ofs << '\n';
        }
    }
}

Strings
splitChar(string const & str,char ch,bool ie)
{
    Strings         ret;
    string          curr;
    for (size_t ii=0; ii<str.size(); ++ii) {
        if (str[ii] == ch) {
            if (!curr.empty() || ie) {
                ret.push_back(curr);
                curr.clear();
            }
        }
        else
            curr += (str[ii]);
    }
    if (!curr.empty() || ie)
        ret.push_back(curr);
    return ret;
}

Strings
splitWhitespace(string const & str)
{
    Strings  retval;
    bool            symbolFlag = false,
                    quoteFlag = false;
    string          currSymbol;
    for (size_t ii=0; ii<str.size(); ++ii)
    {
        if (quoteFlag)
        {
            if (str[ii] == '\"')
            {
                retval.push_back(currSymbol);
                currSymbol.clear();
                quoteFlag = false;
            }
            else
                currSymbol.push_back(str[ii]);
        }
        else if (isspace(str[ii]))
        {
            if (symbolFlag)
            {
                retval.push_back(currSymbol);
                currSymbol.clear();
                symbolFlag = false;
            }
        }
        else if (str[ii] == '\"')
        {
            if (symbolFlag)
            {
                retval.push_back(currSymbol);
                currSymbol.clear();
            }
            quoteFlag = true;
        }
        else
        {
            currSymbol.push_back(str[ii]);
            symbolFlag = true;
        }
    }
    if (symbolFlag)
        retval.push_back(currSymbol);
    return retval;
}

void
fgTestmLoadCsv(CLArgs const & args)
{
    Syntax        syntax(args,"<file>.csv");
    Stringss         data = loadCsv(syntax.next());
    for (size_t rr=0; rr<data.size(); ++rr) {
        Strings const &  fields = data[rr];
        fgout << fgnl << "Record " << rr << " with " << fields.size() << " fields: " << fgpush;
        for (size_t ff=0; ff<fields.size(); ++ff)
            fgout << fgnl << "Field " << ff << ": " << fields[ff];
        fgout << fgpop;
    }
}

string
asciify(string const & in)
{
    string          ret;
    map<char32_t,char>  hg;     // homoglyph map
    hg[166] = ':';
    hg[167] = 'S';
    hg[168] = '"';
    hg[183] = '.';
    hg[193] = 'A';
    hg[194] = 'A';
    hg[195] = 'A';
    hg[199] = 'C';
    hg[211] = 'o';
    hg[216] = 'o';
    hg[223] = 'B';
    hg[224] = 'a';
    hg[225] = 'a';
    hg[226] = 'a';
    hg[227] = 'a';
    hg[228] = 'a';
    hg[230] = 'a';
    hg[231] = 'c';
    hg[232] = 'e';
    hg[233] = 'e';
    hg[236] = 'i';
    hg[237] = 'i';
    hg[239] = 'i';
    hg[241] = 'n';
    hg[242] = 'o';
    hg[243] = 'o';
    hg[246] = 'o';
    hg[248] = 'o';
    hg[250] = 'u';
    hg[252] = 'u';
    hg[304] = 'I';
    hg[305] = '1';
    hg[351] = 's';
    hg[402] = 'f';
    hg[732] = '"';
    hg[8211] = '-';
    hg[8216] = '\'';
    hg[8218] = ',';
    hg[8220] = '"';
    hg[8221] = '"';
    hg[8230] = '-';
    hg[65381] = '\'';
    String32       utf32 = toUtf32(in);
    for (char32_t ch32 : utf32) {
        string  utf8 = toUtf8(ch32);
        if (utf8.size() == 1)
            ret.push_back(utf8[0]);
        else {
            auto    it = hg.find(ch32);
            if (it == hg.end())
                ret.push_back('?');
            else
                ret.push_back(it->second);
        }
    }
    return ret;
}

String32
replaceAll(String32 const & str,char32_t a,char32_t b)
{
    String32       ret;
    ret.reserve(str.size());
    for (const char32_t & c : str)
        if (c == a)
            ret.push_back(b);
        else
            ret.push_back(c);
    return ret;
}

}
