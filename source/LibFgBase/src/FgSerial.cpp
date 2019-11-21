//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgSerial.hpp"
#include "MurmurHash2.h"
#include "FgCommand.hpp"

using namespace std;

namespace Fg {

uint64
fgHash(uint64 k0,uint64 k1)
{
    uint64  h[2];
    h[0] = k0;
    h[1] = k1;
    // MurmurHas64A yields the same result on 32/64 bit compiles (with same seed):
    return MurmurHash64A(h,16,0x18D75B7621B4434DULL);
}

uint64
fgHash(uint64 k0,uint64 k1,uint64 k2)
{
    uint64  h[3];
    h[0] = k0;
    h[1] = k1;
    h[2] = k2;
    return MurmurHash64A(h,24,0x18D75B7621B4434DULL);
}

void
fgDsr(const char * & ptr,const char * end,long & val)
{
    FGASSERT(end-ptr >= int64(sizeof(int64)));
    int64           tmp;
    fgDsrBuiltin(ptr,end,tmp);
    FGASSERT((tmp <= std::numeric_limits<long>::max()) && (tmp >= std::numeric_limits<long>::min()));
    val = long(tmp);
}

void
fgDsr(const char * & ptr,const char * end,unsigned long & val)
{
    FGASSERT(end-ptr >= int64(sizeof(uint64)));
    uint64          tmp;
    fgDsrBuiltin(ptr,end,tmp);
    FGASSERT(tmp <= std::numeric_limits<unsigned long>::max());
    val = long(tmp);
}

// TEST

struct  FgSerTest
{
    char        c;
    int         i;
};

template<>
uint64
fgSerSig<FgSerTest>()
{
    return fgHash(fgSerSig<decltype(FgSerTest::c)>(),fgSerSig<decltype(FgSerTest::i)>());
}

void
fgSerTest(CLArgs const &)
{
    uint64  sig = fgSerSig<FgSerTest>();
    fgout << fgnl << sig;
}

}
