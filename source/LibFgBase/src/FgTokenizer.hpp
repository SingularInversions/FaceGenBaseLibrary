//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Andrew Beatty
// Date: Dec 27, 2011
//

#ifndef FGTOKENIZER_HPP
#define FGTOKENIZER_HPP

inline
bool
fgIsDigit(char cc)
{return ((cc >= '0') && (cc <= '9')); }

inline
bool
fgIsDigitOrMinus(char cc)
{return ((fgIsDigit(cc)) || (cc == '-')); }

inline
uint
fgAsciiToDigit(char cc)
{
    uint    ret = uint(cc) - uint('0');
    FGASSERT(ret < 10);
    return ret;
}

#endif
