//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty, Sohail Somani
// Created:     March 29, 2008
//

#include "stdafx.h"

#include "FgDepGraph.hpp"
#include "FgTime.hpp"
#include "FgThread.hpp"
#include "FgTestUtils.hpp"
#include "FgTempFile.hpp"
#include "FgMetaFormat.hpp"
#include "FgTempFile.hpp"
#include "FgStdVector.hpp"
#include "FgCommand.hpp"

using namespace std;

static uint             m_count(0);
static boost::mutex     m_mutex;

static
FGLINK(calcAddInts)
{
    FGASSERT(inputs.size() > 0);
    FGASSERT(outputs.size() > 0);
    int     sum = 0;
    for (size_t ii=0; ii<inputs.size(); ii++)
    {
        int tmp = inputs[ii]->getCRef<int>();
        sum += tmp;
    }
    for (size_t ii=0; ii<outputs.size(); ++ii)
        *outputs[ii] = sum;
    {
        m_mutex.lock();
        ++m_count;
        m_mutex.unlock();
    }
}

static void
testDepGraphSimple()
{
    FgDepGraph  dg;
    uint        idxA = dg.addNode(2,"A"),
                idxB = dg.addNode(3,"B"),
                idxC = dg.addNode(0,"C");
    dg.addLink(calcAddInts,fgSvec(idxA,idxB),fgSvec(idxC));

    // Append a source with no link dependency:
    uint        idxF = dg.addNode(4,"F");
    dg.appendSource(idxF,idxC);

    uint        idxD = dg.addNode(7,"D"),
                idxE = dg.addNode(0,"E");
    dg.addLink(calcAddInts,fgSvec(idxD),fgSvec(idxE));
    dg.appendSource(idxC,idxE);

    int     res2 = dg.valueCRef(idxB),
            res3 = dg.valueCRef(idxC),
            res5 = dg.valueCRef(idxE);
    FGASSERT(res2 == 3);
    FGASSERT(res3 == 9);
    FGASSERT(res5 == 16);
    FGASSERT(m_count == 2);

    int     intB = dg.getNode(idxB).getCRef<int>();
    FGASSERT(intB == 3);
    dg.setNodeVal(idxB,10);
    res5 = dg.getNode(idxE).getCRef<int>();
    FGASSERT(res5 == 23);

    const int & cint = dg.getNode(idxB).valueRef();
    FGASSERT(cint == 10);

    // Test appending a source to a clean graph:
    uint    idxG = dg.addNode(9,"G");
    dg.appendSource(idxG,idxC);
    res5 = dg.getNode(idxE).getCRef<int>();
    FGASSERT(res5 == 32);

    // Test adding a link to a clean graph, outputting to a clean node:
    dg.addLink(calcAddInts,fgSvec(idxG),fgSvec(idxD));
    res5 = dg.getNode(idxE).getCRef<int>();
    FGASSERT(res5 == 34);

    // Test changing a node which is an input to >1 link:
    dg.setNodeVal(idxG,10);
    res5 = dg.getNode(idxE).getCRef<int>();
    FGASSERT(res5 == 36);

    // Test adding a link with 2 outputs:
    uint    idxH = dg.addNode(int(),"H"),
            idxK = dg.addNode(int(),"K");
    dg.addLink(calcAddInts,fgSvec(idxC,idxD),fgSvec(idxH,idxK));
    int     res6 = dg.getNode(idxH).getCRef<int>();
    FGASSERT(res6 == res5);

    // Test updating a node with multiple dependency paths to some sources:
    FgDgn<int>  idxJ = dg.addNode(int(),"J");
    dg.addLink(calcAddInts,fgSvec(idxE,idxH,idxK),fgSvec(idxJ.idx()));
    dg.setNodeVal(idxG,10);
    uint    baseCount = m_count;
    dg.nodeVal(idxJ);
    FGASSERT(m_count - baseCount == 5);

    if (FgTempFile::getKeepTempFiles())
        fgDepGraph2Pdf(dg.linkGraph(),"testCalcs");
}

static uint m_num_concats(0);

static FGLINK(concatenate)
{
    std::string result;
    FGASSERT(inputs.size() >= 1);
    for(size_t ii = 0; ii < inputs.size()-1; ++ii)
        result += inputs[ii]->getCRef<std::string>() + " ";
    result += inputs.back()->getCRef<std::string>();
    FGASSERT(outputs.size() == 1);
    *outputs[0] = result;
    {
        m_mutex.lock();
        ++m_num_concats;
        m_mutex.unlock();
    }
}

/// Tests that when only a subset is required to be
/// calculated, then only that subgraph is updated.
static void
testDepGraphSubset()
{
    FgDepGraph               m_graph;
    vector<uint>    sources,sinks;
    uint            str1 = m_graph.addNode<std::string>("how","str1"),
                    str2 = m_graph.addNode<std::string>("are","str2"),
                    con1 = m_graph.addNode<std::string>("","concat1");
    m_graph.addLink(concatenate,fgSvec(str1,str2),fgSvec(con1));

    uint            str3 = m_graph.addNode<std::string>("Hello,","str3"),
                    con2 = m_graph.addNode<std::string>("","concat2");
    m_graph.addLink(concatenate,fgSvec(str3,con1),fgSvec(con2));

    uint            str4 = m_graph.addNode<std::string>("you?","str4"),
                    con3 = m_graph.addNode<std::string>("","concat3");
    m_graph.addLink(concatenate,fgSvec(con1,str4),fgSvec(con3));

    if (FgTempFile::getKeepTempFiles())
        fgDepGraph2Pdf(m_graph.linkGraph(),"testSubset.pdf");

    const std::string & concat2 = m_graph.getNode(con2).valueRef();
    fgout << fgnl << "|" << concat2 << "|";
    FGASSERT(concat2 == "Hello, how are");
    FGASSERT(m_num_concats == 2);

    const std::string & concat3 = m_graph.getNode(con3).valueRef();
    fgout << fgnl << "|" << concat3 << "|";
    FGASSERT(concat3 == "how are you?");
    FGASSERT(m_num_concats == 3);
}

static FGLINK(add)
{
    std::size_t result = 0;
    for(std::size_t ss = 0 ; ss < inputs.size(); ++ss)
        result += inputs[ss]->getCRef<std::size_t>();
    FGASSERT(outputs.size() == 1);
    *outputs[0] = result;
}

static std::size_t
s_fib(std::size_t n)
{
    if(n <= 2) return 1;
    else return s_fib(n-1) + s_fib(n-2);
}

static FGLINK(fib)
{
    FGASSERT(inputs.size() == 1);
    FGASSERT(outputs.size() == 1);
    *outputs[0] = s_fib(inputs[0]->getCRef<std::size_t>());
}

static void
testCalcMulti(uint threads)
{
    FGASSERT(threads > 0);
    uint m_num_threads = threads;
    FgDepGraph m_graph(threads);
    FgTimer timer;
    timer.start();

        // This ensures that we always do atleast 2 fib(40)
        // computations.
    const uint N = m_num_threads <= 1 ? 2 : m_num_threads;

    uint    idxn = m_graph.addNode<std::size_t>(40,"n"),
            idxF = m_graph.addNode<std::size_t>(0,"F(n)");
    m_graph.addLink(fib,fgSvec(idxn),fgSvec(idxF));

    vector<uint>    sources(1,idxF);
    for(uint tt = 1; tt < N; ++tt)
    {
        uint    idxFt = m_graph.addNode<std::size_t>(0,"F(n)");
        sources.push_back(idxFt);
        m_graph.addLink(fib,fgSvec(idxn),fgSvec(idxFt));
    }

    uint        idxNF = m_graph.addNode<std::size_t>(0,"N*F(n)");
    m_graph.addLink(add,sources,fgSvec(idxNF));

    std::size_t sum = m_graph.valueCRef(idxNF);
    fgout << fgnl << "N*F(n) = " << sum;
    double secs = timer.read();
    fgout << fgnl << "Test took " << secs << " seconds ";
    FGASSERT(sum == N*s_fib(40));
}

static void
testDepGraphMulti(uint num_threads)
{
    fgout << fgnl << "Running multi-threaded test with " << num_threads << " thread(s).";
    fgout.push();
    testCalcMulti(num_threads);
    fgout.pop();
}

static void
testDepGraphMulti()
{
    testDepGraphMulti(1);
    testDepGraphMulti(2);
    uint    hthreads = boost::thread::hardware_concurrency(),
            hcores = boost::thread::physical_concurrency();
    fgout << fgnl << "Hardware threads: " << hthreads << " cores: " << hcores;
    if(hthreads > 2)
        testDepGraphMulti(hthreads);
}

static void
throw_exception(
    const std::vector<const FgVariant*> &,
    const std::vector<FgVariant*> &)
{
    fgThrow("An exception");
}

static void
testCalcExceptions()
{
    FgDepGraph m_graph;
    uint    idxn = m_graph.addNode<std::size_t>(40,"n"),
            idxt = m_graph.addNode<std::size_t>(0,"throw_exception(n)");
    m_graph.addLink(throw_exception,fgSvec(idxn),fgSvec(idxt));
    FG_TEST_CHECK_THROW_1(m_graph.getNode(idxt),
                          "A computation within an FgDepGraph has generated an exception on link");
}

static void
testDepGraphExceptions()
{
    fgout << fgnl << "Running single-threaded exceptions test";
    fgout.push();
    testCalcExceptions();
    fgout.pop();
}

static void
testDepGraphCopyable()
{
    FgDepGraph dg1,dg2;
    dg1 = dg2;

    FgDepGraph dg3(dg1);
}

static boost::barrier   m_barrier(2);

static void
doit(const std::vector<const FgVariant*> & /*sources*/,
     const std::vector<FgVariant*> & /*sinks*/)
{
        // First wait for both threads to reach so we can force
        // one exception to be thrown first. Note that if you get
        // a deadlock here, something is seriously screwed :-)
    m_barrier.wait();
    fgThrow("Thrown from multiple threads at the same time!");
}

static void
testDepGraphExceptionsMulti()
{
    FgTestDir   td("depGraphExceptionsMulti");
    fgout << fgnl << "Running exceptions from multiple threads test.";
    FgDepGraph      m_graph(2);
    uint            t1 = m_graph.addNode<std::size_t>(0,"t1"),
                    t1t1 = m_graph.addNode<std::size_t>(0,"t1(t1)");
    m_graph.addLink(doit,fgSvec(t1),fgSvec(t1t1));

    uint            t2 = m_graph.addNode<std::size_t>(0,"t2"),
                    t2t2 = m_graph.addNode<std::size_t>(0,"t2(t2)");
    m_graph.addLink(doit,fgSvec(t2),fgSvec(t2t2));

    uint            throws = m_graph.addNode<std::size_t>(0,"throws");
    m_graph.addLink(doit,fgSvec(t1t1,t2t2),fgSvec(throws));

    FG_TEST_CHECK_THROW_1(m_graph.getNode(throws),
                          "A computation within an FgDepGraph has generated an exception");
}

void
fgDepGraphTest(const FgArgs &)
{
    fg_debug_thread = true;           // Turn on debug output
    testDepGraphSimple();
    testDepGraphSubset();
    testDepGraphExceptions();
    testDepGraphExceptionsMulti();
    testDepGraphCopyable();
    fg_debug_thread = false;
    testDepGraphMulti();
}
