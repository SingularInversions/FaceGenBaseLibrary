//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     March 22, 2011
//

#include "stdafx.h"

#include "FgGuiApiSplit.hpp"
#include "FgGuiWin.hpp"
#include "FgThrowWindows.hpp"
#include "FgMatrixC.hpp"
#include "FgBounds.hpp"
#include "FgDefaultVal.hpp"
#include "FgMetaFormat.hpp"

using namespace std;

struct  FgGuiWinSplit : public FgGuiOsBase
{
    FgGuiApiSplit               m_api;
    FgGuiOsPtrs                 m_panes;
    FgString                    m_store;
    HWND                        m_hwndParent;
    FgVect2I                    m_base;     // client area
    FgVect2I                    m_size;
    // Cache the return values of sub-window getMinSize() calls to avoid exponentiating:
    mutable vector<FgVect2UI>   m_minSizes;

    FgGuiWinSplit(const FgGuiApiSplit & api) :
        m_api(api)
    {}

    virtual void
    create(HWND parentHwnd,int,const FgString & store,DWORD /*extStyle*/,bool visible)
    {
//fgout << fgnl << "FgGuiWinSplit::create" << fgpush;
        // Ignore extStyle since this isn't a win32 window and it's not recursively passed.
        m_store = store;
        m_hwndParent = parentHwnd;
        // Since this is not a win32 window, we create the sub-windows here within the parent
        // WM_CREATE handler:
        m_panes.resize(m_api.panes.size());
        for (size_t ii=0; ii<m_api.panes.size(); ++ii) {
            m_panes[ii] = m_api.panes[ii]->getInstance();
            m_panes[ii]->create(m_hwndParent,int(ii),m_store+"_"+fgToString(ii),NULL,visible);
        }
//fgout << fgpop;
    }

    virtual void
    destroy()
    {
        for (size_t ii=0; ii<m_panes.size(); ++ii)
            m_panes[ii]->destroy();
    }

    void
    minSizes() const
    {
        m_minSizes.resize(m_panes.size());
        for (size_t ii=0; ii<m_panes.size(); ++ii)
            m_minSizes[ii] = m_panes[ii]->getMinSize();
    }

    virtual FgVect2UI
    getMinSize() const
    {
        minSizes();         // Refresh cache
        uint        sd = splitDim(),
                    nd = 1 - sd,
                    sdSum = 0,
                    ndMax = 0;
        for (size_t ii=0; ii<m_panes.size(); ++ii) {
            FgVect2UI   size = m_minSizes[ii];
            sdSum += size[sd];
            ndMax = fgMax(ndMax,size[nd]);
        }
        FgVect2UI   ret;
        ret[sd] = sdSum + padTotal();
        ret[nd] = ndMax;
        return ret;
    }
        // Here's how it would look for a grid layout ... but resizing more complex:
        //FgVect2UI           gd = m_api.grid;
        //vector<uint>        maxMinWid(gd[1],0),
        //                    maxMinHgt(gd[0],0);
        //for (uint yy=0; yy<gd[1]; ++yy) {
        //    for (uint xx=0; xx<gd[0]; ++xx) {
        //        FgVect2UI   minSize = m_panes[yy*gd[0]+xx]->getMinSize();
        //        maxMinWid[yy] = fgMax(maxMinWid[yy],minSize[0]);
        //        maxMinHgt[xx] = fgMax(maxMinHgt[xx],minSize[1]);
        //    }
        //}
        //FgVect2UI           minSize = FgVect2UI(fgSum(maxMinWid),fgSum(maxMinHgt));
        //FgVect2UI           padding = (m_api.grid-FgVect2UI(1)) * 8;
        //return minSize+padding;


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
        for (size_t ii=0; ii<m_panes.size(); ++ii)
            m_panes[ii]->updateIfChanged();
    }

    virtual void
    moveWindow(FgVect2I base,FgVect2I size)
    {
//fgout << fgnl << "FgGuiWinSplit::moveWindow " << base << " , " << size << fgpush;
        m_base = base;
        m_size = size;
        minSizes();
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
            vector<uint>        mins(m_panes.size());
            vector<bool>        strets(m_panes.size());
            uint                sd = splitDim(),
                                fixed = 0,
                                adjust = 0;
            for (size_t ii=0; ii<m_panes.size(); ++ii) {
                mins[ii] = m_minSizes[ii][sd];
                strets[ii] = m_panes[ii]->wantStretch()[sd];
                if (strets[ii])
                    adjust += mins[ii];
                else
                    fixed += mins[ii];
            }
            double              stretSize = double(m_size[sd] - fixed - padTotal());
            FgVect2I            lo(0),hi(m_size);
            uint                pb = padBetween();
            for (size_t ii=0; ii<m_panes.size(); ++ii) {
                if (!strets[ii])
                    hi[sd] = lo[sd] + mins[ii];
                else
                    hi[sd] = lo[sd] + fgRound(double(mins[ii]) / double(adjust) * stretSize);
                FgVect2I    sz = hi-lo;
                m_panes[ii]->moveWindow(m_base+lo,sz);
                lo[sd] = hi[sd] + pb;
            }
        }
    }

    uint
    splitDim() const
    {return m_api.horiz ? 0 : 1; }

    uint
    padBetween() const
    {
        uint        sd = splitDim(),
                    sdMax = 0;
        for (size_t ii=0; ii<m_panes.size(); ++ii)
            sdMax = fgMax(sdMax,m_minSizes[ii][sd]);
        return fgMin(8,int(0.1 * sdMax));
    }

    uint
    padTotal() const
    {return (padBetween() * (uint(m_panes.size())-1)); }
};

FgSharedPtr<FgGuiOsBase>
fgGuiGetOsInstance(const FgGuiApiSplit & def)
{return FgSharedPtr<FgGuiOsBase>(new FgGuiWinSplit(def)); }
