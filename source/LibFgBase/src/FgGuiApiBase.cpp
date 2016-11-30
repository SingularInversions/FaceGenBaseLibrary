//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Andrew Beatty
// Created: June 24, 2014
//

#include "stdafx.h"
#include "FgGuiApiBase.hpp"

using namespace std;

FgGuiDiagHandler        g_guiDiagHandler;
FgGuiGraph              g_gg;

void
FgGuiGraph::setInputsToDefault(uint nodeIdx)
{
    typedef FgLinkGraph<FgDepNode,FgLink> LG;
    const LG &          lg = dg.linkGraph();
    vector<bool>        nodesTouched(lg.numNodes()),
                        linksTouched(lg.numLinks());
    fgTraverseUp(lg,nodeIdx,nodesTouched,linksTouched);
    for (size_t ii=0; ii<nodesTouched.size(); ++ii) {
        if (nodesTouched[ii]) {
            const LG::Node &   node = lg.m_nodes[ii];
            if (!node.incomingLink.valid()) {
                for (size_t jj=0; jj<m_inputSaves.size(); ++jj) {
                    if (m_inputSaves[jj].nodeIdx == ii)
                        dg.setNode(uint(ii),m_inputSaves[jj].defaultVal);
                }
            }
        }
    }

}

// */
