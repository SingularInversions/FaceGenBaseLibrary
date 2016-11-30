//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Oct 26, 2015
//

#include "stdafx.h"

#include "FgGuiApiDynamic.hpp"
#include "FgGuiWin.hpp"
#include "FgThrowWindows.hpp"
#include "FgMatrixC.hpp"
#include "FgBounds.hpp"
#include "FgDefaultVal.hpp"

using namespace std;

struct  FgGuiWinDynamic : public FgGuiOsBase
{
    FgGuiApiDynamic             m_api;
    FgGuiOsPtr                  m_win;
    FgString                    m_store;
    HWND                        m_hwndParent;
    FgVect2I                    m_lastMoveLo;
    FgVect2I                    m_lastMoveSz;

    FgGuiWinDynamic(const FgGuiApiDynamic & api) :
        m_api(api)
    {}

    virtual void
    create(HWND parentHwnd,int,const FgString & store,DWORD /*extStyle*/,bool visible)
    {
//fgout << fgnl << "FgGuiWinDynamic::create" << fgpush;
        // Ignore extStyle since this isn't a win32 window and it's not recursively passed.
        m_store = store;
        m_hwndParent = parentHwnd;
        // Since this is not a win32 window, we create the sub-windows here within the parent
        // WM_CREATE handler:
        m_win = m_api.getWin()->getInstance();
        m_win->create(parentHwnd,0,store,NULL,visible);
//fgout << fgpop;
    }

    virtual void
    destroy()
    {m_win->destroy(); }

    virtual FgVect2UI
    getMinSize() const
    {return m_api.minSize; }

    virtual FgVect2B
    wantStretch() const
    {return m_api.wantStretch; }

    virtual void
    updateIfChanged()
    {
        if (g_gg.dg.update(m_api.updateFlagIdx)) {
            // This function is only called by the parent if this window is visible:
            m_win->destroy();
            m_win = m_api.getWin()->getInstance();
            m_win->create(m_hwndParent,0,m_store,NULL,true);
            // Must tell the panels what size to be before they can be displayed:
            m_win->moveWindow(m_lastMoveLo,m_lastMoveSz);
        }
        m_win->updateIfChanged();
    }

    virtual void
    moveWindow(FgVect2I base,FgVect2I size)
    {
//fgout << fgnl << "FgGuiWinDynamic::moveWindow " << base << " , " << size << fgpush;
        m_lastMoveLo = base;
        m_lastMoveSz = size;
        m_win->moveWindow(base,size);
//fgout << fgpop;
    }

    virtual void
    showWindow(bool s)
    {m_win->showWindow(s); }

    virtual void
    saveState()
    {m_win->saveState(); }
};

FgPtr<FgGuiOsBase>
fgGuiGetOsInstance(const FgGuiApiDynamic & def)
{return FgPtr<FgGuiOsBase>(new FgGuiWinDynamic(def)); }
