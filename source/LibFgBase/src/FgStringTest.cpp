//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//


#include "stdafx.h"

#include "FgTestUtils.hpp"
#include "FgString.hpp"
#include "FgMain.hpp"

using namespace std;

namespace Fg {

static void Construct()
{
    {   Ustring s;
        FGASSERT(s.size() == 0); }
    {   Ustring s("12345");
        FGASSERT(s.size() == 5);}
#ifdef _WIN32
    {   Ustring s(L"12345");
        FGASSERT(s.size() == 5); }
#endif
    {   Ustring s1("12345");
        Ustring s2(s1);
        FGASSERT(s1.size() == s2.size() && s1.size() == 5); }
}

static void Copy()
{
    {
        // Ensure that we are copying on copy
        Ustring original("abcd");
        Ustring copy(original);
        FGASSERT(copy == original);
        copy += "efghi";
        FGASSERT(copy != original);
    }
}

static void Assign()
{
    {   Ustring s;
        s = "12345";
        FGASSERT(s.size() == 5); }
#ifdef _WIN32
    {   Ustring s;
        s = L"12345";
        FGASSERT(s.size() == 5); }
    {   Ustring s;
        s = L"12345";
        Ustring s1;
        s1 = s;
        FGASSERT(s1.size() == 5); }
#endif
}

static void Append()
{
    {   Ustring s("12345");
        s += "12345";
        FGASSERT(s.size() == 10); }
#ifdef _WIN32
    {   Ustring s("12345");
        s += L"12345";
        FGASSERT(s.size() == 10); }
#endif
    {   Ustring s("12345");
        s += s;
        FGASSERT(s.size() == 10); }
    {   Ustring s("abcd");
        FGASSERT(s=="abcd");
        s+="efg";
        FGASSERT(s!="abcd");
        FGASSERT(s=="abcdefg"); }
}

static void Comparisons()
{
#ifdef _WIN32
    {   Ustring s("12345");
        FGASSERT(s == "12345");
        FGASSERT(s == L"12345");
        FGASSERT(s == Ustring("12345"));
        FGASSERT(Ustring("12345") == s); }
    {   Ustring s("12345");
        FGASSERT(s != "abcdefg");
        FGASSERT(s != L"Hi");
        FGASSERT(s != Ustring("Bye"));
        FGASSERT(s == "12345"); }
#endif
    {   Ustring s("abcdefg");
        FGASSERT(s == "abcdefg");
        FGASSERT(s != "abc"); }
}

static void Encoding()
{
    {
        // Character as Unicode code point
        uint32  character = 0x0161;
        // Same character as UTF-8 encoded string
        Ustring fg_character("\xC5\xA1");
        FGASSERT(character == fg_character[0]);
    }
    {
        TestDir   td("string_encoding");
        Ustring    source("UTF-8:\xC5\xA1\xC4\x8E");
        {
            std::ofstream ofs("testString_utf8.txt");
            FGASSERT(ofs);
            ofs << source << '\n';
        }
        {
            std::ifstream ifs("testString_utf8.txt");
            FGASSERT(ifs);
            Ustring target;
            ifs >> target;
            FGASSERT(source == target); }
    }
}

static void Replace()
{
    {   Ustring original("ab");
        FGASSERT(Ustring("aa") == original.replace('b','a'));
        FGASSERT(Ustring("bb") == original.replace('a','b')); }
    {   Ustring original("abcd");
        FGASSERT(Ustring("aacd") == original.replace('b','a'));
        FGASSERT(Ustring("abbd") == original.replace('c','b')); }
}

static void Compare()
{
    Ustring a("a");
    Ustring b("b");
    FGASSERT(a < b);
    FGASSERT(!(b < a));
}

static void Split()
{
    {   Ustring a("a/b/c");
        Ustrings comps = a.split('/');
        FGASSERT(comps.size() == 3);
        FGASSERT(comps[0] == Ustring("a"));
        FGASSERT(comps[1] == Ustring("b"));
        FGASSERT(comps[2] == Ustring("c")); }
    {   Ustring a("noseparator");
        Ustrings comps = a.split(' ');
        FGASSERT(comps.size() == 1);
        FGASSERT(comps[0] == a); }
}

static void StartsWith()
{
    {
        Ustring a("abcd");
        FGASSERT(a.beginsWith(Ustring("a")));
        FGASSERT(a.beginsWith(Ustring("ab")));
        FGASSERT(a.beginsWith(Ustring("abc")));
        FGASSERT(a.beginsWith(Ustring("abcd")));
        FGASSERT(!a.beginsWith(Ustring("d")));
        FGASSERT(!a.beginsWith(Ustring("db")));
        FGASSERT(!a.beginsWith(Ustring("dbc")));
        FGASSERT(!a.beginsWith(Ustring("dbcd")));
        FGASSERT(!a.beginsWith(Ustring("abcde")));        
    }
    {
        Ustring a("a kind of longish string to test with since small strings may hide some bugs");
        FGASSERT(a.beginsWith(Ustring("a kind of longish string to test with")));
    }
}

static void Convert()
{
    string      utf8 = fgSlurp(dataDir()+"base/test/utf8_language_samples.txt"),
                reg32 = fgToUtf8(fgToUtf32(utf8));
    FGASSERT(utf8 == reg32);
#ifdef _WIN32
    string     reg16 = fgToUtf8(fgToUtf16(utf8));
    FGASSERT(utf8 == reg16);
#endif
}

void
fgStringTest(CLArgs const &)
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

}
