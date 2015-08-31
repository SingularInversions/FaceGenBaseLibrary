//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Andrew Beatty
// Created: Jan 17, 2012
//

#include "stdafx.h"
#include "FgCluster.hpp"

using namespace std;

FgClusterRole   fgClusterContext = CLUSTER_STANDALONE;
string          fgClusterCommand;
bool            fgClusterAws = false;
uint            fgClusterThreads = 0;
