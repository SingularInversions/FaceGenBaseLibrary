//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     March 22, 2011
//

#include "stdafx.h"

#include "FgGuiApiSpacer.hpp"
#include "FgGuiWin.hpp"
#include "FgThrowWindows.hpp"
#include "FgMatrixC.hpp"
#include "FgBounds.hpp"
#include "FgDefaultVal.hpp"
#include "FgMetaFormat.hpp"

using namespace std;

struct  FgGuiWinSpacer : public FgGuiOsBase
{
    FgGuiApiSpacer              m_api;

    FgGuiWinSpacer(const FgGuiApiSpacer & api) :
        m_api(api)
    {}

    virtual void
    create(HWND,int,const FgString &,DWORD,bool)
    {}

    virtual void
    destroy()
    {}

    virtual FgVect2UI
    getMinSize() const
    {return m_api.size; }

    virtual FgVect2B
    wantStretch() const
    {return FgVect2B(false,false); }

    virtual void
    updateIfChanged()
    {}

    virtual void
    moveWindow(FgVect2I,FgVect2I)
    {}

    virtual void
    showWindow(bool)
    {}

    virtual void
    saveState()
    {}
};

FgPtr<FgGuiOsBase>
fgGuiGetOsInstance(const FgGuiApiSpacer & def)
{return FgPtr<FgGuiOsBase>(new FgGuiWinSpacer(def)); }
