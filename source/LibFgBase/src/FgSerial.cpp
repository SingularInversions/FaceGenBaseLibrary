//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgSerial.hpp"
#include "FgCommand.hpp"

using namespace std;

namespace Fg {

void
srlz_(bool v,String & s)
{
    uchar           b = v ? 1 : 0;
    srlz_(b,s);
}

void
dsrlz_(String const & s,size_t & p,bool & v)
{
    uchar           b;
    dsrlz_(s,p,b);
    v = (b == 1);
}

void
dsrlz_(String const & s,size_t & p,long & v)
{
    int64           t;
    dsrlzRaw_(s,p,t);
    FGASSERT(t >= std::numeric_limits<long>::lowest());
    FGASSERT(t <= std::numeric_limits<long>::max());
    v = static_cast<long>(t);
}
void
dsrlz_(String const & s,size_t & p,unsigned long & v)
{
    uint64          t;
    dsrlzRaw_(s,p,t);
    FGASSERT(t <= std::numeric_limits<unsigned long>::max());
    v = static_cast<unsigned long>(t);
}

void
srlz_(String const & v,String & s)
{
    srlz_(uint64(v.size()),s);
    s.append(v);
}
void
dsrlz_(String const & s,size_t & p,String & v)
{
    uint64              sz;
    dsrlz_(s,p,sz);
    FGASSERT(p+sz <= s.size());
    v.assign(s,p,size_t(sz));
    p += sz;
}

namespace {

void
test0()
{
    int                 i = 42;
    FGASSERT(i == dsrlz<int>(srlz(i)));
    uint                u = 42U;
    FGASSERT(u == dsrlz<uint>(srlz(u)));
    long                l = 42L;
    FGASSERT(l == dsrlz<long>(srlz(l)));
    long long           ll = 42LL;
    FGASSERT(ll == dsrlz<long long>(srlz(ll)));
    unsigned long long  ull = 42ULL;
    FGASSERT(ull == dsrlz<unsigned long long>(srlz(ull)));
    String              s = "Test String";
    FGASSERT(s == dsrlz<String>(srlz(s)));
}

void
test1()
{
    Strings         tns;
    int             a = 5;
    double          b = 3.14;
    typeNames_(tns,a,b);
    fgout << fgnl << tns;
}

}

struct  A
{
    int         i;
    float       f;
};

FG_SERIAL_2(A,i,f)

struct B
{
    A           a;
    double      d;
};

FG_SERIAL_2(B,a,d)

struct  Op
{
    double          acc {0};

    template<typename T>
    void operator()(T r) {acc += double(r); }
};

void traverseMembers_(Op & op,int s) {op(s); }
void traverseMembers_(Op & op,float s) {op(s); }
void traverseMembers_(Op & op,double s) {op(s); }

void
test2()
{
    Strings         names;
    reflectNames_<B>(names);
    fgout << fgnl << names;
    
    A               a {3,0.1f};
    B               b {a,2.7};
    Op              op;
    traverseMembers_(op,b);
    fgout << fgnl << "Acc: " << op.acc;
}

void
testSerial(CLArgs const &)
{
    test0();
    test1();
    test2();
    {
        Svec<string>        in {"first","second"},
                            out = dsrlz<Strings>(srlz(in));
        FGASSERT(in == out);
    }
    {
        String8                 dd = dataDir() + "base/test/";
        String                  msg = "This is a test",
                                ser = srlz(msg);
        //saveRaw(ser,dd+"serial32");
        //saveRaw(ser,dd+"serial64");
        String                  msg32 = dsrlz<String>(loadRaw(dd+"serial32")),
                                msg64 = dsrlz<String>(loadRaw(dd+"serial64"));
        FGASSERT(msg32 == msg);
        FGASSERT(msg64 == msg);
    }
}

}
