//
// Copyright (c) 2025 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// These functions are defined in the OS-specific files (nix) / library (win)

#ifndef FGTCP_HPP
#define FGTCP_HPP

#include "FgSerial.hpp"

namespace Fg {

// Returns false if unable to connect to server:
bool            runTcpClient_(
    String const &      hostname,           // DNS or IP
    uint16              port,
    Bytes const &       message,
    Bytes *             responsePtr);       // no response expected if nullptr

inline bool     runTcpClient(String const & hostname,uint16 port,Bytes const & data)
{
    return runTcpClient_(hostname,port,data,nullptr);
}

inline bool     runTcpClient_(String const & hostname,uint16 port,Bytes const & data,Bytes & response)
{
    return runTcpClient_(hostname,port,data,&response);
}

typedef Sfun<bool               // return false to terminate server
    (String const &,            // IP Address of the client
     Bytes const &,             // message from the client
     Bytes &)>                  // response to send back to client if 'respond' below is true. If empty, connection will be closed.
     TcpHandlerFunc;

void            runTcpServer(
    uint16              port,
    // If true, don't disconnect client until handler returns, then respond only if the handler has returned a non-zero
    // size message. Hander must complete before TCP timeout if it does generate a response.
    bool                respond,
    TcpHandlerFunc      handler_,
    size_t              maxRecvBytes);  // Maximum number of bytes to receive in incomimg message

}

#endif
