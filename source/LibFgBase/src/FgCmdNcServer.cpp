//
// Copyright (C) Singular Inversions Inc. 2011
//
// Network Computing server.
// Simplest possible server for handling scripts remotely for CI and CC setup.
// Totally insecure, use only on private LAN.
//

#include "stdafx.h"

#include "FgPlatform.hpp"
#include "FgCommand.hpp"
#include "FgSyntax.hpp"
#include "FgFileSystem.hpp"
#include "FgTcp.hpp"
#include "FgDiagnostics.hpp"
#include "FgTime.hpp"
#include "FgNc.hpp"
#include "FgImageIo.hpp"

#ifndef FG_SANDBOX

using namespace std;

namespace Fg {

static
bool
run(string const & logFile,string const & cmd)
{
    fgWriteFile(logFile,"<h3>" + cmd + "</h3>\n<pre>\n");
#ifdef _MSC_VER         // win
    // The command must have quotes around the entire thing since the 'system' call is the same as typing
    // 'cmd.exe /c ... ', which strips any initial quote (and its pair) thus making it otherwise impossible
    // to invoke a command containing a space:
    string      cmdLog = '\"' + cmd + " >> " + logFile + " 2>&1 " + '\"';
#else                   // *nix
    string      cmdLog = cmd + " >> " + logFile + " 2>&1 ";
#endif
    // Log file must be closed for this command to write to it.
    int         ret = system(cmdLog.c_str());
    sleepSeconds(1);   // VS17 returns before releasing log file
    fgWriteFile(logFile,"</pre>\n");
    return (ret == 0);
}

static
bool
runScript(string const & logFile,Strings const & cmds)
{
    string const    push = "fgPush ",
                    pop = "fgPop";
    PushDir       dirStack;
    for (string const & cmd : cmds) {
        if (beginsWith(cmd,push)) {
            string      dir(cmd.begin()+push.size(),cmd.end());
            Ofstream  ofs(logFile,true);
            ofs << "<h3> pushd " << dir << "</h3>\n";
            if (!pathExists(dir))
                createDirectory(dir);
            if (isDirectory(dir))
                dirStack.push(dir);
            else {
                ofs << "directory not found\n";
                return false;
            }
        }
        else if (beginsWith(cmd,pop)) {
            Ofstream  ofs(logFile,true);
            ofs << "<h3> popd </h3>\n";
            dirStack.pop();
        }
        else if (!run(logFile,cmd))
            return false;
    }
    return true;
}

static
bool
handler(
    string const &  addr,
    string const &  dataIn,
    string &)
{
    NcScript      script = fromSerialMessage<NcScript>(dataIn);
    Path          logPath(script.logFile);
    if (!logPath.root)                                          // If path is relative
        logPath = Path(getCurrentDir()+script.logFile);     // Make absolute
    createPath(logPath.dir());                                // Create path if necessary
    script.logFile = logPath.str().m_str;
    string          dirBase = logPath.dirBase().m_str;
    ImgC4UC     img(32,32,RgbaUC(255,255,0,255));
    saveImage(dirBase+".jpg",img);
    fgWriteFile(script.logFile,
            "<html>\n"
            "<head>\n"
            "<title>" + script.title + "</title>\n"
            "</head>\n"
            "<body>\n"
            "<h1>" + script.title + "</h1>\n"
            "<h3>" + getDateTimeString() + " (client: " + addr + ")</h3>\n"
            "<font size=\"2\">\n",false);
    bool            res = runScript(script.logFile,script.cmds);
    Ofstream      ofs(script.logFile,true);
    ofs << "\n<h2>DONE - " << (res ? "SUCCESS" : "FAILURE") << "</h2>\n"
        << "</body>\n</html>\n";
    if (res)
        img = ImgC4UC(32,32,RgbaUC(0,255,0,255));
    else
        img = ImgC4UC(32,32,RgbaUC(255,0,0,255));
    saveImage(dirBase+".jpg",img);
    return true;
}

void
fgCmdNcServer(CLArgs const & args)
{
    if (args.size() > 1)
        fgThrow(args[0]+" takes no arguments");
    fgout.setDefOut(true);
    fgout.logFile("fgNcServerLog.txt");
    // Despite this running in a new process each time, TCP errors can render
    // the port unusable on Windows until an OS reboot.
    runTcpServer(getNcServerPort(),false,handler,0x10000);
}

}

#endif

// */
