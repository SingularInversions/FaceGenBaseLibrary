//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Andrew Beatty
// Created: Feb 16, 2018
//

#include "stdafx.h"

#include "FgGuiApiDialogs.hpp"

using namespace std;

static
void
fileLoad(FgString fileTypesDescription,FgStrs extensions,string storeID,FgDgn<FgString> output)
{
    FgOpt<FgString>     fname = fgGuiDialogFileLoad(fileTypesDescription,extensions,storeID);
    if (fname.valid())
        g_gg.setVal(output,fname.val());
}

FgGuiPtr
fgGuiFileLoadButton(
    const FgString &            buttonText,
    const FgString &            fileTypesDescription,
    const FgStrs &              extensions,
    const string &              storeID,
    FgDgn<FgString>             output)
{
    FgGuiApiButton          ret;
    ret.label = buttonText;
    ret.action = boost::bind(fileLoad,fileTypesDescription,extensions,storeID,output);
    return fgsp(ret);
}

// */
