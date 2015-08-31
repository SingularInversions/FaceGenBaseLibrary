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
    FgString                label;
    FgVectD2                range;
    double                  tickSpacing;
    FgGuiApiTickLabels      tickLabels;
    FgGuiApiTickLabels      tockLabels;     // On other side from ticks
};

FgGuiPtr
fgGuiSlider(
    FgDgn<double>   val,
    uint            updateFlagIdx,
    FgString        label,
    FgVectD2        range,
    double          tickSpacing,
    const FgGuiApiTickLabels & tl = FgGuiApiTickLabels(),
    const FgGuiApiTickLabels & ul = FgGuiApiTickLabels());

FgGuiPtr
fgGuiSlider(
    FgDgn<double>   val,
    FgString        label,
    FgVectD2        range,
    double          tickSpacing,
    const FgGuiApiTickLabels & tl = FgGuiApiTickLabels(),
    const FgGuiApiTickLabels & ul = FgGuiApiTickLabels());

// Create a panel of similar sliders with numbered names:

struct  FgGuiSliders
{
    vector<FgGuiPtr>        sliders;
    FgDgn<vector<double> >  outputIdx;
    vector<FgDgn<double> >  inputInds;
};

FgGuiSliders
fgGuiSliders(
    uint                    numSliders,
    FgString                baseLabel,
    FgVectD2                range,
    double                  initVal,
    double                  tickSpacing);

#endif
