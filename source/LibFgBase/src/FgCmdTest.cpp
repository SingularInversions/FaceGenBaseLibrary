//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Test C++ behaviour
//

#include "stdafx.h"

#include "FgCommand.hpp"
#include "FgImage.hpp"
#include "FgTestUtils.hpp"
#include "FgBuild.hpp"
#include "FgTime.hpp"
#include "FgMath.hpp"
#include "FgDataflow.hpp"

using namespace std;

namespace Fg {

namespace {

struct      Rvo
{
    Doubles             data;                                   // something with construction worth eliding

    Rvo() {}
    explicit Rvo(double v) : data(1,v) {}
    explicit Rvo(Doubles const & vals) : data(vals) {}
    Rvo(Rvo const & rhs) : data(rhs.data) { fgout << "CC "; }   // show copy constructor invocations
    Rvo & operator=(Rvo const & rhs) = default;                 // must be explicit when CC is custom
};

void                testRvo(CLArgs const &)
{
    Rvo             t;
    {
        PushIndent      pind{"construct at single return: "};
        auto            fn = [](){return Rvo{3.14}; };
        fn();
    }
    {
        PushIndent      pind{"construct at multiple returns: "};
        auto            fn = [](bool r)
        {
            if (r)
                return Rvo{3.14};
            else
                return Rvo{2.72};
        };
        fn(true);
        fn(false);
    }
    {
        PushIndent      pind{"construct, modify, return: "};
        auto            fn = [](){
            Rvo             ret;
            ret.data.push_back(3.14);
            return ret;
        };
        fn();
    }
    {
        PushIndent      pind{"construct, mutiple modify paths, single return: "};
        auto            fn = [](bool r)
        {
            Rvo             ret;
            if (r)
                ret.data.push_back(3.14);
            else
                ret.data.push_back(2.72);
            return ret;
        };
        fn(true);
        fn(false);
    }
    {
        PushIndent      pind{"construct, mutiple returns: "};
        auto            fn = [](bool r)
        {
            Rvo             ret;
            if (r) {
                ret.data.push_back(3.14);
                return ret;
            }
            else {
                ret.data.push_back(2.72);
                return ret;
            }
        };
        fn(true);
        fn(false);
    }
    {
        // this works, but not sure how:
        PushIndent      pind{"assign to self from pass by reference: "};
        auto            fn = [](Rvo const & vals)
        {
            Rvo             ret;
            for (double v : vals.data)
                ret.data.push_back(2.72 * v);
            return ret;
        };
        Doubles             vals = cRandNormals(256);
        Rvo                 in {vals};
        // I guess the return value constructed first then input to operator=() here
        in = fn(in);
        FGASSERT(in.data == mapMul(2.72,vals));
    }
}

void                testExp(CLArgs const &)
{
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
        fgout << fgnl << "exp() time: " << 1000000.0 * tm.elapsedMilliseconds() / reps << " ns  (dummy val: " << acc << ")";
    }
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
        fgout << fgnl << "expFast() time: " << 1000000.0 * tm.elapsedMilliseconds() / reps << " ns  (dummy val: " << acc << ")";
    }
}

void                testAny(CLArgs const &)
{
    {   // test copy semantics:
        any             v0 = 42,
                        v1 = v0;
        int *           v0Ptr = any_cast<int>(&v0);
        *v0Ptr = 7;
        fgout << fgnl << "Original small value: " << *v0Ptr << " but copy remains at " << any_cast<int>(v1);
        // Now try with a heavy object that is not subject to small value optimization (16 bytes) onto the stack:
        v0 = Mat44D(42);
        v1 = v0;
        Mat44D *      v0_ptr = any_cast<Mat44D>(&v0);
        (*v0_ptr)[0] = 7;
        fgout << fgnl << "Original big value: " << (*v0_ptr)[0] << " but copy remains at " << any_cast<Mat44D>(v1)[0];
    }
    {   // error message for empty any_cast 
        any             vint;
        try {cout << any_cast<uint>(vint); }
        catch (std::exception const & e) {
            fgout << fgnl << "std::any_cast on empty: " << e.what();
        }
    }
    {   // error message for wrong type any_cast
        any             vint = 42;
        try {cout << any_cast<uint>(vint); }
        catch (std::exception const & e) {
            fgout << fgnl << "std::any_cast on differing types: " << e.what();
        }
    }
}

void                testStdHash(CLArgs const &)
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

template<typename T>
void                testmNLF(String type)
{
    fgout
        << fgnl << type << " min: " << numeric_limits<T>::min()
        << fgnl << type << " max: " << numeric_limits<T>::max()
        << fgnl << type << " epsilon: " << numeric_limits<T>::epsilon()
        << fgnl << type << " lowest: " << numeric_limits<T>::lowest();
}

void                testmNL(CLArgs const &)
{
    PushIndent          pind {"numeric limits"};
    testmNLF<float>("float");
    testmNLF<double>("double");
}

void                testArrSpeed(CLArgs const &)
{
    // Generate data and put in both parallel and packed arrays:
    size_t          N = 100000,
                    A = 8;
    Doubless         pins(A),
                    pouts(A);
    for (Doubles & pin : pins)
        pin = cRandNormals(N);
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
    size_t          time = tm.elapsedMilliseconds();
    double          val = 0;
    for (Doubles const & outs : pouts)
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
    time = tm.elapsedMilliseconds();
    val = cSum(souts);
    fgout << fgnl << "Packed array in, packed array out: " << time << "ms. (" << val << ")";

    // Packed -> Parallel
    // I have no idea why this is faster than packed->packed. I looked at the MSVC17 x64 O2
    // disassembler (on Goldbolt using just 4 vals and a single mult) and the multiply, add, store
    // operands were identical ...
    for (Doubles & pout : pouts)
        mapAssign_(pout,0.0);
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
    time = tm.elapsedMilliseconds();
    val = 0;
    for (Doubles const & outs : pouts)
        val += cSum(outs);
    fgout << fgnl << "Packed array in, paral arrays out: " << time << "ms. (" << val << ")";
}

void                testCppType(CLArgs const &)
{
    uchar constexpr         uc = 1;
    ushort constexpr        us = 1;
    uint constexpr          ui = 1;
    fgout
        << fgnl << "uchar * uchar = " << typeid(uc * uc).name()
        << fgnl <<  "ushort * ushort = " << typeid(us * us).name()
        << fgnl <<  "uint * uint = " << typeid(ui * ui).name()
        << fgnl << "uchar + uchar = " << typeid(uc + uc).name()
        << fgnl <<  "ushort + ushort = " << typeid(us + us).name()
        << fgnl <<  "uint + uint = " << typeid(ui + ui).name();
}

void                testCpp(CLArgs const & args)
{
    Cmds        cmds {
        {testAny,"any","Test std::any copy semantics"},
        {testArrSpeed,"array","Test speedup of switching from parallel to packed arrays"},
        {testExp,"exp","Measure the speed of library exp(double) and expFast"},
        {testStdHash,"hash","Test std::hash behaviour"},
        {testmNL,"num","view numeric limits"},
        {testRvo,"rvo","Return value optimization / copy elision"},
        {testCppType,"type","fundamental types output by operators as a function of input types"},
    };
    doMenu(args,cmds,true);
}

}

void                testDataflow(CLArgs const &);
void                testFilesystem(CLArgs const &);
void                testOpenFile(CLArgs const &);
void                testGeometry(CLArgs const &);
void                testGridTriangles(CLArgs const &);
void                testGui(CLArgs const & args);
void                testHash(CLArgs const &);
void                testImage(CLArgs const &);
void                testKdTree(CLArgs const &);
void                testMatrixSolver(CLArgs const &);
void                testMath(CLArgs const &);
void                testMatrixC(CLArgs const &);
void                testMatrixV(CLArgs const &);
void                testMetaFormat(CLArgs const &);
void                testMesh(CLArgs const &);
void                testMorph(CLArgs const &);
void                testParse(CLArgs const &);
void                testPath(CLArgs const &);
void                testQuaternion(CLArgs const &);
void                testRandom(CLArgs const &);
void                testRender(CLArgs const &);
void                testRenderCmd(CLArgs const &);
void                testSerial(CLArgs const &);
void                testSimilarity(CLArgs const &);
void                testSurf(CLArgs const &);
void                testTopo(CLArgs const &);
void                testString(CLArgs const &);

void                testBase(CLArgs const & args)
{
    Cmds            cmds {
        {testCpp,"cpp","C++ behaviour tests"},
        {testDataflow,"dataflow"},
        {testFilesystem,"filesystem"},
        {testOpenFile,"open"},
        {testGeometry,"geometry"},
        {testGui,"gui"},
        {testGridTriangles,"grid","gridTriangles grid index"},
        {testHash,"hash"},
        {testImage,"image"},
        {testKdTree,"kd"},
        {testMatrixSolver,"matSol","Matrix Solver"},
        {testMath,"math"},
        {testMatrixC,"matC","MatrixC"},
        {testMatrixV,"matV","MatrixV"},
        {testMetaFormat,"meta","meta format for serialized data"},
        {testMesh,"mesh","3D mesh related functionality"},
        {testMorph,"morph"},
        {testParse,"parse"},
        {testPath,"path"},
        {testQuaternion,"quat","quaterion"},
        {testRandom,"random"},
        {testRender,"rend","sofware rendering"},
        {testRenderCmd,"rendc","render command"},
        {testSerial,"serial"},
        {testSimilarity,"sim","similarity transform and solver"},
        {testSurf,"surf","operations on polygonal surfaces"},
        {testTopo,"topo","surface topology analysis"},
        {testString,"string"},
    };
    doMenu(args,cmds,true);
}

}

// */
