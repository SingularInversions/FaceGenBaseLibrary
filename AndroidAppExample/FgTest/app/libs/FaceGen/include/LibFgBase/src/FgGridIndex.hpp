//
// Copyright (c) 2018 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     18.14.23
//
// 2D grid spatial index

#ifndef FG_GRIDINDEX_HPP
#define FG_GRIDINDEX_HPP

#include "FgImage.hpp"
#include "FgAffineCwC.hpp"

template<typename T>
struct  FgGridIndex
{
    FgAffineCw2F                clientToGridIpcs;
    FgImage<std::vector<T> >    grid;       // Bins of client objects (bins not exactly square)
    // Return when client request out of bounds. Should be 'const' but VS13 can't handle that:
    std::vector<T>              empty;

    // Typically use the number of lookup objects for 'numBins':
    void
    setup(FgMat22F clientBounds,uint approxNumBins)
    {
        FGASSERT((approxNumBins > 0) && (approxNumBins < (1 << 20)));   // Sanity check
        FgVect2F        clientSz = clientBounds.colVec(1) - clientBounds.colVec(0);
        FGASSERT((clientSz[0]>0) && (clientSz[1]>0));
        double          scaleToBins = std::sqrt(double(approxNumBins)/double(clientSz.cmpntsProduct()));
        FgVect2F        gridSizef = clientSz * float(scaleToBins);
        FgVect2UI       gridSize = FgVect2UI(gridSizef + FgVect2F(0.5f));
        gridSize = fgClipElemsLo(gridSize,1U);
        FgMat22F        ipcsBounds(0,gridSize[0],0,gridSize[1]);
        clientToGridIpcs = FgAffineCw2F(clientBounds,ipcsBounds);
        grid.resize(gridSize);
    }

    void
    add(const T & val,FgMat22F clientBounds)
    {
        FgMat22F        ipcsBounds = clientToGridIpcs * clientBounds;
        ipcsBounds[0] = fgMax(ipcsBounds[0],0.0f);
        ipcsBounds[2] = fgMax(ipcsBounds[2],0.0f);
        if ((ipcsBounds[0] > ipcsBounds[1]) || (ipcsBounds[2] > ipcsBounds[3]))
            return;
        FgMat22UI       ircsBounds = FgMat22UI(ipcsBounds);         // All elements now guaranteed  positive
        ircsBounds[1] = fgMin(ircsBounds[1]+1,grid.width());        // Convert to exlusive upper bounds (EUB)
        ircsBounds[3] = fgMin(ircsBounds[3]+1,grid.height());       // and clip to grid.
        for (uint yy=ircsBounds[2]; yy<ircsBounds[3]; ++yy) {      // Invalid bounds implicity skipped
            for (uint xx=ircsBounds[0]; xx<ircsBounds[1]; ++xx)
                grid.xy(xx,yy).push_back(val);
        }
    }

    const std::vector<T> &
    operator[](const FgVect2F & clientPos) const
    {
        FgVect2F        posIpcs = clientToGridIpcs*clientPos;
        if ((posIpcs[0] < 0.0f) || (posIpcs[1] < 0.0f))
            return empty;
        FgVect2UI       posIrcs = FgVect2UI(posIpcs);
        if ((posIrcs[0] < grid.width()) && (posIrcs[1] < grid.height()))
            return grid[posIrcs];
        return empty;
    }
};

#endif

// */
