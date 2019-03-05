//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     March 30, 2010
//
// Make a POD initialize to a default value.
// Safer than initializers in composing structure.

#ifndef FGDEFAULTVAL_HPP
#define FGDEFAULTVAL_HPP

#include "FgSerialize.hpp"

template<typename Type,Type Default>
struct  FgDefaultVal
{
    Type           val;

    FG_SERIALIZE1(val)

    FgDefaultVal() : val(Default) {}

    FgDefaultVal(const Type & v) : val(v) {}

    operator Type() const
    {return val; }
};

typedef FgDefaultVal<bool,false>    FgBoolF;
typedef FgDefaultVal<bool,true>     FgBoolT;

#endif
