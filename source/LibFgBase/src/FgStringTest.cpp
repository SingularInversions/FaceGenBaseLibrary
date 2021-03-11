//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
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
    {   String8 s;
        FGASSERT(s.size() == 0); }
    {   String8 s("12345");
        FGASSERT(s.size() == 5);}
#ifdef _WIN32
    {   String8 s(L"12345");
        FGASSERT(s.size() == 5); }
#endif
    {   String8 s1("12345");
        String8 s2(s1);
        FGASSERT(s1.size() == s2.size() && s1.size() == 5); }
}

static void Copy()
{
    {
        // Ensure that we are copying on copy
        String8 original("abcd");
        String8 copy(original);
        FGASSERT(copy == original);
        copy += "efghi";
        FGASSERT(copy != original);
    }
}

static void Assign()
{
    {   String8 s;
        s = "12345";
        FGASSERT(s.size() == 5); }
#ifdef _WIN32
    {   String8 s;
        s = L"12345";
        FGASSERT(s.size() == 5); }
    {   String8 s;
        s = L"12345";
        String8 s1;
        s1 = s;
        FGASSERT(s1.size() == 5); }
#endif
}

static void Append()
{
    {   String8 s("12345");
        s += "12345";
        FGASSERT(s.size() == 10); }
#ifdef _WIN32
    {   String8 s("12345");
        s += L"12345";
        FGASSERT(s.size() == 10); }
#endif
    {   String8 s("12345");
        s += s;
        FGASSERT(s.size() == 10); }
    {   String8 s("abcd");
        FGASSERT(s=="abcd");
        s+="efg";
        FGASSERT(s!="abcd");
        FGASSERT(s=="abcdefg"); }
}

static void Comparisons()
{
#ifdef _WIN32
    {   String8 s("12345");
        FGASSERT(s == "12345");
        FGASSERT(s == L"12345");
        FGASSERT(s == String8("12345"));
        FGASSERT(String8("12345") == s); }
    {   String8 s("12345");
        FGASSERT(s != "abcdefg");
        FGASSERT(s != L"Hi");
        FGASSERT(s != String8("Bye"));
        FGASSERT(s == "12345"); }
#endif
    {   String8 s("abcdefg");
        FGASSERT(s == "abcdefg");
        FGASSERT(s != "abc"); }
}

static void Encoding()
{
    {
        // Character as Unicode code point
        uint32          ch32 = 0x0161;
        // Same character as UTF-8 encoded string
        char            ch8[] = "\xC5\xA1";
        String8         str {ch8};
        FGASSERT(ch32 == str[0]);
        String32        s32 = toUtf32(ch8);
        FGASSERT(str == toUtf8(s32));
    }
    {
        String          s8 {"hello"};
        String32        s32 = toUtf32("hello");
        FGASSERT(toUtf8(s32) == s8);
        FGASSERT(s32 == toUtf32(s8));
    }
    {
        TestDir   td("string_encoding");
        String8    source("UTF-8:\xC5\xA1\xC4\x8E");
        {
            std::ofstream ofs("testString_utf8.txt");
            FGASSERT(ofs);
            ofs << source << '\n';
        }
        {
            std::ifstream ifs("testString_utf8.txt");
            FGASSERT(ifs);
            String8 target;
            ifs >> target;
            FGASSERT(source == target); }
    }
}

static void Replace()
{
    {   String8 original("ab");
        FGASSERT(String8("aa") == original.replace('b','a'));
        FGASSERT(String8("bb") == original.replace('a','b')); }
    {   String8 original("abcd");
        FGASSERT(String8("aacd") == original.replace('b','a'));
        FGASSERT(String8("abbd") == original.replace('c','b')); }
}

static void Compare()
{
    String8 a("a");
    String8 b("b");
    FGASSERT(a < b);
    FGASSERT(!(b < a));
}

static void Split()
{
    {   String8 a("a/b/c");
        String8s comps = a.split('/');
        FGASSERT(comps.size() == 3);
        FGASSERT(comps[0] == String8("a"));
        FGASSERT(comps[1] == String8("b"));
        FGASSERT(comps[2] == String8("c")); }
    {   String8 a("noseparator");
        String8s comps = a.split(' ');
        FGASSERT(comps.size() == 1);
        FGASSERT(comps[0] == a); }
}

static void StartsWith()
{
    {
        String8 a("abcd");
        FGASSERT(a.beginsWith(String8("a")));
        FGASSERT(a.beginsWith(String8("ab")));
        FGASSERT(a.beginsWith(String8("abc")));
        FGASSERT(a.beginsWith(String8("abcd")));
        FGASSERT(!a.beginsWith(String8("d")));
        FGASSERT(!a.beginsWith(String8("db")));
        FGASSERT(!a.beginsWith(String8("dbc")));
        FGASSERT(!a.beginsWith(String8("dbcd")));
        FGASSERT(!a.beginsWith(String8("abcde")));        
    }
    {
        String8 a("a kind of longish string to test with since small strings may hide some bugs");
        FGASSERT(a.beginsWith(String8("a kind of longish string to test with")));
    }
}

static void Convert()
{
    string      utf8 = loadRawString(dataDir()+"base/test/utf8_language_samples.txt"),
                reg32 = toUtf8(toUtf32(utf8));
    FGASSERT(utf8 == reg32);
#ifdef _WIN32
    string     reg16 = toUtf8(toUtf16(utf8));
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
