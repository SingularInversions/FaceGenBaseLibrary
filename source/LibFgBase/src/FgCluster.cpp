//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgCluster.hpp"
#include "FgDiagnostics.hpp"
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

namespace Fg {

void
fgClusterDeploy(
    string const &          name,
    const FgFuncCrdntor &   crdntor,
    const FgFnStr2Str &     worker,
    string const &          coordIP,
    Strings const &          workIPs,
    String8s const &       files)
{
    String8        exeDir = getExecutableDirectory();
    if (pathExists(exeDir+"cluster_worker.flag"))         // Client could be in an arbitrary data subdirectory at this point
        return fgClustWorker(worker,fgClusterPortDefault());
    if (pathExists(exeDir+"cluster_coordinator.flag")) {
        std::shared_ptr<FgClustDispatcher> dispatcher = fgClustDispatcher(workIPs);
        return crdntor(dispatcher.get());               // Performs all cluster work before returning
    }
    // This instance must be deployment:
    fgout << fgnl <<
        "Have you built the updated linux binaries and started the coordinator and worker machines ?\n"
        "<enter> to confirm, 'q' to quit:";
    if (uint(fgGetch()) != 13)
        return;
    // Use coordinator machine to copy executable to file server via CIFS, preserving executable bit permission:
    string          filesDirLocal = getNcShare() + "cc/" + name + '/',
                    filesDirUbu = getNcShare(BuildOS::linux) + "cc/" + name + '/';
    createPath(filesDirLocal+"data/");
    NcScript      scriptCrdntor;
    scriptCrdntor.outputBase = "cc/" + name + "/_log_setup";
    scriptCrdntor.title = name + " coordinator (" + coordIP + ")";
    String8        binBase = Path(getExecutablePath()).base;
    scriptCrdntor.cmds.push_back("cp " + getCiShareBoot(BuildOS::linux) + "bin/linux/clang/64/release/" + binBase.m_str + " " + filesDirUbu + binBase.m_str);
    if (!runTcpClient(coordIP,getNcServerPort(),toMessageExplicit(scriptCrdntor)))
        fgThrow("Cluster deploy unable to access coordinator",coordIP);
    // Wait for file to be copied (asynchronous operation):
    sleepSeconds(1);
    // Deploy data files from current repo:
    String8        dd = dataDir(),
                    td = filesDirLocal+"data/";
    for (size_t ff=0; ff<files.size(); ++ff) {
        String8    dest = td + files[ff];
        createPath(Path(dest).dir());
        fileCopy(dd+files[ff],dest,true);
    }
    // Start workers:
    string          args = mainArgs();
    Strings          copyData;
    copyData.push_back("fgPush cc");
    copyData.push_back("fgPush " + name);
    copyData.push_back("cp -r " + filesDirUbu + "* .");
    for (size_t ww=0; ww<workIPs.size(); ++ww) {
        NcScript      script;
        string          idxStr = toStrDigits(ww,3);
        script.outputBase = "cc/" + name + "/_log";
        script.title = name + " worker " + idxStr + " (" + workIPs[ww] + ")";
        script.cmds = copyData;
        script.cmds.push_back("> cluster_worker.flag");
        script.cmds.push_back("./" + args);
        if (!runTcpClient(workIPs[ww],getNcServerPort(),toMessageExplicit(script)))
            fgThrow("Cluster deploy unable to start worker",workIPs[ww]);
    }
    // Wait to ensure workers ready to receive (asynchronous operation):
    sleepSeconds(1);
    // Start Coordinator:
    scriptCrdntor.outputBase = "cc/" + name + "/_log";
    scriptCrdntor.cmds = copyData;
    scriptCrdntor.cmds.push_back("> cluster_coordinator.flag");
    scriptCrdntor.cmds.push_back("./" + args);
    if (!runTcpClient(coordIP,getNcServerPort(),toMessageExplicit(scriptCrdntor)))
        fgThrow("Cluster deploy unable to start coordinator",coordIP);
}

static
string
testWorkerFunc(string const & msg)
{
    Doubles      vals;
    fgDeserialize(msg,vals);
    //fgout << fgnl << "Received: " << vals;
    return fgSerialize(cSum(vals));
}

static
void
testCoordinator(const FgClustDispatcher * dispatcher)
{
    Doubles              vals = svec(3.14,2.72,1.41);
    string              msg = fgSerialize(vals);
    size_t              sz = dispatcher->numMachines();
    Strings              msgsOut(sz,msg),
                        msgsIn(sz);
    dispatcher->batchProcess(msgsOut,msgsIn);
    for (size_t ww=0; ww<sz; ++ww) {
        double          res;
        fgDeserialize(msgsIn[ww],res);
        //fgout << fgnl << "Worker " << ww << " result: " << res;
        FGASSERT(res == cSum(vals));
    }
}

// Fully automated test is limited to host computer so can only test with a single worker
// (TCP connections are only unique to {IP+fgClusterPortDefault <-> IP+fgClusterPortDefault} 4-tuple)
void
fgClusterTest(CLArgs const &)
{
    std::thread       worker(fgClustWorker,testWorkerFunc,fgClusterPortDefault());
    shared_ptr<FgClustDispatcher>   dispatcher = fgClustDispatcher(svec<string>("127.0.0.1"),fgClusterPortDefault());
    testCoordinator(dispatcher.get());      // Local host loop-back IP for testing
}

void
fgClusterTestm(CLArgs const & args)
{
    Syntax            syntax(args,"w | (c <ip>+)\n"
        "    w - start worker machine. Make sure you do this first on all computers to be referenced by <ip>+\n"
        "    c - coordinator. Makes use of the workers at <ip>+"
    );
    if (syntax.next() == "w")
        return fgClustWorker(testWorkerFunc,fgClusterPortDefault());
    if (syntax.curr() == "c") {
        Strings      ips;
        do
            ips.push_back(syntax.next());
        while (syntax.more());
        shared_ptr<FgClustDispatcher>   dispatcher = fgClustDispatcher(ips,fgClusterPortDefault());
        return testCoordinator(dispatcher.get());
    }
    syntax.error("Unknown command",syntax.curr());
}

void
fgClusterDeployTestm(CLArgs const & args)
{
    Syntax        syntax(args,"<crdntorIP> <workerIP>+\n"
        "    <IP> - If no periods are entered then '192.168.0' is automatically prepended.\n"
        "NOTES\n"
        "    * All <IP> machines must be running fgNcServer under linux"
    );
    string      crdntorIP = syntax.next();
    if (!contains(crdntorIP,'.'))
        crdntorIP = "192.168.0." + crdntorIP;
    Strings      workerIPs;
    do {
        string  wip = syntax.next();
        if (!contains(wip,'.'))
            wip = "192.168.0." + wip;
        workerIPs.push_back(wip);
    }
    while (syntax.more());
    fgClusterDeploy("test0",testCoordinator,testWorkerFunc,crdntorIP,workerIPs,String8s());
}

}

// */
