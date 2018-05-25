//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     April 21, 2005
//

#include "stdafx.h"

#include "FgTypes.hpp"
#include "FgException.hpp"
#include "FgParse.hpp"
#include "FgFileSystem.hpp"
#include "FgSyntax.hpp"

using namespace std;

FgStrs
fgTokenize(const string & str)
{
    FgStrs      ret;
    string      acc;
    for (char c : str) {
        if (fgIsWhitespaceOrInvalid(c)) {
            if (!acc.empty()) {
                ret.push_back(acc);
                acc.clear();
            }
        }
        else if (fgIsDigitLetterDashUnderscore(c)) {
            if (acc.empty())
                acc.push_back(c);
            else {
                if (fgIsDigitLetterDashUnderscore(acc.back()))
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

FgStrs
fgSplitLines(const string & src,bool backslashContinuation)
{
    FgStrs          ret;
    string          acc;
    for (size_t ii=0; ii<src.size(); ++ii) {
        if (backslashContinuation && (src[ii] == '\\')) {
            if ((ii+1 < src.size()) && (isCrOrLf(src[ii+1]))) {
                ++ii;
                while ((ii+1 < src.size()) && (isCrOrLf(src[ii])))
                    ++ii;
            }
        }
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

vector<vector<uint32> >
fgSplitLines(const vector<uint32> & src,bool incEmpty)
{
    vector<vector<uint32> > ret;
    size_t          base = 0;
    for (size_t ii=0; ii<src.size(); ++ii) {
        if ((src[ii] == 0x0A) || (src[ii] == 0x0D)) {   // LF or CR resp.
            if ((ii > base) || incEmpty)
                ret.push_back(vector<uint32>(src.begin()+base,src.begin()+ii));
            base = ii+1; } }
    if (base < src.size())
        ret.push_back(vector<uint32>(src.begin()+base,src.end()));
    return ret;
}

FgStrings
fgSplitLinesUtf8(const string & utf8,bool includeEmptyLines)
{
    FgStrings            ret;
    vector<vector<uint32> >     res = fgSplitLines(FgString(utf8).as_utf32(),includeEmptyLines);
    ret.resize(res.size());
    for (size_t ii=0; ii<res.size(); ++ii)
        ret[ii] = FgString(res[ii]);
    return ret;
}

bool
isCrLf(uint ch)
{return ((ch == '\r') || (ch == '\n')); }

static
void
consumeCrLf(const FgUints & in,size_t & idx)    // Current idx must point to CR or LF
{
    uint        ch0 = in[idx++];
    if (idx == in.size())
        return;
    uint        ch1 = in[idx];
    if (isCrLf(ch1) && (ch0 != ch1))            // Allow for both CR/LF (Windows) and LF/CR (RISC OS)
        ++idx;
    return;
}

static
string
csvGetField(const FgUints & in,size_t & idx)    // idx must initially point to valid data but may point to end on return
{
    string      ret;
    if (in[idx] == '"') {               // Quoted field
        ++idx;
        for (;;) {
            if (idx == in.size())
                return ret;
            uint    ch = in[idx++];
            if (ch == '"') {
                if ((idx < in.size()) && (in[idx] == '"')) {    // Double quote inside quoted field
                    ret.push_back('"');
                    ++idx;
                }
                else
                    return ret;         // End of quoted field
            }
            else
                ret += fgUtf32ToUtf8(fgSvec(ch));
        }
    }
    else {                             // Unquoted field
        for (;;) {
            if (idx == in.size())
                return ret;
            uint    ch = in[idx];
            if ((ch == ',') || (isCrLf(ch)))
                return ret;
            ret += fgUtf32ToUtf8(fgSvec(ch));
            ++idx;
        }
    }
}

static
FgStrs
csvGetLine(
    const FgUints & in,
    size_t &        idx)    // idx must initially point to valid data but may point to end on return
{
    FgStrs      ret;
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
    return ret;
}

FgStrss
fgLoadCsv(const FgString & fname)
{
    FgStrss     ret;
    FgUints     data = fgUtf8ToUtf32(fgSlurp(fname));
    size_t      idx = 0;
    while (idx < data.size()) {
        FgStrs  line = csvGetLine(data,idx);
        if (!line.empty())                      // Ignore empty lines
            ret.push_back(line);
    }
    return ret;
}

static
string
csvField(const string & data)
{
    string      ret = "\"";
    FgUints     utf32 = fgUtf8ToUtf32(data);
    for (uint ch32 : utf32) {
        if (ch32 == uint('"'))
            ret += "\"\"";
        else
            ret += fgUtf32ToUtf8(fgSvec(ch32));
    }
    ret += "\"";
    return ret;
}

void
fgSaveCsv(const FgString & fname,const FgStrss & csvLines)
{
    FgOfstream      ofs(fname);
    for (FgStrs line : csvLines) {
        if (!line.empty()) {
            ofs << csvField(line[0]);
            for (size_t ii=1; ii<line.size(); ++ii) {
                ofs << "," << csvField(line[ii]);
            }
            ofs << endl;
        }
    }
}

FgStrs
fgSplitChar(const string & str,char ch,bool ie)
{
    FgStrs  ret;
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

FgStrs
fgWhiteBreak(const string & str)
{
    FgStrs  retval;
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
fgTestmLoadCsv(const FgArgs & args)
{
    FgSyntax        syntax(args,"<file>.csv");
    FgStrss         data = fgLoadCsv(syntax.next());
    for (size_t rr=0; rr<data.size(); ++rr) {
        const FgStrs &  fields = data[rr];
        fgout << fgnl << "Record " << rr << " with " << fields.size() << " fields: " << fgpush;
        for (size_t ff=0; ff<fields.size(); ++ff)
            fgout << fgnl << "Field " << ff << ": " << fields[ff];
        fgout << fgpop;
    }
}

string
fgAsciify(const string & in)
{
    string          ret;
    map<uint,char>  hg;     // homoglyph map
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
    FgUints     utf32 = fgUtf8ToUtf32(in);
    for (uint ch32 : utf32) {
        string  utf8 = fgUtf32ToUtf8(fgSvec(ch32));
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
