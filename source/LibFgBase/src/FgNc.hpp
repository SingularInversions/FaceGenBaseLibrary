//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Network Computation

#ifndef FGNC_HPP
#define FGNC_HPP

#include "FgStdString.hpp"
#include "FgStdVector.hpp"
#include "FgSerialize.hpp"
#include "FgBuild.hpp"
#include "FgFileSystem.hpp"
#include "FgSerial.hpp"

namespace Fg {

// Location of network computing share root location as specified on given *native* build OS:
String
getNcShare(BuildOS nativeBuildOS);

// As above for current host OS:
String
getNcShare();

// An HTML log of given commands and outputs will be appended to 'logFile' and a 32x32 image
// will be written to <logFileBaseName>.jpg, green for success of all commands, red for a fail:
struct  NcScript
{
    String              logFile;    // Directory path will be created if it doesn't exist
    String              title;      // Title line of log file
    // Each such command will be shell executed in order. In addition some builtin commands are supported:
    // fgPush <dir>     - push <dir> to current for this process
    // fgPop            - pop back to previous dir for this process
    Strings             cmds;

    String ser() const
    {
        String ret;
        ret.append(fgSer(logFile));
        ret.append(fgSer(title));
        ret.append(fgSer(cmds));
        return ret;
    }

    void dsr(const char * & ptr,const char * end)
    {
        fgDsr(ptr,end,logFile);
        fgDsr(ptr,end,title);
        fgDsr(ptr,end,cmds);
    }

    String serMsg() const
    {
        String ret = fgSer(0xFE785A765844B8D1ULL);
        ret.append(ser());
        return ret;
    }

    void dsrMsg(String const & msg)
    {
        const char *ptr = &msg[0],*end = ptr+msg.size();
        uint64 ver;
        fgDsr(ptr,end,ver);
        FGASSERT(ver == 0xFE785A765844B8D1ULL);
        dsr(ptr,end);
    }
};

inline
uint16
getNcServerPort()
{return 59405; }

inline
String
getCiShareBoot()
{return getNcShare() + toNativeSeparator("ci/boot/"); }

inline
String
getCiShareBoot(BuildOS os)
{return getNcShare(os) + cNsOs("ci/boot/",os); }

inline
String
getCiShareRepo()
{return getNcShare() + toNativeSeparator("ci/root/"); }

inline
String
getCiShareRepo(BuildOS os)
{return getNcShare(os) + cNsOs("ci/root/",os); }

}

#endif

// */
