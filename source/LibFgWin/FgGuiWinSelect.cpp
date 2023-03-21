//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
//

#include "stdafx.h"

#include "FgGuiApiSelect.hpp"
#include "FgGuiWin.hpp"
#include "FgThrowWindows.hpp"
#include "FgBounds.hpp"
#include "FgMetaFormat.hpp"

using namespace std;

namespace Fg {

struct  GuiSelectWin : public GuiBaseImpl
{
    GuiSelect               m_api;
    GuiImplPtrs             m_panes;
    size_t                  m_currPane;     // Which one is Windows currently displaying ?
    Vec2I                   m_lo,m_sz;
    String8                 m_store;
    HWND                    parentHwnd;
    DWORD                   extStyle;

    GuiSelectWin(const GuiSelect & api)
        : m_api(api)
    {
        FGASSERT(api.wins.size() > 0);
        m_panes.resize(api.wins.size());
        for (size_t ii=0; ii<m_panes.size(); ++ii)
            m_panes[ii] = api.wins[ii]->getInstance();
    }

    virtual void
    create(HWND parentHwnd_,int,String8 const & store,DWORD extStyle_,bool visible)
    {
        m_store = store;
        parentHwnd = parentHwnd_;
        extStyle = extStyle_;
        FGASSERT(m_api.selection.cref() < m_panes.size());
        m_currPane = m_api.selection.cref();
        m_panes[m_currPane]->create(parentHwnd,int(m_currPane),m_store+"_"+toStr(m_currPane),extStyle,visible);
    }

    virtual void
    destroy()
    {
        m_panes[m_currPane]->destroy();
    }

    virtual Vec2UI
    getMinSize() const
    {
        Vec2UI   max(0);
        for (size_t ii=0; ii<m_panes.size(); ++ii)
            max = cMax(max,m_panes[ii]->getMinSize());
        return max;
    }

    virtual Vec2B
    wantStretch() const
    {
        Vec2B    ret(false,false);
        for (size_t ii=0; ii<m_panes.size(); ++ii)
            ret = mapOr(ret,m_panes[ii]->wantStretch());
        return ret;
    }

    virtual void
    updateIfChanged()
    {
        if (m_api.selection.checkUpdate()) {
            size_t      currPane = m_api.selection.cref();
            if (currPane != m_currPane) {
                FGASSERT1(currPane < m_panes.size(),toStr(currPane));
                m_panes[m_currPane]->destroy();
                m_currPane = currPane;
                m_panes[m_currPane]->create(parentHwnd,int(m_currPane),m_store+"_"+toStr(m_currPane),extStyle,true);
                // Only previously current pane was last updated for size, plus the
                // MoveWindow call will refresh the screen (ShowWindow doesn't):
                m_panes[m_currPane]->moveWindow(m_lo,m_sz);
            }
        }
        m_panes[m_currPane]->updateIfChanged();
    }

    virtual void
    moveWindow(Vec2I lo,Vec2I sz)
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
};

GuiImplPtr
guiGetOsImpl(const GuiSelect & api)
{return GuiImplPtr(new GuiSelectWin(api)); }

}
