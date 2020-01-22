//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Test C++ behaviour
//

#include "stdafx.h"

#include <boost/any.hpp>

#include "FgCmd.hpp"
#include "FgSyntax.hpp"
#include "FgMetaFormat.hpp"
#include "FgImage.hpp"
#include "FgTestUtils.hpp"
#include "FgBuild.hpp"
#include "FgVersion.hpp"
#include "FgTime.hpp"

using namespace std;

namespace Fg {

static size_t rvoCount;

struct  FgTestmRvo
{
    Doubles      data;
    FgTestmRvo() {}
    FgTestmRvo(double v) : data(1,v) {}
    FgTestmRvo(const FgTestmRvo & rhs) : data(rhs.data) {++rvoCount; }
};

FgTestmRvo
fgTestmRvo()
{
    Doubles      t0(1,3.14159);
    return FgTestmRvo(t0[0]);
}

FgTestmRvo
fgTestmRvoMr(bool sel)
{
    Doubles      t0(1,2.71828),
                t1(1,3.14159);
    if (sel)
        return FgTestmRvo(t0[0]);
    else
        return FgTestmRvo(t1[0]);
}

FgTestmRvo
fgTestmNrvo(bool sel)
{
    Doubles      t0(1,2.71828),
                t1(1,3.14159);
    FgTestmRvo  ret;
    if (sel)
        ret = FgTestmRvo(t0[0]);
    else
        ret = FgTestmRvo(t1[0]);
    return ret;
}

FgTestmRvo
fgTestmNrvoMr(bool sel)
{
    Doubles      t0(1,2.71828),
                t1(1,3.14159);
    FgTestmRvo  ret;
    if (sel) {
        ret = FgTestmRvo(t0[0]);
        return ret;
    }
    else {
        ret = FgTestmRvo(t1[0]);
        return ret;
    }
}

static
void
rvo(CLArgs const &)
{
    FgTestmRvo      t;
    rvoCount = 0;
    t = fgTestmRvo();
    fgout << fgnl << "RVO copies: " << rvoCount;
    rvoCount = 0;
    t = fgTestmRvoMr(true);
    t = fgTestmRvoMr(false);
    fgout << fgnl << "RVO with multiple returns (both paths) copies: " << rvoCount;
    rvoCount = 0;
    t = fgTestmNrvo(true);
    t = fgTestmNrvo(false);
    fgout << fgnl << "NRVO copies (both assignment paths): " << rvoCount;
    rvoCount = 0;
    t = fgTestmNrvoMr(true);
    t = fgTestmNrvoMr(false);
    fgout << fgnl << "NRVO with multiple returns (both paths) copies: " << rvoCount;
}

static
void
speedExp(CLArgs const &)
{
    double      val = 0,
                inc = 2.718281828,
                mod = 3.141592653,
                acc = 0;
    size_t      reps = 10000000;
    Timer     tm;
    for (size_t ii=0; ii<reps; ++ii) {
        acc += exp(-val);
        val += inc;
        if (val > mod)
            val -= mod;
    }
    fgout << fgnl << "exp() time: " << 1000000.0 * tm.readMs() / reps << " ns  (dummy val: " << acc << ")";
}

static
void
fgexp(CLArgs const &)
{
    double      maxRel = 0,
                totRel = 0;
    size_t      cnt = 0;
    for (double dd=0; dd<5; dd+=0.001) {
        double  baseline = exp(-dd),
                test = expFast(-dd),
                meanVal = (test+baseline) * 0.5,
                relDel = (test-baseline) / meanVal;
        maxRel = cMax(maxRel,relDel);
        totRel += relDel;
        ++cnt;
    }
    fgout << "Max Rel Del: " << maxRel << " mean rel del: " << totRel / cnt;
    double      val = 0,
                inc = 2.718281828,
                mod = 3.141592653,
                acc = 0;
    size_t      reps = 10000000;
    Timer     tm;
    for (size_t ii=0; ii<reps; ++ii) {
        acc += expFast(-val);
        val += inc;
        if (val > mod)
            val -= mod;
    }
    fgout << fgnl << "exp() time: " << 1000000.0 * tm.readMs() / reps << " ns  (dummy val: " << acc << ")";
}

static
void
any(CLArgs const &)
{
    boost::any      v0 = 42,
                    v1 = v0;
    int *           v0Ptr = boost::any_cast<int>(&v0);
    *v0Ptr = 7;
    fgout << fgnl << "Original small value: " << *v0Ptr << " but copy remains at " << boost::any_cast<int>(v1);
    // Now try with a heavy object that is not subject to small value optimization (16 bytes) onto the stack:
    v0 = Mat44D(42);
    v1 = v0;
    Mat44D *      v0_ptr = boost::any_cast<Mat44D>(&v0);
    (*v0_ptr)[0] = 7;
    fgout << fgnl << "Original big value: " << (*v0_ptr)[0] << " but copy remains at " << boost::any_cast<Mat44D>(v1)[0];
}

static
void
thash(CLArgs const &)
{
    for (uint ii=0; ii<3; ++ii) {
        string              str = "Does this string give a consistent hash on subsequent runs ?";
        std::hash<string>   hfn;        // Returns size_t
#ifdef FG_64                            // size_t is 64 bits:
        fgout << fgnl << "64 bit hash: " << hfn(str);
#else                                   // size_t is 32 bits:
        size_t              hval32 = hfn(str);
        uint64              lo = hval32,
                            hi = hfn(str+toStr(lo)),
                            hval64 = lo | (hi << 32);
        fgout << fgnl << "32 bit hash: " << hval32 << " and 64 composite: " << hval64;
#endif
    }
}

static
void
parr(CLArgs const &)
{
    // Generate data and put in both parallel and packed arrays:
    size_t          N = 100000,
                    A = 8;
    Doubless         pins(A),
                    pouts(A);
    for (Doubles & pin : pins)
        pin = generate<double>(randNormal,N);
    for (Doubles & pout : pouts)
        pout.resize(N,0);
    Doubles          sins,
                    souts(N*A,0.0);
    for (size_t ii=0; ii<N; ++ii)
        for (size_t jj=0; jj<A; ++jj)
            sins.push_back(pins[jj][ii]);

    // Parallel -> Parallel
    Timer         tm;
    for (size_t rr=0; rr<100; ++rr) {
        for (size_t ii=0; ii<N; ++ii) {
            pouts[0][ii] += pins[0][ii]*pins[1][ii] + pins[2][ii]*pins[3][ii];
            pouts[1][ii] += pins[1][ii]*pins[2][ii] + pins[3][ii]*pins[4][ii];
            pouts[2][ii] += pins[2][ii]*pins[3][ii] + pins[4][ii]*pins[5][ii];
            pouts[3][ii] += pins[3][ii]*pins[4][ii] + pins[5][ii]*pins[6][ii];
            pouts[4][ii] += pins[4][ii]*pins[5][ii] + pins[6][ii]*pins[7][ii];
            pouts[5][ii] += pins[5][ii]*pins[6][ii] + pins[7][ii]*pins[0][ii];
            pouts[6][ii] += pins[6][ii]*pins[7][ii] + pins[0][ii]*pins[1][ii];
            pouts[7][ii] += pins[7][ii]*pins[0][ii] + pins[1][ii]*pins[2][ii];
        }
    }
    size_t          time = tm.readMs();
    double          val = 0;
    for (const Doubles & outs : pouts)
        val += cSum(outs);
    fgout << fgnl << "Paral arrays in, paral arrays out: " << time << "ms. (" << val << ")";

    // Packed -> Packed
    tm.start();
    for (size_t rr=0; rr<100; ++rr) {
        for (size_t ii=0; ii<N; ++ii) {
            // Inlining this value in every use made no speed difference; the compiler
            // takes care of this automatically. We only leave it for clarity:
            size_t          idx = ii*8;
            souts[idx+0] += sins[idx+0]*sins[idx+1] + sins[idx+2]*sins[idx+3];
            souts[idx+1] += sins[idx+1]*sins[idx+2] + sins[idx+3]*sins[idx+4];
            souts[idx+2] += sins[idx+2]*sins[idx+3] + sins[idx+4]*sins[idx+5];
            souts[idx+3] += sins[idx+3]*sins[idx+4] + sins[idx+5]*sins[idx+6];
            souts[idx+4] += sins[idx+4]*sins[idx+5] + sins[idx+6]*sins[idx+7];
            souts[idx+5] += sins[idx+5]*sins[idx+6] + sins[idx+7]*sins[idx+0];
            souts[idx+6] += sins[idx+6]*sins[idx+7] + sins[idx+0]*sins[idx+1];
            souts[idx+7] += sins[idx+7]*sins[idx+0] + sins[idx+1]*sins[idx+2];
        }
    }
    time = tm.readMs();
    val = cSum(souts);
    fgout << fgnl << "Packed array in, packed array out: " << time << "ms. (" << val << ")";

    // Packed -> Parallel
    // I have no idea why this is faster than packed->packed. I looked at the MSVC17 x64 O2
    // disassembler (on Goldbolt using just 4 vals and a single mult) and the multiply, add, store
    // operands were identical ...
    for (Doubles & pout : pouts)
        fgFill(pout,0.0);
    tm.start();
    for (size_t rr=0; rr<100; ++rr) {
        for (size_t ii=0; ii<N; ++ii) {
            // Inlining this value in every use made no speed difference; the compiler
            // takes care of this automatically. We only leave it for clarity:
            size_t          idx = ii*8;
            pouts[0][ii] += sins[idx+0]*sins[idx+1] + sins[idx+2]*sins[idx+3];
            pouts[1][ii] += sins[idx+1]*sins[idx+2] + sins[idx+3]*sins[idx+4];
            pouts[2][ii] += sins[idx+2]*sins[idx+3] + sins[idx+4]*sins[idx+5];
            pouts[3][ii] += sins[idx+3]*sins[idx+4] + sins[idx+5]*sins[idx+6];
            pouts[4][ii] += sins[idx+4]*sins[idx+5] + sins[idx+6]*sins[idx+7];
            pouts[5][ii] += sins[idx+5]*sins[idx+6] + sins[idx+7]*sins[idx+0];
            pouts[6][ii] += sins[idx+6]*sins[idx+7] + sins[idx+0]*sins[idx+1];
            pouts[7][ii] += sins[idx+7]*sins[idx+0] + sins[idx+1]*sins[idx+2];
        }
    }
    time = tm.readMs();
    val = 0;
    for (const Doubles & outs : pouts)
        val += cSum(outs);
    fgout << fgnl << "Packed array in, paral arrays out: " << time << "ms. (" << val << ")";



}

void
fgCmdTestmCpp(CLArgs const & args)
{
    Cmds   cmds;
    cmds.push_back(Cmd(any,"any","Test boost any copy semantics"));
    cmds.push_back(Cmd(fgexp,"fgexp","Test and mesaure speed of interal optimized exp"));
    cmds.push_back(Cmd(thash,"hash","Test std::hash behaviour"));
    cmds.push_back(Cmd(parr,"parr","Test speedup of switching from parallel to packed arrays"));
    cmds.push_back(Cmd(rvo,"rvo","Return value optimization / copy elision"));
    cmds.push_back(Cmd(speedExp,"exp","Measure the speed of library exp(double)"));
    doMenu(args,cmds);
}

}

// */
