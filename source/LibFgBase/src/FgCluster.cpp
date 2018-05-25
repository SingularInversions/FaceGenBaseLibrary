//
// Copyright (c) 2015 Singular Inversions Inc.
//
// Authors: Andrew Beatty
// Created: May 31, 2017
//

#include "stdafx.h"

#include "FgCluster.hpp"
#include "FgDiagnostics.hpp"
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
#include "FgTcp.hpp"

using namespace std;

void
fgClusterDeploy(
    const string &          name,
    const FgFuncCrdntor &   crdntor,
    const FgFuncStr2Str &   worker,
    const string &          coordIP,
    const FgStrs &          workIPs,
    const FgStrings &       files)
{
    FgString        exeDir = fgExecutableDirectory();
    if (fgExists(exeDir+"cluster_worker.flag"))         // Client could be in an arbitrary data subdirectory at this point
        return fgClustWorker(worker,fgClusterPortDefault());
    if (fgExists(exeDir+"cluster_coordinator.flag")) {
        std::shared_ptr<FgClustDispatcher> dispatcher = fgClustDispatcher(workIPs);
        return crdntor(dispatcher.get());               // Performs all cluster work before returning
    }
    // This instance must be deployment:
    fgout << fgnl <<
        "Have you built the updated Ubuntu binaries and started the coordinator and worker machines ?\n"
        "<enter> to confirm, 'q' to quit:";
    if (uint(fgGetch()) != 13)
        return;
    // Use coordinator machine to copy executable to file server via CIFS, preserving executable bit permission:
    string          filesDirLocal = fgNcShare() + "cc/" + name + '/',
                    filesDirUbu = fgNcShare("ubuntu") + "cc/" + name + '/';
    fgCreatePath(filesDirLocal+"data/");
    FgNcScript      scriptCrdntor;
    scriptCrdntor.logFile = "cc/" + name + "/_log_setup.html";
    scriptCrdntor.title = name + " coordinator (" + coordIP + ")";
    FgString        binBase = FgPath(fgExecutablePath()).base;
    scriptCrdntor.cmds.push_back("cp " + fgCiShareBoot("ubuntu") + "bin/ubuntu/clang/64/release/" + binBase.m_str + " " + filesDirUbu + binBase.m_str);
    if (!fgTcpClient(coordIP,fgNcServerPort(),scriptCrdntor.serMsg()))
        fgThrow("Cluster deploy unable to access coordinator",coordIP);
    // Wait for file to be copied (asynchronous operation):
    fgSleep(1);
    // Deploy data files from current repo:
    FgString        dd = fgDataDir(),
                    td = filesDirLocal+"data/";
    for (size_t ff=0; ff<files.size(); ++ff) {
        FgString    dest = td + files[ff];
        fgCreatePath(FgPath(dest).dir());
        fgCopyFile(dd+files[ff],dest,true);
    }
    // Start workers:
    string          args = fgMainArgs();
    FgStrs          copyData;
    copyData.push_back("fgPush cc");
    copyData.push_back("fgPush " + name);
    copyData.push_back("cp -r " + filesDirUbu + "* .");
    for (size_t ww=0; ww<workIPs.size(); ++ww) {
        FgNcScript      script;
        string          idxStr = fgToStringDigits(ww,3);
        script.logFile = "cc/" + name + "/_log.html";
        script.title = name + " worker " + idxStr + " (" + workIPs[ww] + ")";
        script.cmds = copyData;
        script.cmds.push_back("> cluster_worker.flag");
        script.cmds.push_back("./" + args);
        if (!fgTcpClient(workIPs[ww],fgNcServerPort(),script.serMsg()))
            fgThrow("Cluster deploy unable to start worker",workIPs[ww]);
    }
    // Wait to ensure workers ready to receive (asynchronous operation):
    fgSleep(1);
    // Start Coordinator:
    scriptCrdntor.logFile = "cc/" + name + "/_log.html";
    scriptCrdntor.cmds = copyData;
    scriptCrdntor.cmds.push_back("> cluster_coordinator.flag");
    scriptCrdntor.cmds.push_back("./" + args);
    if (!fgTcpClient(coordIP,fgNcServerPort(),scriptCrdntor.serMsg()))
        fgThrow("Cluster deploy unable to start coordinator",coordIP);
}

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
testCoordinator(const FgClustDispatcher * dispatcher)
{
    FgDbls              vals = fgSvec(3.14,2.72,1.41);
    string              msg = fgSerialize(vals);
    size_t              sz = dispatcher->numMachines();
    FgStrs              msgsOut(sz,msg),
                        msgsIn(sz);
    dispatcher->batchProcess(msgsOut,msgsIn);
    for (size_t ww=0; ww<sz; ++ww) {
        double          res;
        fgDeserialize(msgsIn[ww],res);
        fgout << fgnl << "Worker " << ww << " result: " << res;
        FGASSERT(res == fgSum(vals));
    }
}

// Fully automated test is limited to host computer so can only test with a single worker
// (TCP connections are only unique to {IP+fgClusterPortDefault <-> IP+fgClusterPortDefault} 4-tuple)
void
fgClusterTest(const FgArgs &)
{
    boost::thread       worker(fgClustWorker,testWorkerFunc,fgClusterPortDefault());
    shared_ptr<FgClustDispatcher>   dispatcher = fgClustDispatcher(fgSvec<string>("127.0.0.1"),fgClusterPortDefault());
    testCoordinator(dispatcher.get());      // Local host loop-back IP for testing
}

void
fgClusterTestm(const FgArgs & args)
{
    FgSyntax            syntax(args,"w | (c <ip>+)\n"
        "    w - start worker machine. Make sure you do this first on all computers to be referenced by <ip>+\n"
        "    c - coordinator. Makes use of the workers at <ip>+"
    );
    if (syntax.next() == "w")
        return fgClustWorker(testWorkerFunc,fgClusterPortDefault());
    if (syntax.curr() == "c") {
        FgStrs      ips;
        do
            ips.push_back(syntax.next());
        while (syntax.more());
        shared_ptr<FgClustDispatcher>   dispatcher = fgClustDispatcher(ips,fgClusterPortDefault());
        return testCoordinator(dispatcher.get());
    }
    syntax.error("Unknown command",syntax.curr());
}

void
fgClusterDeployTestm(const FgArgs & args)
{
    FgSyntax        syntax(args,"<crdntorIP> <workerIP>+\n"
        "    <IP> - If no periods are entered then '192.168.0' is automatically prepended.\n"
        "NOTES\n"
        "    * All <IP> machines must be running fgNcServer under Ubuntu"
    );
    string      crdntorIP = syntax.next();
    if (!fgContains(crdntorIP,'.'))
        crdntorIP = "192.168.0." + crdntorIP;
    FgStrs      workerIPs;
    do {
        string  wip = syntax.next();
        if (!fgContains(wip,'.'))
            wip = "192.168.0." + wip;
        workerIPs.push_back(wip);
    }
    while (syntax.more());
    fgClusterDeploy("test0",testCoordinator,testWorkerFunc,crdntorIP,workerIPs,FgStrings());
}

// */
