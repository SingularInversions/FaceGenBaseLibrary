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

using namespace std;

vector<string>
fgSplitLines(const string & src)
{
    vector<string>  ret;
    string          acc;
    for (size_t ii=0; ii<src.size(); ++ii) {
        if ((src[ii] == 0x0A) || (src[ii] == 0x0D)) {   // LF or CR resp.
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

vector<vector<string> >
fgReadCsvFile(const FgString & filename)
{
    vector<vector<string> >  ret;
    vector<string>      lines = fgSplitLines(fgSlurp(filename));
    for (size_t ii=0; ii<lines.size(); ++ii) {
        const string & line = lines[ii];
        vector<string>          csvLine;
        string                  field;
        for (uint idx=0; idx<line.size(); idx++) {
            if (line[idx] == ',') {
                csvLine.push_back(field);
                field.clear(); }
            else
                field.push_back(line[idx]); }
        csvLine.push_back(field);
        ret.push_back(csvLine); }
    return ret;
}

vector<string>
fgSplitChar(const string & str,char ch,bool ie)
{
    vector<string>  ret;
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
