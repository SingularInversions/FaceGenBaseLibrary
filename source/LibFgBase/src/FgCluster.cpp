//
// Copyright (c) 2015 Singular Inversions Inc.
//
// Authors: Andrew Beatty
// Created: Jan 17, 2012
//

#include "stdafx.h"

#include "FgDiagnostics.hpp"
#include "FgCluster.hpp"
#include "FgStdVector.hpp"
#include "FgPath.hpp"
#include "FgNc.hpp"
#include "FgFileSystem.hpp"
#include "FgBuild.hpp"
#include "FgCl.hpp"
#include "FgConio.hpp"
#include "FgTime.hpp"
#include "FgCommand.hpp"
#include "FgSyntax.hpp"
#include "FgParse.hpp"

using namespace std;

// Only one port is needed because we can have simultaneous TCP connections with all workers since:
// 1. Set keepalive option (defaults to 2 hour interval) and works on non-proxy connection
// 2. Connections defined by 2 IPs and 2 port number tuple, so unique.
// This will keep things simpler than having 2-way connection architecture.
static inline
uint16
port() {return 59407; }

static
string
testWorkerFunc(const string & msg)
{
    FgDbls      vals;
    fgDeserialize(msg,vals);
    fgout << fgnl << "Received: " << vals;
    return fgSerialize(fgSum(vals));
}

static
void
testCoordinator(const FgStrs & workerIPs)
{
    shared_ptr<FgClustDispatcher>   dispatcher = fgClustDispatcher(workerIPs,port(),512);
    FgDbls              vals = fgSvec(3.14,2.72,1.41);
    string              msg = fgSerialize(vals);
    FgStrs              msgsOut(workerIPs.size(),msg),
                        msgsIn(workerIPs.size());
    dispatcher->batchProcess(msgsOut,msgsIn);
    for (size_t ww=0; ww<workerIPs.size(); ++ww) {
        double          res;
        fgDeserialize(msgsIn[ww],res);
        fgout << fgnl << "Worker " << ww << " result: " << res;
        FGASSERT(res == fgSum(vals));
    }
}

// Fully automated test is limited to host computer so can only test with a single worker
// (TCP connections are only unique to {IP+port <-> IP+port} 4-tuple)
void
fgClusterTest(const FgArgs &)
{
    boost::thread       worker(fgClustWorker,port(),testWorkerFunc,256);
    testCoordinator(fgSvec<string>("127.0.0.1"));    // Local host loop-back IP for testing
}

void
fgClusterTestm(const FgArgs & args)
{
    FgSyntax            syn(args,"w | (c <ip>+)\n"
        "    w - start worker machine. Make sure you do this first on all computers to be referenced by <ip>+\n"
        "    c - coordinator. Makes use of the workers at <ip>+"
    );
    if (syn.next() == "w")
        return fgClustWorker(port(),testWorkerFunc,256);
    if (syn.curr() == "c") {
        FgStrs      ips;
        do
            ips.push_back(syn.next());
        while (syn.more());
        return testCoordinator(ips);
    }
    syn.error("Unknown command",syn.curr());
}

// */
