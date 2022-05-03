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

void                FgException::addContext(std::string const & english,std::string const & data)
{
    // TODO: if current language is not english, look up translation for context:
    contexts.emplace_back(english,"",data);
}

string              FgException::tr_message() const
{
    string          ret;
    for (Context const & ctxt : contexts) {
        ret += ctxt.msgEnglish;
        if (!ctxt.dataUtf8.empty())
            ret += " : " + ctxt.dataUtf8;
        if (!ctxt.msgNative.empty())
            ret += "\n" + ctxt.msgNative + " : " + ctxt.dataUtf8;
        ret += "\n";
    }
    return ret;
}

}
