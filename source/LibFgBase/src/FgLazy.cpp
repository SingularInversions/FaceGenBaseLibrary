//
// Copyright (c) 2018 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Andrew Beatty
// Created: 18.07.28
//

#include "stdafx.h"

#include "FgLazy.hpp"
#include "FgString.hpp"
#include "FgDiagnostics.hpp"

using namespace std;

namespace FgLeg {

// User-settable input node. Cannot be used as a sink.
struct  Input
{
    FgAny                   object;         // User data object
    FgAnyWeaks              sinks;          // Sink nodes
    FnDestruct              onDestruct;     // Generally used to save state to file

    explicit Input(const FgAny & o) : object(o) {}

    Input(const FgAny & o,const FnDestruct & f) : object(o), onDestruct(f) {}

    ~Input()
    {
        if (onDestruct)
            onDestruct(object);
    }
};

struct  Link
{
    FgAny                   object;             // User data object
    FgAnys                  sources;            // Source nodes
    FgAnyWeaks              sinks;              // Sink nodes
    FnLink                  func;
    bool                    updated = false;    // Re-compute only if false
};

struct  Select
{
    FgAny                   index;
    FgAnys                  sources;
    FgAnyWeaks              sinks;
    bool                    updated = false;
};

// A singly owned node that tracks updates for the owner. Cannot be used as a source.
struct  Sentinel
{
    FgAny                   source;             // Source node to get value from
    bool                    updated = false;    // Has the owner of this object updated it ?

    explicit Sentinel(FgAny & sourceNode) : source(sourceNode)
    {}
};

FgAny
inputImpl(const FgAny & obj)
{return FgAny(Input(obj)); }

FgAny
inputImpl(const FgAny & obj,const FnDestruct & fn)
{return FgAny(Input(obj,fn)); }

void
dirty(FgAnyWeaks & sinks);

void
dirty(FgAnyWeak & aw)
{
    if (aw.is<Sentinel>())
        aw.ref<Sentinel>().updated = false;
    else if (aw.is<Link>()) {
        Link &      lnk = aw.ref<Link>();
        if (lnk.updated) {
            lnk.updated = false;
            dirty(lnk.sinks);
        }
    }
    else if (aw.is<Select>()) {
        Select &    sel = aw.ref<Select>();
        // TODO: optimize for select:
        dirty(sel.sinks);
    }
    else
        fgThrow("FgLeg::dirty unhandled type",aw.typeName());
}

void
dirty(FgAnyWeaks & aws)
{
    for (FgAnyWeak & aw : aws)
        dirty(aw);
}

FgAny &
objectRef(FgAny & node)
{
    if (node.is<Input>()) {
        Input &     inp = node.ref<Input>();
        dirty(inp.sinks);
        return inp.object;
    }
    fgThrow("FgLeg::set on non-Input node",node.typeName());
    FG_UNREACHABLE_RETURN(node)
}

void
addSink(FgAny & source,const FgAnyWeak & sink)
{
    if (source.is<Input>())
        source.ref<Input>().sinks.push_back(sink);
    else if (source.is<Link>())
        source.ref<Link>().sinks.push_back(sink);
    else if (source.is<Select>())
        source.ref<Select>().sinks.push_back(sink);
    else
        fgThrow("FgLeg::addSink unhandled type",source.typeName());
}

void
addSink(FgAnys & sources,const FgAnyWeak & sink)
{
    for (FgAny & source : sources)
        addSink(source,sink);
}

FgAny
createLinkImpl(FgAnys & sources,const FnLink & fn,const FgAny & objEmpty)
{
    Link        lnk;
    lnk.object = objEmpty;  // Link function expects this to be already instantiated with correct type
    lnk.sources = sources;
    lnk.func = fn;
    FgAny       ret(lnk);
    addSink(sources,ret.weak());
    return ret;
}

FgAny
select(FgAny & selNode,FgAnys & sources)
{
    Select      sel;
    sel.index = selNode;
    sel.sources = sources;
    FgAny       ret(sel);
    addSink(selNode,ret.weak());
    addSink(sources,ret.weak());
    return ret;
}

FgAny
sentinel(FgAny & srcNode)
{
    FGASSERT(srcNode.is<Input>() || srcNode.is<Link>());
    FgAny       ret = Sentinel(srcNode);
    addSink(srcNode,ret.weak());
    return ret;
}

const FgAny &
getImpl(FgAny & node)
{
    if (node.is<Input>())
        return node.as<Input>().object;
    else if (node.is<Link>()) {
        Link &      lnk = node.ref<Link>();
        if (!lnk.updated) {
            FgAnys          inputs;
            inputs.reserve(lnk.sources.size());
            for (FgAny & s : lnk.sources)
                inputs.push_back(getImpl(s));
            lnk.func(inputs,lnk.object);
            lnk.updated = true;
        }
        return lnk.object;
    }
    else if (node.is<Select>()) {
        Select &        sel = node.ref<Select>();
        size_t          idx = getImpl(sel.index).as<size_t>();
        FGASSERT(idx < sel.sources.size());
        return getImpl(sel.sources[idx]);
    }
    else if (node.is<Sentinel>())
        return getImpl(node.ref<Sentinel>().source);
    fgThrow("FgLeg::getImpl unhandled type",node.typeName());
    FG_UNREACHABLE_RETURN(node)
}

}

// */
