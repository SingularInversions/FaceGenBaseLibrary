//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#ifndef FGGUIAPISPLIT_HPP
#define FGGUIAPISPLIT_HPP

#include "FgGuiApiBase.hpp"
#include "FgStdVector.hpp"

namespace Fg {

// This function must be defined in the corresponding OS-specific implementation:
struct  GuiSplit;
GuiImplPtr guiGetOsImpl(GuiSplit const & guiApi);

// Algorithmically proportioned split window with all contents viewable:
struct  GuiSplit : GuiBase
{
    bool                    horiz;
    Svec<GuiPtr>        panes;

    virtual
    GuiImplPtr getInstance() {return guiGetOsImpl(*this); }
};

GuiPtr
guiSplit(bool horiz,const Svec<GuiPtr> & panes);

inline
GuiPtr
guiSplit(bool horiz,GuiPtr p0,GuiPtr p1)
{return guiSplit(horiz,svec(p0,p1)); }

inline
GuiPtr
guiSplit(bool horiz,GuiPtr p0,GuiPtr p1,GuiPtr p2)
{return guiSplit(horiz,svec(p0,p1,p2)); }

inline
GuiPtr
guiSplit(bool horiz,GuiPtr p0,GuiPtr p1,GuiPtr p2,GuiPtr p3)
{return guiSplit(horiz,svec(p0,p1,p2,p3)); }

inline
GuiPtr
guiSplit(bool horiz,GuiPtr p0,GuiPtr p1,GuiPtr p2,GuiPtr p3,GuiPtr p4)
{return guiSplit(horiz,svec(p0,p1,p2,p3,p4)); }

// This function must be defined in the corresponding OS-specific implementation:
struct  GuiSplitAdj;
GuiImplPtr guiGetOsImpl(GuiSplitAdj const & guiApi);

// Adjustable split dual window with central divider:
struct  GuiSplitAdj : GuiBase
{
    bool                    horiz;
    GuiPtr                pane0;
    GuiPtr                pane1;

    GuiSplitAdj(bool h,GuiPtr p0,GuiPtr p1)
        : horiz(h), pane0(p0), pane1(p1)
        {}

    virtual
    GuiImplPtr getInstance() {return guiGetOsImpl(*this); }
};

inline
GuiPtr
guiSplitAdj(bool horiz,GuiPtr p0,GuiPtr p1)
{return std::make_shared<GuiSplitAdj>(horiz,p0,p1); }

// This function must be defined in the corresponding OS-specific implementation:
struct  GuiSplitScroll;
GuiImplPtr guiGetOsImpl(GuiSplitScroll const & guiApi);

// Vertically scrollable split window (panes thickness is fixed to minimum):
struct  GuiSplitScroll : GuiBase
{
    DfgFPtr                 updateFlag;     // Has the panes info been updated ?
    // This function must not depend on the same guigraph node depended on by its children or
    // the windows will be destroyed and recreated with each child update and thus not work:
    std::function<GuiPtrs(void)> getPanes;
    Vec2UI                   minSize;        // Of client area (not including scroll bar)
    uint                        spacing;        // Insert this spacing above each sub-win

    GuiSplitScroll() : minSize(300,300), spacing(0) {}

    virtual
    GuiImplPtr getInstance() {return guiGetOsImpl(*this); }
};

GuiPtr
guiSplitScroll(const GuiPtrs & panes,uint spacing=0);

GuiPtr
guiSplitScroll(std::function<GuiPtrs(void)> getPanes);

GuiPtr
guiSplitScroll(
    const DfgFPtr &                 updateNodeIdx,  // Must be unique to this object
    std::function<GuiPtrs(void)>      getPanes,
    uint                                spacing=0);

}

#endif
