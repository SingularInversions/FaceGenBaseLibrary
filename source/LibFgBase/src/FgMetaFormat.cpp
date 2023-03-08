//
// Copyright (c) 2022 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"
#include "FgMetaFormat.hpp"
#include "FgSerial.hpp"
#include "FgTestUtils.hpp"
#include "FgCommand.hpp"

using namespace std;

namespace Fg {

void                testMetaFormat(CLArgs const &)
{
    TestDir             td("MetaFormat");
    int                 a = 42;
    saveMessage(a,"test.fgbin");
    int                 b;
    loadMessage_("test.fgbin",b);
    FGASSERT(a == b);
}

}

// */
