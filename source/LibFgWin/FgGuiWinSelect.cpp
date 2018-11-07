//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Oct 14, 2011
//

#include "stdafx.h"

#include "FgGuiApiSelect.hpp"
#include "FgGuiWin.hpp"
#include "FgThrowWindows.hpp"
#include "FgBounds.hpp"
#include "FgDefaultVal.hpp"
#include "FgMetaFormat.hpp"
#include "FgAlgs.hpp"

using namespace std;

struct  FgGuiWinSelect : public FgGuiOsBase
{
    FgGuiApiSelect                  m_api;
    vector<FgPtr<FgGuiOsBase> >     m_panes;
    size_t                          m_currPane;     // Which one is Windows currently displaying ?
    FgVect2I                        m_lo,m_sz;
    FgString                        m_store;

    FgGuiWinSelect(const FgGuiApiSelect & api)
        : m_api(api)
    {
        FGASSERT(api.wins.size() > 0);
        m_panes.resize(api.wins.size());
        for (size_t ii=0; ii<m_panes.size(); ++ii)
            m_panes[ii] = api.wins[ii]->getInstance();
    }

    virtual void
    create(HWND parentHwnd,int,const FgString & store,DWORD extStyle,bool visible)
    {
        m_store = store;
        for (size_t ii=0; ii<m_panes.size(); ++ii)
            m_panes[ii]->create(parentHwnd,int(ii),m_store+"_"+fgToStr(ii),extStyle,false);
        m_currPane = g_gg.getVal(m_api.selection);
        if (visible)
            m_panes[m_currPane]->showWindow(true);
    }

    virtual void
    destroy()
    {
        for (size_t ii=0; ii<m_panes.size(); ++ii)
            m_panes[ii]->destroy();
    }

    virtual FgVect2UI
    getMinSize() const
    {
        FgVect2UI   max(0);
        for (size_t ii=0; ii<m_panes.size(); ++ii)
            max = fgMax(max,m_panes[ii]->getMinSize());
        return max;
    }

    virtual FgVect2B
    wantStretch() const
    {
        FgVect2B    ret(false,false);
        for (size_t ii=0; ii<m_panes.size(); ++ii)
            ret = fgOr(ret,m_panes[ii]->wantStretch());
        return ret;
    }

    virtual void
    updateIfChanged()
    {
        if (g_gg.dg.update(m_api.updateNodeIdx)) {
            size_t      currPane = g_gg.getVal(m_api.selection);
            if (currPane != m_currPane) {
                m_panes[m_currPane]->showWindow(false);
                m_currPane = currPane;
                m_panes[m_currPane]->showWindow(true);
                // Only previously current pane was last updated for size, plus the
                // MoveWindow call will refresh the screen (ShowWindow doesn't):
                m_panes[m_currPane]->moveWindow(m_lo,m_sz);
            }
        }
        m_panes[m_currPane]->updateIfChanged();
    }

    virtual void
    moveWindow(FgVect2I lo,FgVect2I sz)
    {
        if (sz[0] * sz[1] > 0) {
            m_lo = lo;
            m_sz = sz;
            m_panes[m_currPane]->moveWindow(lo,sz);
        }
    }

    virtual void
    showWindow(bool s)
    {m_panes[m_currPane]->showWindow(s); }

    virtual void
    saveState()
    {
        for (size_t ii=0; ii<m_panes.size(); ++ii)
            m_panes[ii]->saveState();
    }
};

FgPtr<FgGuiOsBase>
fgGuiGetOsInstance(const FgGuiApiSelect & api)
{return FgPtr<FgGuiOsBase>(new FgGuiWinSelect(api)); }
