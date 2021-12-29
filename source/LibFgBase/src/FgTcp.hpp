//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// These functions are defined in the OS-specific files (nix) / library (win)

#ifndef FGTCP_HPP
#define FGTCP_HPP

#include "FgStdString.hpp"
#include "FgTypes.hpp"

namespace Fg {

// Returns false if unable to connect to server:
bool            runTcpClient_(
    String const &      hostname,       // DNS or IP
    uint16              port,
    String const &      data,
    String *            response);      // no response expected if nullptr

inline bool     runTcpClient(String const & hostname,uint16 port,String const & data)
{
    return runTcpClient_(hostname,port,data,nullptr);
}

inline bool     runTcpClient_(String const & hostname,uint16 port,String const & data,String & response)
{
    return runTcpClient_(hostname,port,data,&response);
}

typedef std::function<bool        // Return false to terminate server
    (String const &,                // IP Address of the client
     String const &,                // Data from the client
     // Data to be returned to client (ignored if server not supposed to respond). If empty, connection closed:
     String &)>
     TcpHandlerFunc;

void            runTcpServer(
    uint16              port,
    // If true, don't disconnect client until handler returns, then respond. Hander must complete
    // before TCP timeout in this case:
    bool                respond,
    TcpHandlerFunc      handler,
    size_t              maxRecvBytes);  // Maximum number of bytes to receive in incomimg message

}

#endif
