//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Jan 11, 2012
//

#ifndef FGCLUSTER_HPP
#define FGCLUSTER_HPP

#include "FgTypes.hpp"
#include "FgDefaultVal.hpp"

enum    FgClusterRole
{
    CLUSTER_STANDALONE = 0,
    CLUSTER_MASTER = 1,
    CLUSTER_SLAVE = 2
};

extern FgClusterRole    fgClusterContext;
extern std::string      fgClusterCommand;
extern bool             fgClusterAws;
extern uint             fgClusterThreads;   // Sets depgraph thread number across cluster if > 0

#endif

// */
