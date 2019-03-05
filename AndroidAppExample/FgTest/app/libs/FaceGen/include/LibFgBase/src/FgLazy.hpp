//
// Copyright (c) 2018 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     18.07.28
//
// Lazy evaluation graph
//
// USE:
//
// * Only use the 'INTERFACE' functions below:
// * Not multi-thread safe.

#ifndef FGLAZY_HPP
#define FGLAZY_HPP

#include "FgStdFunction.hpp"
#include "FgAny.hpp"

using namespace std;

namespace FgLeg {

typedef std::function<void(const FgAny &)>                    FnDestruct;
typedef std::function<void(const FgAnys & ins,FgAny & out)>   FnLink;

// IMPLEMENTATION HELPERS

FgAny
inputImpl(const FgAny & obj);

FgAny
inputImpl(const FgAny & obj,const FnDestruct & fn);

template<class T>
void
dispatchDestruct(const FgAny & obj,const std::function<void(const T &)> & fn)
{fn(obj.as<T>()); }

FgAny &
objectRef(FgAny & node);

FgAny
linkImpl(FgAnys & sources,const FnLink & fn,const FgAny & objEmpty);

const FgAny &
getImpl(FgAny & node);

// INTERFACE:

template<class T>
FgAny
input(const T & obj)
{return inputImpl(FgAny(obj)); }

template<class T>
FgAny
input(const T & obj,const std::function<void(const T &)> & fn)
{return inputImpl(obj,std::bind(dispatchDestruct,std::placeholders::_1,fn)); }

template<class T>
FgAny
link(FgAnys & sources,const FnLink & fn)
{return linkImpl(sources,fn,FgAny(T()));  }

// Input selection implemented at the interface level because update treatment is special:
FgAny
select(
    FgAny &         selNode,        // Must be of type 'size_t'
    FgAnys &        sources);

FgAny
sentinel(const FgAny & srcNode);

// Only works on Inputs:
template<class T>
void
set(FgAny & node,const T & val)
{objectRef(node).ref<T>() = val; }

template<class T>
const T &
get(FgAny & node)       // Arg not const due to possible lazy update.
{return getImpl(node).as<T>(); }

}

#endif
