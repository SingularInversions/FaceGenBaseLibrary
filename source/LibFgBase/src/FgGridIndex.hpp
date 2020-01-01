//
// Copyright (c) 2018 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// 2D grid spatial index

#ifndef FG_GRIDINDEX_HPP
#define FG_GRIDINDEX_HPP

#include "FgImage.hpp"
#include "FgAffineCwC.hpp"

namespace Fg {

template<typename T>
struct  GridIndex
{
    AffineEw2F                clientToGridIpcs;
    Img<Svec<T> >    grid;       // Bins of client objects (bins not exactly square)
    // Return when client request out of bounds. Should be 'const' but VS13 can't handle that:
    Svec<T>              empty;

    // Typically use the number of lookup objects for 'numBins':
    void
    setup(Mat22F clientBounds,uint approxNumBins)
    {
        FGASSERT((approxNumBins > 0) && (approxNumBins < (1 << 20)));   // Sanity check
        Vec2F        clientSz = clientBounds.colVec(1) - clientBounds.colVec(0);
        FGASSERT((clientSz[0]>0) && (clientSz[1]>0));
        double          scaleToBins = std::sqrt(double(approxNumBins)/double(clientSz.cmpntsProduct()));
        Vec2F        gridSizef = clientSz * float(scaleToBins);
        Vec2UI       gridSize = Vec2UI(gridSizef + Vec2F(0.5f));
        gridSize = clampLo(gridSize,1U);
        Mat22F        ipcsBounds(0,gridSize[0],0,gridSize[1]);
        clientToGridIpcs = AffineEw2F(clientBounds,ipcsBounds);
        grid.resize(gridSize);
    }

    void
    add(const T & val,Mat22F clientBounds)
    {
        Mat22F        ipcsBounds = clientToGridIpcs * clientBounds;
        ipcsBounds[0] = cMax(ipcsBounds[0],0.0f);
        ipcsBounds[2] = cMax(ipcsBounds[2],0.0f);
        if ((ipcsBounds[0] > ipcsBounds[1]) || (ipcsBounds[2] > ipcsBounds[3]))
            return;
        Mat22UI       ircsBounds = Mat22UI(ipcsBounds);         // All elements now guaranteed  positive
        ircsBounds[1] = cMin(ircsBounds[1]+1,grid.width());        // Convert to exlusive upper bounds (EUB)
        ircsBounds[3] = cMin(ircsBounds[3]+1,grid.height());       // and clip to grid.
        for (uint yy=ircsBounds[2]; yy<ircsBounds[3]; ++yy) {      // Invalid bounds implicity skipped
            for (uint xx=ircsBounds[0]; xx<ircsBounds[1]; ++xx)
                grid.xy(xx,yy).push_back(val);
        }
    }

    const Svec<T> &
    operator[](const Vec2F & clientPos) const
    {
        Vec2F        posIpcs = clientToGridIpcs*clientPos;
        if ((posIpcs[0] < 0.0f) || (posIpcs[1] < 0.0f))
            return empty;
        Vec2UI       posIrcs = Vec2UI(posIpcs);
        if ((posIrcs[0] < grid.width()) && (posIrcs[1] < grid.height()))
            return grid[posIrcs];
        return empty;
    }
};

}

#endif

// */
