//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
//

#include "stdafx.h"

#include "FgGuiApiSplit.hpp"
#include "FgGuiWin.hpp"
#include "FgThrowWindows.hpp"
#include "FgMatrixC.hpp"
#include "FgBounds.hpp"
#include "FgMetaFormat.hpp"

using namespace std;

namespace Fg {

struct  GuiSplitWin : public GuiBaseImpl
{
    GuiSplit                m_api;
    GuiImplPtrs             m_panes;
    Ustring                 m_store;
    HWND                    m_hwndParent;
    Vec2I                   m_base;     // client area
    Vec2I                   m_size;

    explicit
    GuiSplitWin(const GuiSplit & api) :
        m_api(api)
    {}

    virtual void
    create(HWND parentHwnd,int,Ustring const & store,DWORD /*extStyle*/,bool visible)
    {
//fgout << fgnl << "GuiSplitWin::create" << fgpush;
        // Ignore extStyle since this isn't a win32 window and it's not recursively passed.
        m_store = store;
        m_hwndParent = parentHwnd;
        // Since this is not a win32 window, we create the sub-windows here within the parent
        // WM_CREATE handler:
        m_panes.resize(m_api.panes.size());
        for (size_t ii=0; ii<m_api.panes.size(); ++ii) {
            m_panes[ii] = m_api.panes[ii]->getInstance();
            m_panes[ii]->create(m_hwndParent,int(ii),m_store+"_"+toStr(ii),NULL,visible);
        }
//fgout << fgpop;
    }

    virtual void
    destroy()
    {
        for (size_t ii=0; ii<m_panes.size(); ++ii)
            m_panes[ii]->destroy();
    }

    uint
    calcPadBetween(Uints const & paneMins) const
    {
        uint        maxMin = paneMins.empty() ? 0 : cMax(paneMins);
        return cMin(8,scast<uint>(0.1*maxMin));
    }

    uint
    calcPadTotal(uint padBetween) const
    {
        uint        gaps = scast<uint>(clampLo(scast<int>(m_panes.size())-1,0));
        return padBetween * gaps;
    }

    virtual Vec2UI
    getMinSize() const
    {
        Uints               paneMins(m_panes.size());
        uint                sd = m_api.horiz ? 0 : 1,
                            nd = 1 - sd,
                            sdSum = 0,
                            ndMax = 0;
        for (size_t ii=0; ii<m_panes.size(); ++ii) {
            Vec2UI          size = m_panes[ii]->getMinSize();
            paneMins[ii] = size[sd];
            sdSum += size[sd];
            ndMax = cMax(ndMax,size[nd]);
        }
        Vec2UI              ret;
        uint                padTotal = calcPadTotal(calcPadBetween(paneMins));
        ret[sd] = sdSum + padTotal;
        ret[nd] = ndMax;
        return ret;
    }
        // Here's how it would look for a grid layout ... but resizing more complex:
        //Vec2UI           gd = m_api.grid;
        //Uints        maxMinWid(gd[1],0),
        //                    maxMinHgt(gd[0],0);
        //for (uint yy=0; yy<gd[1]; ++yy) {
        //    for (uint xx=0; xx<gd[0]; ++xx) {
        //        Vec2UI   minSize = m_panes[yy*gd[0]+xx]->getMinSize();
        //        maxMinWid[yy] = cMax(maxMinWid[yy],minSize[0]);
        //        maxMinHgt[xx] = cMax(maxMinHgt[xx],minSize[1]);
        //    }
        //}
        //Vec2UI           minSize = Vec2UI(cSum(maxMinWid),cSum(maxMinHgt));
        //Vec2UI           padding = (m_api.grid-Vec2UI(1)) * 8;
        //return minSize+padding;


    virtual Vec2B
    wantStretch() const
    {
        Vec2B    ret(false,false);
        for (size_t ii=0; ii<m_panes.size(); ++ii)
            ret = fgOr(ret,m_panes[ii]->wantStretch());
        return ret;
    }

    virtual void
    updateIfChanged()
    {
        for (size_t ii=0; ii<m_panes.size(); ++ii)
            m_panes[ii]->updateIfChanged();
    }

    virtual void
    moveWindow(Vec2I base,Vec2I size)
    {
//fgout << fgnl << "GuiSplitWin::moveWindow " << base << " , " << size << fgpush;
        m_base = base;
        m_size = size;
        resize();
//fgout << fgpop;
    }

    virtual void
    showWindow(bool s)
    {
        for (size_t ii=0; ii<m_panes.size(); ++ii)
            m_panes[ii]->showWindow(s);
    }

    virtual void
    saveState()
    {
        for (size_t ii=0; ii<m_panes.size(); ++ii)
            m_panes[ii]->saveState();
    }

    void
    resize()
    {
        if (m_size[0] * m_size[1] > 0) {
            Uints               paneMins(m_panes.size());
            Bools               paneStrets(m_panes.size());
            uint                sd = m_api.horiz ? 0 : 1,
                                fixed = 0,
                                adjust = 0;
            for (size_t ii=0; ii<m_panes.size(); ++ii) {
                paneMins[ii] = m_panes[ii]->getMinSize()[sd];
                paneStrets[ii] = m_panes[ii]->wantStretch()[sd];
                if (paneStrets[ii])
                    adjust += paneMins[ii];
                else
                    fixed += paneMins[ii];
            }
            uint                padBetween = calcPadBetween(paneMins),
                                padTotal = calcPadTotal(padBetween);
            int                 stretSzI = scast<int>(m_size[sd]) - scast<int>(fixed) - scast<int>(padTotal);
            double              stretSize = scast<double>(clampLo(stretSzI,0));
            Vec2I               lo(0),
                                hi(m_size);


            for (size_t ii=0; ii<m_panes.size(); ++ii) {
                if (!paneStrets[ii])
                    hi[sd] = lo[sd] + paneMins[ii];
                else
                    hi[sd] = lo[sd] + round<int>(double(paneMins[ii]) / double(adjust) * stretSize);
                Vec2I    sz = hi-lo;
                m_panes[ii]->moveWindow(m_base+lo,sz);
                lo[sd] = hi[sd] + padBetween;
            }
        }
    }
};

GuiImplPtr
guiGetOsImpl(const GuiSplit & def)
{return GuiImplPtr(new GuiSplitWin(def)); }

}
