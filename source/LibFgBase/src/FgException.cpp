//
// Coypright (c) 2022 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgException.hpp"
#include "FgStdString.hpp"
#include "FgString.hpp"

using namespace std;

namespace Fg {

string
FgException::Context::trans() const
{
    if (dataUtf8.empty())
        return fgTr(msg).m_str;
    else
        return (fgTr(msg) + " : " + dataUtf8).m_str;
}

string
FgException::Context::noTrans() const
{
    if (dataUtf8.empty())
        return msg;
    else
        return msg + " : " + dataUtf8;
}

string
FgException::tr_message() const
{
    Strings          strs;
    // Translate and reverse order so most proximal message comes first:
    for (auto it=m_ct.rbegin(); it!=m_ct.rend(); ++it)
        strs.push_back(it->trans());
    return cat(strs,"\n");
}

string
FgException::no_tr_message() const
{
    Strings          strs;
    // Translate and reverse order so most proximal message comes first:
    for (auto it=m_ct.rbegin(); it!=m_ct.rend(); ++it)
        strs.push_back(it->noTrans());
    return cat(strs,"\n");
}

}
