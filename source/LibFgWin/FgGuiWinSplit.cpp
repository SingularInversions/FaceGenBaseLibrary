//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
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
#include "FgIter.hpp"

using namespace std;

namespace Fg {

struct  GuiSplitWin : public GuiBaseImpl
{
    GuiSplit                m_api;
    Img<GuiImplPtr>         m_panes;    // 1-1 with m_api.panes
    String8                 m_store;
    HWND                    m_hwndParent;
    Vec2I                   m_base;     // client area
    Vec2I                   m_size;

    explicit
    GuiSplitWin(GuiSplit const & api) :
        m_api(api)
    {}

    virtual void
    create(HWND parentHwnd,int,String8 const & store,DWORD /*extStyle*/,bool visible)
    {
//fgout << fgnl << "GuiSplitWin::create" << fgpush;
        // Ignore extStyle since this isn't a win32 window and it's not recursively passed.
        m_store = store;
        m_hwndParent = parentHwnd;
        // Since this is not a win32 window, we create the sub-windows here within the parent
        // WM_CREATE handler:
        m_panes.resize(m_api.panes.dims());
        for (size_t ii=0; ii<m_api.panes.numPixels(); ++ii) {
            m_panes[ii] = m_api.panes[ii]->getInstance();
            m_panes[ii]->create(m_hwndParent,int(ii),m_store+"_"+toStr(ii),NULL,visible);
        }
//fgout << fgpop;
    }

    virtual void
    destroy()
    {
        for (GuiImplPtr const & ptr : m_panes.m_data)
            ptr->destroy();
    }

    uint
    calcPadBetween(Uints const & maxDims) const
    {
        uint            maxMin(0);
        if (maxDims.size() > 0)
            maxMin = cMax(maxDims);
        return cMin(8,uint(double(maxMin)*0.1));
    }

    uint
    calcPadTotal(uint numPanes,uint padBetween) const
    {
        uint            gaps = cMax(numPanes,1U) - 1U;
        return padBetween * gaps;
    }

    pair<Uints,Uints>       // Col and Row max lower limit pixel sizes
    cMaxLlimPixCR() const
    {
        Vec2UI              dims = m_panes.dims();
        Img<Vec2UI>         pixLlims {dims,
            mapCallT<Vec2UI,GuiImplPtr>(m_panes.m_data,[](GuiImplPtr const & p){return p->getMinSize();})
        };
        Uints               colMaxWids(pixLlims.width(),0),
                            rowMaxHgts(pixLlims.height(),0);
        for (Iter2UI it(dims); it.valid(); it.next()) {
            Vec2UI              pixSz = pixLlims[it()];
            updateMax_(colMaxWids[it()[0]],pixSz[0]);
            updateMax_(rowMaxHgts[it()[1]],pixSz[1]);
        }
        return make_pair(colMaxWids,rowMaxHgts);
    }

    virtual Vec2UI
    getMinSize() const
    {
        if (m_panes.empty())        // When called before create()
            return Vec2UI(0);
        pair<Uints,Uints>   maxLlimPixCR = cMaxLlimPixCR();
        Vec2UI              between;
        between[0] = calcPadBetween(maxLlimPixCR.first);
        between[1] = calcPadBetween(maxLlimPixCR.second);
        Vec2UI              padding;
        padding[0] = calcPadTotal(m_panes.width(),between[0]);
        padding[1] = calcPadTotal(m_panes.height(),between[1]);
        Vec2UI              minSize {cSum(maxLlimPixCR.first),cSum(maxLlimPixCR.second)};
        return minSize+padding;
    }

    virtual Vec2B
    wantStretch() const
    {
        Vec2B           ret(false,false);
        for (GuiImplPtr const & ptr : m_panes.m_data)
            ret = mapOr(ret,ptr->wantStretch());
        return ret;
    }

    virtual void
    updateIfChanged()
    {
        for (GuiImplPtr const & ptr : m_panes.m_data)
            ptr->updateIfChanged();
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
        for (GuiImplPtr const & ptr : m_panes.m_data)
            ptr->showWindow(s);
    }

    pair<Bools,Bools>       // ColWantStretches, RowWantStretches
    cWantStrets() const
    {
        Vec2UI              dims = m_panes.dims();
        Img<Vec2B>          wants {dims,
            mapCallT<Vec2B,GuiImplPtr>(m_panes.m_data,[](GuiImplPtr const & p){return p->wantStretch();})
        };
        Bools               colWants(dims[0],false),
                            rowWants(dims[1],false);
        for (Iter2UI it(dims); it.valid(); it.next()) {
            Vec2B           want = wants[it()];
            colWants[it()[0]] = colWants[it()[0]] || want[0];
            rowWants[it()[1]] = rowWants[it()[1]] || want[1];
        }
        return make_pair(colWants,rowWants);
    }

    // Returns (offset,size) pairs for the subwindow sizes:
    Vec2UIs
    resizeDimension(Uints const & maxLlimPixs,Bools const & paneStrets,int pixSize)
    {
        size_t              numPanes = maxLlimPixs.size();
        FGASSERT(paneStrets.size() == numPanes);
        uint                fixed = 0,
                            adjust = 0;
        for (size_t ii=0; ii<numPanes; ++ii) {
            if (paneStrets[ii])
                adjust += maxLlimPixs[ii];
            else
                fixed += maxLlimPixs[ii];
        }
        uint                padBetween = calcPadBetween(maxLlimPixs),
                            padTotal = calcPadTotal(uint(numPanes),padBetween);
        int                 stretSzI = pixSize - int(fixed) - int(padTotal);
        double              stretSize = double(cMax(stretSzI,0));
        int                 lo(0),
                            hi = int(numPanes);
        Vec2UIs             ret;
        for (size_t ii=0; ii<numPanes; ++ii) {
            if (!paneStrets[ii])
                hi = lo + maxLlimPixs[ii];
            else
                hi = lo + round<int>(double(maxLlimPixs[ii]) / double(adjust) * stretSize);
            ret.push_back(Vec2UI(lo,hi-lo));
            lo = hi + padBetween;
        }
        return ret;
    }

    void
    resize()
    {
        if (m_size[0] * m_size[1] > 0) {
            pair<Uints,Uints>   maxLlimPixCR = cMaxLlimPixCR();
            pair<Bools,Bools>   wantStretCR = cWantStrets();
            Vec2UIs             colOffsetSizes = resizeDimension(maxLlimPixCR.first,wantStretCR.first,m_size[0]),
                                rowOffsetSizes = resizeDimension(maxLlimPixCR.second,wantStretCR.second,m_size[1]);
            for (Iter2UI it(m_panes.dims()); it.valid(); it.next()) {
                Vec2I           base = m_base;
                uint            col = it()[0],
                                row = it()[1];
                Vec2UI          colOffsetSize = colOffsetSizes[col],
                                rowOffsetSize = rowOffsetSizes[row];
                base[0] += int(colOffsetSize[0]);
                base[1] += int(rowOffsetSize[0]);
                Vec2UI          sz {colOffsetSize[1],rowOffsetSize[1]};
                m_panes[it()]->moveWindow(base,Vec2I(sz));
            }
        }
    }
};

GuiImplPtr
guiGetOsImpl(GuiSplit const & def)
{return GuiImplPtr(new GuiSplitWin(def)); }

}
