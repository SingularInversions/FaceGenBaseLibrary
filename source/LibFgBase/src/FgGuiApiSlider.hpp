//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
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
    Ustring             label;

    GuiTickLabel()
    {}

    GuiTickLabel(double p,Ustring l)
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
    Ustring             label;          // Can be empty
    VecD2               range;
    double              tickSpacing;
    GuiTickLabels       tickLabels;     // Can be empty
    GuiTickLabels       tockLabels;     // ". On other side from ticks
    // Set this to larger values if your tick / tock labels overflow the edges:
    uint                edgePadding = 5;

    virtual
    GuiImplPtr getInstance() {return guiGetOsImpl(*this); }
};

GuiPtr
guiSlider(
    IPT<double>         valN,
    Ustring             label,               // Can be empty
    VecD2               range,
    double              tickSpacing,
    const GuiTickLabels & tl = GuiTickLabels(),
    const GuiTickLabels & ul = GuiTickLabels(),
    uint                edgePadding=5,
    bool                editBox=false);     // Numerical edit box on right, clipped to slider range, 2 fractional digits.

// Create a panel of similar sliders with numbered names:

// Keeping tuple of values as input instead of collating allows for simpler state checks
// (eg. normalization) but then doesn't play well with edit boxes so not such a great idea:
Arr<GuiPtr,3>
guiSliders(
    IPT<Vec3F>          valN,
    const Arr<Ustring,3> & labels,
    VecD2               range,
    double              tickSpacing);

struct  GuiSliders
{
    GuiPtrs             sliders;
    Svec<IPT<double> >  valNs;
    NPT<Doubles>        valsN;          // Collated vals if needed
};

GuiSliders
guiSliders(
    Ustrings const &    labels,
    VecD2               range,
    double              initVal,
    double              tickSpacing,
    Ustring const &     relStore="");   // Relative store name for saving state (null if no save)

// Use to auto create labels for above. Labels will have numbers appended:
Ustrings
numberedLabels(Ustring const & baseLabel,size_t num);

// Generate equispaced tick labels:
GuiTickLabels
guiTickLabels(
    VecD2           range,
    double          spacing,
    double          basePos);

}

#endif
