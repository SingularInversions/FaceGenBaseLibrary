//
// Copyright (c) 2025 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgFile.hpp"
#include "FgSerial.hpp"
#include "FgParse.hpp"
#include "FgFileSystem.hpp"
#include "FgCommand.hpp"

using namespace std;

namespace Fg {

bool                containsOnlyDigits(String const & str)
{
    for (char c : str)
        if (!isDigit(c))
            return false;
    return true;
}

Strings             tokenize(String const & str)
{
    Strings             ret;
    String              acc;
    for (char c : str) {
        if (isDigitLetterDashUnderscore(c))
            acc.push_back(c);
        else {                                  // either whitespace or single-character token:
            if (!acc.empty()) {
                ret.push_back(acc);
                acc.clear();
            }
            if (!isWhitespaceOrInvalid(c))      // must be single-character token
                ret.push_back(String{c});
        }
    }
    if (!acc.empty())
        ret.push_back(acc);
    return ret;
}

Strings             splitLines(String const & src,char commentFlag)
{
    Strings             ret;
    String              acc;
    for (char ch : src) {
        if (isCrLf(ch)) {
            if (!acc.empty()) {
                if ((commentFlag == 0) || (acc[0] != commentFlag))
                    ret.push_back(acc);
                acc.clear();
            }
        }
        else
            acc += ch;
    }
    if (!acc.empty())
        if (acc[0] != commentFlag)
            ret.push_back(acc);
    return ret;
}

Str32s           splitLines(Str32 const & src,char32_t commentFlag,bool ie)
{
    Str32s           ret;
    Str32            acc;
    for (char32_t ch : src) {
        if (isCrLf32(ch)) {
            if (!acc.empty() || ie) {
                if ((commentFlag == 0) || (acc[0] != commentFlag))
                    ret.push_back(acc);
                acc.clear();
            }
        }
        else
            acc += ch;
    }
    if (!acc.empty() || ie)
        if ((commentFlag == 0) || (acc[0] != commentFlag))
            ret.push_back(acc);
    return ret;
}

String8s            splitLinesUtf8(String const & utf8,char commentFlag)
{
    Str32s           lines = splitLines(toUtf32(utf8),scast<char32_t>(commentFlag));
    String8s            ret; ret.reserve(lines.size());
    for (Str32 const & line : lines)
        ret.push_back(toUtf8(line));
    return ret;
}

static void         consumeCrLf(Str32 const & in,size_t & idx)    // Current idx must point to CR or LF
{
    char32_t        ch0 = in[idx++];
    if (idx == in.size())
        return;
    char32_t        ch1 = in[idx];
    if (isCrLf(ch1) && (ch0 != ch1))            // Allow for both CR/LF (Windows) and LF/CR (RISC OS)
        ++idx;
    return;
}

static String       csvGetField(
    Str32 const &        in,
    size_t &                idx)    // idx must initially point to valid data but may point to end on return
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

static Strings      csvGetLine(
    Str32 const & in,
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

Stringss            loadCsv(String8 const & fname,size_t fieldsPerLine)
{
    Stringss         ret;
    Str32       data = toUtf32(loadRawString(fname));
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

map<string,Strings> loadCsvToMap(String8 const & fname,size_t keyIdx,size_t fieldsPerLine)
{
    FGASSERT((fieldsPerLine==0) || (keyIdx < fieldsPerLine));
    map<string,Strings> ret;
    Str32            data = toUtf32(loadRawString(fname));
    size_t              idx = 0;
    while (idx < data.size()) {
        Strings             line = csvGetLine(data,idx);
        if ((fieldsPerLine > 0) && (line.size() != fieldsPerLine))
            fgThrow("CSV file contains a line with incorrect field width",fname);
        if (keyIdx >= line.size())
            fgThrow("CSV file line does not contain the key",line);
        string const &      key = line[keyIdx];
        auto                it = ret.find(key);
        if (it == ret.end())
            ret[key] = line;
        else
            fgThrow("CSV file key is not unique",fname,key);
    }
    return ret;
}

static string       csvField(string const & data)
{
    string          ret = "\"";
    Str32       utf32 = toUtf32(data);
    for (char32_t ch32 : utf32) {
        if (ch32 == char32_t('"'))      // VS2013 doesn't support char32_t literal U
            ret += "\"\"";
        else
            ret += toUtf8(ch32);
    }
    ret += "\"";
    return ret;
}

void                saveCsv(String8 const & fname,const Stringss & csvLines)
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

Strings             splitChar(String const & str,char ch,bool ie)
{
    Strings         ret;
    String          curr;
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

Str32s              splitChar(Str32 const & str,char32_t token,bool ies)
{
    Str32s              ret;
    Str32               curr;
    for (char32_t ch : str) {
        if (ch == token) {
            if (!curr.empty() || ies) {
                ret.push_back(curr);
                curr.clear();
            }
        }
        else
            curr += ch;
    }
    if (!curr.empty() || ies)
        ret.push_back(curr);
    return ret;
}

Strings             splitWhitespace(String const & str)
{
    Strings             retval;
    bool                symbolFlag = false,
                        quoteFlag = false;
    String              currSymbol;
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

void                testmLoadCsv(CLArgs const & args)
{
    Syntax              syntax(args,"<file>.csv");
    Stringss            data = loadCsv(syntax.next());
    for (size_t rr=0; rr<data.size(); ++rr) {
        Strings const &     fields = data[rr];
        fgout << fgnl << "Record " << rr << " with " << fields.size() << " fields: " << fgpush;
        for (size_t ff=0; ff<fields.size(); ++ff)
            fgout << fgnl << "Field " << ff << ": " << fields[ff];
        fgout << fgpop;
    }
}

String              asciify(string const & in)
{
    String              ret;
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
    Str32            utf32 = toUtf32(in);
    for (char32_t ch32 : utf32) {
        string              utf8 = toUtf8(ch32);
        if (utf8.size() == 1)
            ret.push_back(utf8[0]);
        else {
            auto            it = hg.find(ch32);
            if (it == hg.end())
                ret.push_back('?');
            else
                ret.push_back(it->second);
        }
    }
    return ret;
}

Str32            replaceAll(Str32 const & str,char32_t a,char32_t b)
{
    Str32       ret;
    ret.reserve(str.size());
    for (const char32_t & c : str)
        if (c == a)
            ret.push_back(b);
        else
            ret.push_back(c);
    return ret;
}

String              noLeadingWhitespace(String const & str)
{
    size_t              idx = 0;
    while (isWhitespace(str[idx]))
        ++idx;
    return str.substr(idx);
}

String                  encodeUrl(String8 const & str)
{
    String                  ret;
    for (char ch : str.m_str) {
        if (ch == ' ')
            ret += "%20";
        else if (ch == '"')
            ret += "%22";
        else if (ch == '%')
            ret += "%25";
        else if (ch == ':')
            ret += "%3A";
        else if (ch & 0x80)     // non-ASCII
            ret += "%" + toHexString(scast<uchar>(ch));
        else
            ret += ch;
    }
    return ret;
}

String8             decodeUrl(String const & str)
{
    String8                 ret;
    size_t                  idx {0};
    auto                    getHexDigit = [&]()
    {
        Valid<uint>         val = fromHex4(str[idx++]);
        if (!val.valid())
            fgThrow("decodeUrl invalid hex digit",str[idx-1]);
        return val.val();
    };
    while (idx < str.size()) {
        char                ch = str[idx++];
        if (ch != '%')
            ret.m_str += ch;
        else {
            if (idx+2 >= str.size())
                fgThrow("decodeUrl truncated control code",toStr(idx));
            uint                val = getHexDigit() * 16;
            val += getHexDigit();
            FGASSERT(val < 256);
            ret.m_str += scast<char>(val);
        }
    }
    return ret;
}

Any                 parseJson_(String const & json,size_t & pos)
{
    size_t              line {0};
    auto                isWhite = [](char ch)   // commas are whitespace
    {
        return (isWhitespace(ch) || (ch==','));
    };
    auto                skipWhitespace = [&]()
    {
        while ((pos < json.size()) && isWhite(json[pos])) {
            if (json[pos] == '\n')
                ++line;
            ++pos;
        }
    };
    auto                readToken = [&]()
    {
        String              tok;
        while (!isWhite(json.at(pos)))
            tok += json[pos++];
        return tok;
    };
    auto                readString = [&]()
    {
        FGASSERT(json.at(pos++) == '"');
        String              name;
        while (json.at(pos) != '"')
            name += json[pos++];
        ++pos;                          // eat closing quote
        return name;
    };
    while (pos < json.size()) {
        skipWhitespace();
        char                ch = json.at(pos);
        if (ch == '{') {                        // property array
            ++pos;                              // eat opening brace
            skipWhitespace();
            JsonObject          object;
            while (json.at(pos) != '}') {
                skipWhitespace();
                if (json[pos] != '"')
                    fgThrow("Property name must be enclosed in quotes on line",toStr(line));
                String              name = readString();
                skipWhitespace();
                if (json[pos++] != ':')
                    fgThrow("Property name must be followed by a colon on line",toStr(line));
                object.emplace_back(name,parseJson_(json,pos));
                skipWhitespace();
            }
            ++pos;                              // eat closing brace
            return Any{object};
        }
        else if (ch == '[') {                   // array
            ++pos;                              // eat opening brace
            skipWhitespace();
            Anys                elements;
            while (json.at(pos) != ']') {
                elements.push_back(parseJson_(json,pos));
                skipWhitespace();
            }
            ++pos;                              // eat closing brace
            return Any{elements};
        }
        else if (ch == '"')
            return Any{readString()};
        else if (isDigit(ch) || (ch=='-'))
            return Any{fromStr<double>(readToken()).value()};
        else {
            String              tok = readToken();
            if (tok == "true")
                return Any{true};
            if (tok == "false")
                return Any{false};
            if (tok == "null")
                return Any{JsonNull{}};
            fgThrow("parseJson unrecognized token",tok);
        }
    }
    return {};
}

void                writeJson_(Any const & node,bool contLine,size_t indent,String & str)
{
    auto                isAtomic = [](Any const & n)
    {
        return ((n.is<double>()) || (n.is<String>()) || (n.is<bool>()) || (n.is<JsonNull>()));
    };
    String              indStr (indent,'\t');
    if (node.is<JsonObject>()) {
        JsonObject const &      obj = node.as<JsonObject>();
        if (!contLine)
            str += "\n" + indStr;
        str += "{";
        for (size_t ii=0; ii<obj.size(); ++ii) {
            str += "\n" + indStr + "\t\"" + obj[ii].name + "\" : ";
            writeJson_(obj[ii].val,true,indent+1,str);
            if (ii+1 < obj.size())
                str += ",";
        }
        str += "\n" + indStr + "}";
    }
    else if (node.is<Anys>()) {
        Anys const &            arr = node.as<Anys>();
        if (!contLine)
            str += "\n" + indStr;
        str += "[";
        if (arr.empty())
            str += " ]";
        else if (isAtomic(arr[0])) {                    // single-line array (assume all are atomic if first is)
            for (size_t ii=0; ii<arr.size(); ++ii) {
                str += " ";
                writeJson_(arr[ii],true,indent,str);
                if (ii+1 < arr.size())
                    str += ",";
            }
            str += " ]";
        }
        else {                                          // per-line array
            for (size_t ii=0; ii<arr.size(); ++ii) {
                writeJson_(arr[ii],false,indent+1,str);
                if (ii+1 < arr.size())
                    str += ",";
            }
            str += "\n" + indStr + "]";
        }
    }
    else {                                              // atomic
        String                  atom;
        if (node.is<double>())
            atom = toStrPrec(node.as<double>(),7);
        else if (node.is<String>())
            atom = "\"" + node.as<String>() + "\"";
        else if (node.is<bool>())
            atom = node.as<bool>() ? "true" : "false";
        else if (node.is<JsonNull>())
            atom = "null";
        else
            fgThrow("writeJson node unrecognized type",node.typeName());
        if (contLine)
            str += atom;
        else
            str += "\n" + indStr + atom;
    }
}

String              writeJson(Any const & node)
{
    String              ret;
    writeJson_(node,true,0,ret);
    return ret;
}

namespace {

void                testUrl(CLArgs const &)
{
    String8             str = "C:/A Path/";         // colon and space must be encoded
    str += scast<char>(0xE4);                       // add UTF-8 chinese character 
    str += scast<char>(0xB8);
    str += scast<char>(0xAD);
    str += "/File.txt";
    FGASSERT(decodeUrl(encodeUrl(str)) == str);
}

void                testJson(CLArgs const &)
{
    String              json {R"(
{
    "name2": "stringVal",
    "name3": true,
    "name4": {
        "name4-1": 16,
        "Name4-2": 42.25
    },
    "name5": [
        {
            "name5-1-1": null,
            "name5-1-2": false
        },
        {
            "name5-1-3": [],
            "name5-1-4": [ 1,2,3,4 ]
        }
    ]
})"
    };
    auto                testFn = [](Any const & node0)
    {
        JsonObject          obj0 = node0.as<JsonObject>();
        FGASSERT(obj0[0].name == "name2");
        FGASSERT(obj0[0].val.as<String>() == "stringVal");
        FGASSERT(obj0[1].name == "name3");
        FGASSERT(obj0[1].val.as<bool>() == true);       // on purpose, Joel Spolsky !
        FGASSERT(obj0[2].name == "name4");
        JsonObject          obj1 = obj0[2].val.as<JsonObject>();
        FGASSERT(obj1[0].name == "name4-1");
        FGASSERT(obj1[0].val.as<double>() == 16);
        FGASSERT(obj1[1].name == "Name4-2");
        FGASSERT(obj1[1].val.as<double>() == 42.25);
        FGASSERT(obj0[3].name == "name5");
        Anys                arr = obj0[3].val.as<Anys>();
        JsonObject          obj30 = arr[0].as<JsonObject>();
        FGASSERT(obj30[0].name == "name5-1-1");
        FGASSERT(obj30[0].val.is<JsonNull>());
        FGASSERT(obj30[1].name == "name5-1-2");
        FGASSERT(obj30[1].val.as<bool>() == false);
        JsonObject          obj31 = arr[1].as<JsonObject>();
        FGASSERT(obj31[0].name == "name5-1-3");
        FGASSERT(obj31[0].val.as<Anys>().empty());
        FGASSERT(obj31[1].name == "name5-1-4");
        FGASSERT(obj31[1].val.as<Anys>().size() == 4);
    };
    Any                 node0 = parseJson(json);
    testFn(node0);
    String              json2 = writeJson(node0);
    testFn(parseJson(json2));
}

}

void                testParse(CLArgs const & args)
{
    Cmds            cmds {
        {testJson,"json","JSON to text and vice versa"},
        {testUrl,"url","URL encoding and decoding"},
    };
    doMenu(args,cmds,true);
}



}
