//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgOut.hpp"
#include "FgTcp.hpp"
#include "FgNc.hpp"
#include "FgMain.hpp"

using namespace std;

namespace Fg {

static
bool
handler(
    const string &  ipAddr,
    const string &  dataIn,
    string &        response)
{
    fgout << fgnl << "Message received from " << ipAddr << " : " << dataIn;
    if (dataIn == "Please respond") {
        response = "Here is the response";
        return true; }
    return false;
}

void
fgTcpServerTest(const CLArgs &)
{
    fgout.setDefOut(true);
    fgTcpServer(fgNcServerPort(),true,handler,1024);
}

void
fgTcpClientTest(const CLArgs &)
{
    fgout.setDefOut(true);
    string          message = "Please respond",
                    response;
    fgTcpClient("peano",fgNcServerPort(),message,response);
    fgout << fgnl << "Response received: " << response;
    message = "My god, it's full of stars";
    fgTcpClient("peano",fgNcServerPort(),message);
}

}
