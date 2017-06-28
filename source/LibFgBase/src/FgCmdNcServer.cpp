//
// Copyright (C) Singular Inversions Inc. 2011
//
// Authors: Andrew Beatty
// Created: Dec 7, 2011
//
// Network Computing server.
// Simplest possible server for handling scripts remotely for CI and CC setup.
// Totally insecure, use only on private LAN.
//

#include "stdafx.h"

#include "FgCommand.hpp"
#include "FgSyntax.hpp"
#include "FgFileSystem.hpp"
#include "FgTcp.hpp"
#include "FgDiagnostics.hpp"
#include "FgTime.hpp"
#include "FgNc.hpp"
#include "FgImageIo.hpp"

using namespace std;

static
bool
run(const string & logFile,const string & cmd)
{
    fgWriteFile(logFile,"<h3>" + cmd + "</h3>\n<pre>\n");
    string      cmdLog = cmd + " >> " + logFile + " 2>&1";
    // Log file must be closed for this command to write to it:
    int         ret = system(cmdLog.c_str());
    fgWriteFile(logFile,"</pre>\n");
    return (ret == 0);
}

static
bool
runScript(const string & logFile,const vector<string> & cmds)
{
    const string    push = "fgPush ",
                    pop = "fgPop";
    FgPushDir       dirStack;
    for (size_t ii=0; ii<cmds.size(); ++ii) {
        const string &  cmd = cmds[ii];
        if (fgStartsWith(cmd,push)) {
            string      dir(cmd.begin()+push.size(),cmd.end());
            FgOfstream  ofs(logFile,true);
            ofs << "<h3> pushd " << dir << "</h3>\n";
            if (!fgExists(dir))
                fgCreateDirectory(dir);
            if (fgIsDirectory(dir))
                dirStack.push(dir);
            else {
                ofs << "directory not found\n";
                return false;
            }
        }
        else if (fgStartsWith(cmd,pop)) {
            FgOfstream  ofs(logFile,true);
            ofs << "<h3> popd </h3>\n";
            dirStack.pop();
        }
        else if (!run(logFile,cmds[ii]))
            return false;
    }
    return true;
}

static
bool
handler(
    const string &  addr,
    const string &  dataIn,
    string &)
{
    FgNcScript      script;
    fgDeserializePort(dataIn,script);
    FgPath          logPath(script.logFile);
    if (!logPath.root)                                          // If path is relative
        logPath = FgPath(fgGetCurrentDir()+script.logFile);     // Make absolute
    fgCreatePath(logPath.dir());                                // Create path if necessary
    script.logFile = logPath.str().m_str;
    string          dirBase = logPath.dirBase().m_str;
    FgImgRgbaUb     img(32,32,FgRgbaUB(255,255,0,255));
    fgSaveImgAnyFormat(dirBase+".jpg",img);
    fgWriteFile(script.logFile,
            "<html>\n"
            "<head>\n"
            "<title>" + script.title + "</title>\n"
            "</head>\n"
            "<body>\n"
            "<h1>" + script.title + "</h1>\n"
            "<h3>" + fgDateTimeString() + " (client: " + addr + ")</h3>\n"
            "<font size=\"2\">\n",false);
    bool            res = runScript(script.logFile,script.cmds);
    FgOfstream      ofs(script.logFile,true);
    ofs << "\n<h2>DONE - " << (res ? "SUCCESS" : "FAILURE") << "</h2>\n"
        << "</body>\n</html>\n";
    if (res)
        img = FgImgRgbaUb(32,32,FgRgbaUB(0,255,0,255));
    else
        img = FgImgRgbaUb(32,32,FgRgbaUB(255,0,0,255));
    fgSaveImgAnyFormat(dirBase+".jpg",img);
    return true;
}

void
fgCmdNcServer(const FgArgs & args)
{
    if (args.size() > 1)
        fgThrow(args[0]+" takes no arguments");
    fgout.setCout(true);
    fgout.logFile("fgNcServerLog.txt");
    // Despite this running in a new process each time, TCP errors can render
    // the port unusable on Windows until an OS reboot.
    fgTcpServer(fgNcServerPort(),false,handler,0x10000);
}

// */
