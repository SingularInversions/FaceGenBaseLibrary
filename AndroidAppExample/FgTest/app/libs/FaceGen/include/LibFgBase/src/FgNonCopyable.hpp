//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:Sohail Somani
//

#ifndef INCLUDED_FGNONCOPYABLE_HPP
#define INCLUDED_FGNONCOPYABLE_HPP

struct FgNonCopyable
{
protected:
    FgNonCopyable(){}
    ~FgNonCopyable(){}
private:
    FgNonCopyable(FgNonCopyable const &);
    FgNonCopyable & operator=(FgNonCopyable const &);
};

#endif
