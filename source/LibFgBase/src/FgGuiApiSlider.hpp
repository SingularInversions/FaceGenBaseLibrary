//
// Coypright (c) 2022 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#ifndef FGGUIAPISLIDER_HPP
#define FGGUIAPISLIDER_HPP

#include "FgGuiApiBase.hpp"
#include "FgMatrixC.hpp"

namespace Fg {

struct  GuiTickLabel
{
    double              pos;
    String8             label;

    GuiTickLabel()
    {}

    GuiTickLabel(double p,String8 l)
    : pos(p), label(l)
    {}
};

typedef Svec<GuiTickLabel> GuiTickLabels;

// This function must be defined in the corresponding OS-specific implementation:
struct  GuiSlider;
GuiImplPtr guiGetOsImpl(GuiSlider const & guiApi);

struct GuiSlider : GuiBase
{
    DfgFPtr             updateFlag;
    // getInput is required 1. to allow for restoring from serialization and 2. to allow
    // for dependent sliders (eg. FaceGen linear controls). It is also then used for the
    // initialization value:
    Sfun<double(void)>  getInput;
    Sfun<void(double)>  setOutput;
    String8             label;          // Can be empty
    VecD2               range;
    double              tickSpacing;
    GuiTickLabels       tickLabels;     // Can be empty
    GuiTickLabels       tockLabels;     // ". On other side from ticks
    // Set this to larger values if your tick / tock labels overflow the edges:
    uint                edgePadding = 5;

    virtual
    GuiImplPtr getInstance() {return guiGetOsImpl(*this); }
};
typedef Svec<GuiSlider> GuiSliders;

GuiPtr
guiSlider(
    IPT<double>         valN,
    String8             label,               // Can be empty
    VecD2               range,
    double              tickSpacing,
    GuiTickLabels const & tl = GuiTickLabels(),
    GuiTickLabels const & ul = GuiTickLabels(),
    uint                edgePadding=5,
    bool                editBox=false);     // Numerical edit box on right, clipped to slider range, 2 fractional digits.

// Array of panes with labels on left, sliders on right:
GuiPtr
guiSliderBank(
    Svec<IPT<double> > const & valNs,
    String8s const &        labels,     // Must be same size as valNs
    VecD2                   range,
    double                  tickSpacing,
    GuiTickLabels const &   tickLabels=GuiTickLabels{});

// Use to auto create labels for above. Labels will have numbers appended:
String8s
numberedLabels(String8 const & baseLabel,size_t num);

// Generate equispaced tick labels:
GuiTickLabels
guiTickLabels(
    VecD2           range,
    double          spacing,
    double          basePos);

}

#endif
