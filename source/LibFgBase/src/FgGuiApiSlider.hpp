//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     March 16, 2011
//

#ifndef FGGUIAPISLIDER_HPP
#define FGGUIAPISLIDER_HPP

#include "FgGuiApiBase.hpp"
#include "FgDepGraph.hpp"
#include "FgMatrixC.hpp"

struct  FgGuiApiTickLabel
{
    double              pos;
    FgString            label;

    FgGuiApiTickLabel()
    {}

    FgGuiApiTickLabel(double p,FgString l)
    : pos(p), label(l)
    {}
};

typedef vector<FgGuiApiTickLabel> FgGuiApiTickLabels;

// Generate equispaced tick labels:
FgGuiApiTickLabels
fgGuiApiTickLabels(
    FgVectD2        range,
    double          spacing,
    double          basePos);

struct FgGuiApiSlider : FgGuiApi<FgGuiApiSlider>
{
    uint                    updateFlagIdx;
    // getInput is required 1. to allow for restoring from serialization and 2. to allow
    // for dependent sliders (eg. FaceGen linear controls). It is also then used for the
    // initialization value:
    boost::function<double(void)>   getInput;
    boost::function<void(double)>   setOutput;
    FgString                label;          // Can be empty
    FgVectD2                range;
    double                  tickSpacing;
    FgGuiApiTickLabels      tickLabels;
    FgGuiApiTickLabels      tockLabels;     // On other side from ticks
    // Set this to larger values if your tick / tock labels overflow the edges:
    uint                    edgePadding;

    FgGuiApiSlider() : edgePadding(5) {}
};

FgGuiPtr
fgGuiSlider(
    FgDgn<double>   val,
    FgString        label,               // Can be empty
    FgVectD2        range,
    double          tickSpacing,
    const FgGuiApiTickLabels & tl = FgGuiApiTickLabels(),
    const FgGuiApiTickLabels & ul = FgGuiApiTickLabels(),
    uint            edgePadding=5,
    bool            editBox=false);     // Numerical edit box on right, clipped to slider range, 2 fractional digits.

// Create a panel of similar sliders with numbered names:

struct  FgGuiSliders
{
    vector<FgGuiPtr>        sliders;
    vector<FgDgn<double> >  inputInds;
    FgDgn<vector<double> >  outputIdx;      // Collated output if you need it
};

FgGuiSliders
fgGuiSliders(
    const FgStrings &       labels,
    FgVectD2                range,
    double                  initVal,
    double                  tickSpacing,
    const FgString &        relStore="");   // Relative store name for saving state (null if no save)

// Use to auto create labels for above. Labels will have numbers appended:
FgStrings
fgNumberedLabels(const FgString & baseLabel,size_t num);

#endif
