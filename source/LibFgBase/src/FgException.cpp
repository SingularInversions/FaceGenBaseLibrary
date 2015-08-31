//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Jan 27, 2007
//

#include "stdafx.h"

#include "FgException.hpp"
#include "FgStdString.hpp"

const char * newline = "\n";

using namespace std;

FgString
FgException::Context::trans() const
{
    if (data.empty())
        return fgTr(msg);
    else
        return fgTr(msg)+" : "+data;
}

FgString
FgException::Context::noTrans() const
{
    if (data.empty())
        return FgString(msg);
    else
        return FgString(msg)+" : "+data;
}

FgString
FgException::tr_message() const
{
    if (m_ct.empty())
        return FgString();
    size_t      last = m_ct.size()-1;
    FgString    ret = m_ct[last].trans();
    for (size_t ii=1; ii<m_ct.size(); ++ii)
    {
        ret += newline;
        ret += m_ct[last-ii].trans();
    }
    return ret;
}

FgString
FgException::no_tr_message() const
{
    if (m_ct.empty())
        return FgString();
    size_t      last = m_ct.size()-1;
    FgString    ret = m_ct[last].noTrans();
    for (size_t ii=1; ii<m_ct.size(); ++ii)
    {
        ret += newline;
        ret += m_ct[last-ii].noTrans();
    }
    return ret;
}
