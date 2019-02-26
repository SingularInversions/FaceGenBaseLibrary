//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Sohail Somani
//

#include "stdafx.h"

#include "FgTestUtils.hpp"
#include "FgString.hpp"
#include "FgMain.hpp"

static void Construct()
{
    {   FgString s;
        FGASSERT(s.length() == 0); }
    {   FgString s("12345");
        FGASSERT(s.length() == 5);}
#ifdef _WIN32
    {   FgString s(L"12345");
        FGASSERT(s.length() == 5); }
#endif
    {   FgString s1("12345");
        FgString s2(s1);
        FGASSERT(s1.length() == s2.length() && s1.length() == 5); }
}

static void Copy()
{
    {
        // Ensure that we are copying on copy
        FgString original("abcd");
        FgString copy(original);
        FGASSERT(copy == original);
        copy += "efghi";
        FGASSERT(copy != original);
    }
}

static void Assign()
{
    {   FgString s;
        s = "12345";
        FGASSERT(s.length() == 5); }
#ifdef _WIN32
    {   FgString s;
        s = L"12345";
        FGASSERT(s.length() == 5); }
    {   FgString s;
        s = L"12345";
        FgString s1;
        s1 = s;
        FGASSERT(s1.length() == 5); }
#endif
}

static void Append()
{
    {   FgString s("12345");
        s += "12345";
        FGASSERT(s.length() == 10); }
#ifdef _WIN32
    {   FgString s("12345");
        s += L"12345";
        FGASSERT(s.length() == 10); }
#endif
    {   FgString s("12345");
        s += s;
        FGASSERT(s.length() == 10); }
    {   FgString s("abcd");
        FGASSERT(s=="abcd");
        s+="efg";
        FGASSERT(s!="abcd");
        FGASSERT(s=="abcdefg"); }
}

static void Comparisons()
{
#ifdef _WIN32
    {   FgString s("12345");
        FGASSERT(s == "12345");
        FGASSERT(s == L"12345");
        FGASSERT(s == FgString("12345"));
        FGASSERT(FgString("12345") == s); }
    {   FgString s("12345");
        FGASSERT(s != "abcdefg");
        FGASSERT(s != L"Hi");
        FGASSERT(s != FgString("Bye"));
        FGASSERT(s == "12345"); }
#endif
    {   FgString s("abcdefg");
        FGASSERT(s == "abcdefg");
        FGASSERT(s != "abc"); }
}

static void Encoding()
{
    {
        // Character as Unicode code point
        uint32  character = 0x0161;
        // Same character as UTF-8 encoded string
        FgString fg_character("\xC5\xA1");
        FGASSERT(character == fg_character[0]);
    }
    {
        FgTestDir   td("string_encoding");
        FgString    source("UTF-8:\xC5\xA1\xC4\x8E");
        {
            std::ofstream ofs("testString_utf8.txt");
            FGASSERT(ofs);
            ofs << source << '\n';
        }
        {
            std::ifstream ifs("testString_utf8.txt");
            FGASSERT(ifs);
            FgString target;
            ifs >> target;
            FGASSERT(source == target); }
    }
}

static void Replace()
{
    {   FgString original("ab");
        FGASSERT(FgString("aa") == original.replace('b','a'));
        FGASSERT(FgString("bb") == original.replace('a','b')); }
    {   FgString original("abcd");
        FGASSERT(FgString("aacd") == original.replace('b','a'));
        FGASSERT(FgString("abbd") == original.replace('c','b')); }
}

static void Compare()
{
    FgString a("a");
    FgString b("b");
    FGASSERT(a < b);
    FGASSERT(!(b < a));
}

static void Split()
{
    {   FgString a("a/b/c");
        FgStrings comps = a.split('/');
        FGASSERT(comps.size() == 3);
        FGASSERT(comps[0] == FgString("a"));
        FGASSERT(comps[1] == FgString("b"));
        FGASSERT(comps[2] == FgString("c")); }
    {   FgString a("noseparator");
        FgStrings comps = a.split(' ');
        FGASSERT(comps.size() == 1);
        FGASSERT(comps[0] == a); }
}

static void StartsWith()
{
    {
        FgString a("abcd");
        FGASSERT(a.beginsWith(FgString("a")));
        FGASSERT(a.beginsWith(FgString("ab")));
        FGASSERT(a.beginsWith(FgString("abc")));
        FGASSERT(a.beginsWith(FgString("abcd")));
        FGASSERT(!a.beginsWith(FgString("d")));
        FGASSERT(!a.beginsWith(FgString("db")));
        FGASSERT(!a.beginsWith(FgString("dbc")));
        FGASSERT(!a.beginsWith(FgString("dbcd")));
        FGASSERT(!a.beginsWith(FgString("abcde")));        
    }
    {
        FgString a("a kind of longish string to test with since small strings may hide some bugs");
        FGASSERT(a.beginsWith(FgString("a kind of longish string to test with")));
    }
}

static void Convert()
{
    string      utf8 = fgSlurp(fgDataDir()+"base/test/utf8_language_samples.txt"),
                reg32 = fgToUtf8(fgToUtf32(utf8));
    FGASSERT(utf8 == reg32);
#ifdef _WIN32
    string     reg16 = fgToUtf8(fgToUtf16(utf8));
    FGASSERT(utf8 == reg16);
#endif
}

void
fgStringTest(const FgArgs &)
{
    Construct();
    Copy();
    Assign();
    Append();
    Comparisons();
    Encoding();
    Replace();
    Compare();
    Split();
    StartsWith();
    Convert();
}
