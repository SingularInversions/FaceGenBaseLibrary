//
// Copyright (c) 2015 Singular Inversions Inc.
//
// Authors:     Andrew Beatty
// Created:     Jan 11, 2012
//
// Compute clustering for synchronous batch jobs.
//
// Uses boost asio but is defined in platform-specific libs due to header & define requirements.

#ifndef FGCLUSTER_HPP
#define FGCLUSTER_HPP

#include "FgStdVector.hpp"
#include "FgStdString.hpp"

typedef boost::function<string(const string &)>     FgFuncStr2Str;

// Serves a single client synchronously until client shuts connection.
void
fgClustWorker(
    uint16              port,
    FgFuncStr2Str       handler,
    size_t              maxRecvBytes);  // Max bytes to receive in incomimg message (for safety)

struct  FgClustDispatcher
{
    virtual ~FgClustDispatcher() {};

    // Dispatches outgoing messages to all workers, receives all responses, then returns:
    virtual void batchProcess(
        const FgStrs &  msgsSend,   // Messages serialized to byte strings by client
        // Worker-serialized responses, unless an error happened in which case it's the error description:
        FgStrs &        msgsRecv) = 0;
};

// Must be called after 'fgClustWorker' has been called on worker machines:
std::shared_ptr<FgClustDispatcher>
fgClustDispatcher(
    const FgStrs &      hostnames,      // DNS or IP
    uint16              port,
    size_t              maxRecvBytes);

#endif

// */
