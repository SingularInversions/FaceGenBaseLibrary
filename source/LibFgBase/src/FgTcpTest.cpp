//
// Copyright (c) 2025 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgSerial.hpp"
#include "FgTcp.hpp"
#include "FgNc.hpp"
#include "FgMain.hpp"

using namespace std;

namespace Fg {

static
bool
handler(
    String const &      ipAddr,
    Bytes const &       dataIn,
    Bytes &             response)
{
    fgout << fgnl << "Message received from " << ipAddr << " : " << bytesToString(dataIn);
    if (dataIn == stringToBytes("Please respond")) {
        response = stringToBytes("Here is the response");
        return true; }
    return false;
}

void
fgTcpServerTest(CLArgs const &)
{
    fgout.setDefOut(true);
    runTcpServer(getNcServerPort(),true,handler,1024);
}

void
fgTcpClientTest(CLArgs const &)
{
    fgout.setDefOut(true);
    Bytes           message = stringToBytes("Please respond"),
                    response;
    runTcpClient_("peano",getNcServerPort(),message,response);
    fgout << fgnl << "Response received: " << bytesToString(response);
    message = stringToBytes("My god, it's full of stars");
    runTcpClient("peano",getNcServerPort(),message);
}

}
