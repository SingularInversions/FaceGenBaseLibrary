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

#include "FgStdLibs.hpp"
#include "FgTypes.hpp"

// Returns false if unable to connect to server:
bool
fgTcpClient(
    const std::string & hostname,
    uint16              port,
    const std::string & data,
    bool                getResponse,
    std::string &       response);

inline
bool
fgTcpClient(
    const std::string & hostname,
    uint16              port,
    const std::string & data)
{
    std::string     dummy;
    return fgTcpClient(hostname,port,data,false,dummy);
}

inline
bool
fgTcpClient(
    const std::string & hostname,
    uint16              port,
    const std::string & data,
    std::string &       response)
{
    return fgTcpClient(hostname,port,data,true,response);
}

void
fgTcpServer(
    uint16      port,
    bool        respond,                // Don't disconnect client until handler returns, then respond.
    bool(*handler)(                     // Return false to terminate server
        const std::string & ipAddr,
        const std::string & dataIn,
        std::string &       response),  // No response sent if left empty or if waitForResponse=false
    size_t      maxRecvBytes);          // Maximum number of bytes to receive in incomimg message

#endif
