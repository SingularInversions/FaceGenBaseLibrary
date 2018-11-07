//
// Copyright (c) 2018 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Andrew Beatty
// Created: 18.07.27
//

#include "stdafx.h"

#include "FgGaBase.hpp"
#include "FgCommand.hpp"

using namespace std;

void
fgGaTestm(const FgArgs &)
{
    FgAny           txtDat = FgLeg::input(string("Hello World"));
    FgGa::WinPtr    txtWin = FgGa::winText(txtDat);
    FgGa::startMain(txtWin,"fgGaTestm");
}

namespace FgGa {

struct  WinText : Win
{
    FgAny               text;
    FgMat22UI           dims;   // Rows are X,Y, columns are min/max

    WinText(const FgAny & txtDat) : text(txtDat), dims(20,1000,20,8000) {}

    virtual FgMat22UI getDims() const
    {return dims; }

    virtual void render(FgVect2UI /*region*/,const FgAny & /*osData*/) const {}
};

WinPtr
winText(const FgAny & txtDat)
{return make_shared<WinText>(txtDat); }

}

// */
