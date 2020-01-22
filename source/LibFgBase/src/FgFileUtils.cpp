//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgFileUtils.hpp"

using namespace std;

namespace Fg {

bool
fgUpdateFiles(uint num,Ustrings const & ins,Ustrings const & outs,FgUpdateFilesFunc func)
{
    if (fileNewer(ins,outs)) {
        fgout.logFile(toStr(num)+"_log.txt",false,false);
        fgout << fgnl << "(" << cat(ins,",") << ") -> (" << cat(outs,",") << ")" << fgpush;
        Timer     time;
        func(ins,outs);
        double      elapsed = time.read();
        fgout << fgpop;
        if (elapsed > 3)    // Don't report time if less than 3s
            fgout << fgnl << time;
        fgout << fgnl;      // Avoids 'no newline' warnings in Mercurial
        fgout.logFileClose();
        return true;
    }
    return false;
}

}

// */
