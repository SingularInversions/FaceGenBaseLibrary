//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     April 4, 2011
//

#ifndef FGGUIAPISPLIT_HPP
#define FGGUIAPISPLIT_HPP

#include "FgGuiApiBase.hpp"
#include "FgStdVector.hpp"

// Algorithmically proportioned split window with all contents viewable:
struct  FgGuiApiSplit : FgGuiApi<FgGuiApiSplit>
{
    bool                    horiz;
    vector<FgGuiPtr>        panes;
};

FgGuiPtr
fgGuiSplit(bool horiz,const vector<FgGuiPtr> & panes);

inline
FgGuiPtr
fgGuiSplit(bool horiz,FgGuiPtr p0,FgGuiPtr p1)
{return fgGuiSplit(horiz,fgSvec(p0,p1)); }

inline
FgGuiPtr
fgGuiSplit(bool horiz,FgGuiPtr p0,FgGuiPtr p1,FgGuiPtr p2)
{return fgGuiSplit(horiz,fgSvec(p0,p1,p2)); }

inline
FgGuiPtr
fgGuiSplit(bool horiz,FgGuiPtr p0,FgGuiPtr p1,FgGuiPtr p2,FgGuiPtr p3)
{return fgGuiSplit(horiz,fgSvec(p0,p1,p2,p3)); }

inline
FgGuiPtr
fgGuiSplit(bool horiz,FgGuiPtr p0,FgGuiPtr p1,FgGuiPtr p2,FgGuiPtr p3,FgGuiPtr p4)
{return fgGuiSplit(horiz,fgSvec(p0,p1,p2,p3,p4)); }

// Adjustable split dual window with central divider:
struct  FgGuiApiSplitAdj : FgGuiApi<FgGuiApiSplitAdj>
{
    bool                    horiz;
    FgGuiPtr                pane0;
    FgGuiPtr                pane1;

    FgGuiApiSplitAdj(bool h,FgGuiPtr p0,FgGuiPtr p1)
        : horiz(h), pane0(p0), pane1(p1)
        {}
};

inline
FgGuiPtr
fgGuiSplitAdj(bool horiz,FgGuiPtr p0,FgGuiPtr p1)
{return fgnew<FgGuiApiSplitAdj>(horiz,p0,p1); }

// Vertically scrollable split window (panes thickness is fixed to minimum):
struct  FgGuiApiSplitScroll : FgGuiApi<FgGuiApiSplitScroll>
{
    uint                        updateFlagIdx;  // Has the panes info been updated ?
    // This function must not depend on the same guigraph node depended on by its children or
    // the windows will be destroyed and recreated with each child update and thus not work:
    boost::function<FgGuiPtrs(void)> getPanes;
    FgVect2UI                   minSize;        // Of client area (not including scroll bar)
    uint                        spacing;        // Insert this spacing above each sub-win

    FgGuiApiSplitScroll() : minSize(300,300), spacing(0) {}
};

// Static:

FgGuiPtr
fgGuiSplitScroll(const FgGuiPtrs & panes,uint spacing=0);

FgGuiPtr
fgGuiSplitScroll(boost::function<FgGuiPtrs(void)> getPanes);

FgGuiPtr
fgGuiSplitScroll(
    uint                                updateNodeIdx,  // Must be unique to this object
    boost::function<FgGuiPtrs(void)>    getPanes,
    uint                                spacing=0);

#endif
