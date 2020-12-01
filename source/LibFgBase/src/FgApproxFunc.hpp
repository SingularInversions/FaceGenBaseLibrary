//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Simple function approximation using interpolated LUT.
// Possible future optimization: Use Chebyshev approximation.
//

#ifndef FGAPPROXFUNC_HPP
#define FGAPPROXFUNC_HPP

#include "FgStdLibs.hpp"
#include "FgStdExtensions.hpp"
#include "FgTypes.hpp"
#include "FgDiagnostics.hpp"

namespace Fg {

template<class Float>   // 'float' or 'double'
struct  FgApproxFunc
{
    Svec<Float>  m_lut;
    Float               m_lutBase;
    Float               m_lutScale;

    template<class Func>    // 'Float Func::operator()(Float)' defined
    FgApproxFunc(
        Func                func,
        Float               boundLo,
        Float               boundHi,
        uint                lutSize)
        :
        m_lut(lutSize), m_lutBase(boundLo)
    {
        FGASSERT(lutSize > 2);
        Float               delta = boundHi - boundLo;
        FGASSERT(delta > Float(0));
        Float               step = delta / Float(lutSize-1);
        for (uint ii=0; ii<lutSize; ++ii)
            m_lut[ii] = func(boundLo + Float(ii) * step);
        m_lutScale = Float(lutSize-1) / delta;
    }

    // Values outside specified range are clamped:
    Float
    operator()(Float val)
    {
        Float   valLut = m_lutScale * (val - m_lutBase),
                valFlr = std::floor(valLut);
        FGASSERT_FAST(valFlr < Float(std::numeric_limits<int>::max()));
        int     idxLo = int(valFlr),
                idxHi = idxLo + 1;
        Float   wgtLo = Float(idxHi) - valLut,
                wgtHi = Float(1) - wgtLo;
        if (idxLo < 0)
            idxLo = idxHi = 0;
        else if (size_t(idxHi) >= m_lut.size())
            idxHi = idxLo = int(m_lut.size()-1);
        return (wgtLo * m_lut[idxLo] + wgtHi * m_lut[idxHi]);
    }
};

}

#endif
