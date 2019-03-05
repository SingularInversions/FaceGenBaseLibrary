//
// Copyright (c) 2015 Singular Inversions Inc.
//
// Authors:     Andrew Beatty
// Created:     May 31, 2017
//
// Compute clustering for synchronous batch jobs.
//
// Uses boost asio but those parts are defined in platform-specific libs due to header & define requirements.

#ifndef FGCLUSTER_HPP
#define FGCLUSTER_HPP

#include "FgStdVector.hpp"
#include "FgStdString.hpp"
#include "FgStdFunction.hpp"
#include "FgString.hpp"

// Only one fgClusterPortDefault is needed because we can have simultaneous TCP connections with all workers since:
// 1. Set keepalive option (defaults to 2 hour interval) and works on non-proxy connection
// 2. Connections defined by 2 IPs and 2 fgClusterPortDefault number tuple, so unique.
// This will keep things simpler than having 2-way connection architecture.
inline
uint16
fgClusterPortDefault() {return 59407; }

// Serves a single client synchronously until client shuts connection.
void
fgClustWorker(
    FgFuncStr2Str       handler,       // Must do it's own deserialization/serialization
    uint16              port=fgClusterPortDefault());

struct  FgClustDispatcher
{
    virtual ~FgClustDispatcher() {};

    virtual size_t numMachines() const = 0;

    // Dispatches outgoing messages to all workers, receives all responses, then returns:
    virtual void batchProcess(
        const FgStrs &  msgsSend,   // Messages serialized to byte strings by client
        // Worker-serialized responses, unless an error happened in which case it's the error description:
        FgStrs &        msgsRecv) const = 0;
};

// Must be called after 'fgClustWorker' has been called on worker machines:
std::shared_ptr<FgClustDispatcher>
fgClustDispatcher(
    const FgStrs &      hostnames,      // DNS or IP
    uint16              port=fgClusterPortDefault());

typedef std::function<void(const FgClustDispatcher *)>    FgFuncCrdntor;

// Deploys Ubuntu version to LAN:
void
fgClusterDeploy(
    const string &          name,       // Name of this run (for file logging)
    const FgFuncCrdntor &   crdntor,    // Coordinates cluster work and returns on completion of all work
    const FgFuncStr2Str &   worker,     // Cluster worker function
    const string &          coordIP,    // IP address (or domain name) of coordinator
    const FgStrs &          workIPs,    // IP addresses (or domain names) of workers
    const FgStrings &       files);     // Data files to sync before deployment (relative to data directory)

// Handy for passing the current role through functions (none means no clustering)
enum class FgClusterRole { none, deploy, crdntor, worker };

#endif

// */
