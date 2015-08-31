//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     March 26, 2011
//

#ifndef FGGUIAPIGROUPBOX_HPP
#define FGGUIAPIGROUPBOX_HPP

#include "FgGuiApiBase.hpp"

struct
FgGuiApiGroupbox : FgGuiApi<FgGuiApiGroupbox>
{
    FgGuiApiGroupbox(const FgString & l,FgGuiPtr c)
    : label(l), contents(c)
    {}

    FgString        label;
    FgGuiPtr     contents;
};

inline
FgGuiPtr
fgGuiGroupboxTr(const std::string & label,FgGuiPtr p)
{return fgnew<FgGuiApiGroupbox>(fgTr(label),p); }

inline
FgGuiPtr
fgGuiGroupbox(const FgString & label,FgGuiPtr p)
{return fgnew<FgGuiApiGroupbox>(label,p); }

#endif
