//
// Coypright (c) 2022 Singular Inversions Inc. (facegen.com)
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
    string const &  ipAddr,
    string const &  dataIn,
    string &        response)
{
    fgout << fgnl << "Message received from " << ipAddr << " : " << dataIn;
    if (dataIn == "Please respond") {
        response = "Here is the response";
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
    string          message = "Please respond",
                    response;
    runTcpClient_("peano",getNcServerPort(),message,response);
    fgout << fgnl << "Response received: " << response;
    message = "My god, it's full of stars";
    runTcpClient("peano",getNcServerPort(),message);
}

}
