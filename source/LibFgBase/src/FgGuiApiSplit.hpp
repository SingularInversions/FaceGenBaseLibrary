//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#ifndef FGGUIAPISPLIT_HPP
#define FGGUIAPISPLIT_HPP

#include "FgGuiApiBase.hpp"
#include "FgImageBase.hpp"

namespace Fg {

// This function must be defined in the corresponding OS-specific implementation:
struct  GuiSplit;
GuiImplPtr          guiGetOsImpl(GuiSplit const & guiApi);

// Algorithmically proportioned split window with all contents viewable:
struct  GuiSplit : GuiBase
{
    Img<GuiPtr>         panes;

    virtual
    GuiImplPtr getInstance() {return guiGetOsImpl(*this); }
};

// Checks for case of 1x1 image and just returns the single GuiPtr in that case:
GuiPtr              guiSplit(Img<GuiPtr> const & panes);
// Horizontal array of panes:
inline GuiPtr       guiSplitH(GuiPtrs const & panes) {return guiSplit(Img<GuiPtr>{panes.size(),1,panes}); }
// Vertical array of panes:
inline GuiPtr       guiSplitV(GuiPtrs const & panes) {return guiSplit(Img<GuiPtr>{1,panes.size(),panes}); }

// This function must be defined in the corresponding OS-specific implementation:
struct  GuiSplitAdj;
GuiImplPtr          guiGetOsImpl(GuiSplitAdj const & guiApi);

// Adjustable split dual window with central divider:
struct  GuiSplitAdj : GuiBase
{
    bool                horiz;
    GuiPtr              pane0;
    GuiPtr              pane1;

    GuiSplitAdj(bool h,GuiPtr p0,GuiPtr p1)
        : horiz(h), pane0(p0), pane1(p1)
        {}

    virtual
    GuiImplPtr getInstance() {return guiGetOsImpl(*this); }
};

inline GuiPtr       guiSplitAdj(bool horiz,GuiPtr p0,GuiPtr p1) {return std::make_shared<GuiSplitAdj>(horiz,p0,p1); }

// This function must be defined in the corresponding OS-specific implementation:
struct  GuiSplitScroll;
GuiImplPtr          guiGetOsImpl(GuiSplitScroll const & guiApi);

// Vertically scrollable split window (panes thickness is fixed to minimum):
struct  GuiSplitScroll : GuiBase
{
    DfgFPtr                 updateFlag;     // Has the panes info been updated ?
    // This function must not depend on the same guigraph node depended on by its children or
    // the windows will be destroyed and recreated with each child update and thus not work:
    Sfun<GuiPtrs(void)>     getPanes;
    Vec2UI                  minSize;        // Of client area (not including scroll bar)
    uint                    spacing;        // Insert this spacing above each sub-win

    GuiSplitScroll() : minSize(300,300), spacing(0) {}

    virtual GuiImplPtr      getInstance() {return guiGetOsImpl(*this); }
};

GuiPtr              guiSplitScroll(GuiPtrs const & panes,uint spacing=0);
GuiPtr              guiSplitScroll(Sfun<GuiPtrs(void)> const & getPanes);

GuiPtr
guiSplitScroll(
    DfgFPtr const &                 updateNodeIdx,  // Must be unique to this object
    Sfun<GuiPtrs(void)> const &     getPanes,
    uint                            spacing=0);

}

#endif
