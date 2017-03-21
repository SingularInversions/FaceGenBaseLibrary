//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Dec 8, 2016
//

#include "stdafx.h"

#include "FgFileUtils.hpp"

using namespace std;

bool
fgUpdateFiles(uint num,const FgStrings & ins,const FgStrings & outs,FgUpdateFilesFunc func)
{
    if (fgNewer(ins,outs)) {
        fgout.logFile(fgToString(num)+"_log.txt",false,false);
        fgout << fgnl << "(" << fgCat(ins,",") << ") -> (" << fgCat(outs,",") << ")" << fgpush;
        FgTimer     time;
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

// */
