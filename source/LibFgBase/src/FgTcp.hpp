//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Nov 1, 2011
//
// These functions are defined in the OS-specific library

#ifndef FGTCP_HPP
#define FGTCP_HPP

#include "FgStdString.hpp"
#include "FgTypes.hpp"

// Returns false if unable to connect to server:
bool
fgTcpClient(
    const string &      hostname,
    uint16              port,
    const string &      data,
    bool                getResponse,
    string &            response);

inline
bool
fgTcpClient(
    const string &      hostname,
    uint16              port,
    const string &      data)
{
    string     dummy;
    return fgTcpClient(hostname,port,data,false,dummy);
}

inline
bool
fgTcpClient(
    const string &      hostname,
    uint16              port,
    const string &      data,
    string &            response)
{
    return fgTcpClient(hostname,port,data,true,response);
}

typedef boost::function<bool        // Return false to terminate server
    (const string &,                // IP Address of the client
     const string &,                // Data from the client
     string &)>                     // Data to be returned to client (ignored if server not supposed to respond)
     FgFuncTcpHandler;

void
fgTcpServer(
    uint16              port,
    // If true, don't disconnect client until handler returns, then respond. Hander must complete
    // before TCP timeout in this case:
    bool                respond,
    FgFuncTcpHandler    handler,
    size_t              maxRecvBytes);  // Maximum number of bytes to receive in incomimg message

#endif
