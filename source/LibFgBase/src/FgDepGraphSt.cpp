//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:		Andrew Beatty
// Created:		Dec 7, 2006
//

#include "stdafx.h"

#include "FgStdString.hpp"
#include "FgOut.hpp"
#include "FgDepGraph.hpp"
#include "FgStdVector.hpp"
#include "FgDefaultVal.hpp"
#include "FgTime.hpp"

using namespace std;

std::ostream &
operator<<(std::ostream & os,const std::vector<FgLinkTime> & ltv)
{
    for (size_t ii=0; ii<ltv.size(); ++ii)
        os << fgnl << ltv[ii].percentTime << "% : " << ltv[ii].linkNodeNames;
    return os;
}

static vector<uint64>  s_linkTimes;

void
FgDepGraphSt::appendSource(
    uint    sourceInd,
    uint    sinkInd)
{
    uint    linkIdx = m_linkGraph.incomingLink(sinkInd);
    m_linkGraph.appendSource(sourceInd,linkIdx);
    dirtyLink(linkIdx);
}

void
FgDepGraphSt::executeLink(
    uint    linkInd)
    const
{
    s_linkTimes.resize(m_linkGraph.numLinks(),0);
    // We can only get here if the link is ready for calculation:
    const vector<uint> & sources = m_linkGraph.linkSources(linkInd);
    for (size_t ii=0; ii<sources.size(); ii++)
        FGASSERT(!(m_linkGraph.nodeData(sources[ii]).dirty));
    vector<const FgVariant*>    srcList(sources.size());
    for (size_t ii=0; ii<sources.size(); ii++)
        srcList[ii] = &(m_linkGraph.nodeData(sources[ii]).value);
    const vector<uint> &        sinks = m_linkGraph.linkSinks(linkInd);
    vector<FgVariant*>          snkList(sinks.size());
    for (uint ii=0; ii<sinks.size(); ii++)
        snkList[ii] = &(m_linkGraph.nodeData(sinks[ii]).value);
    FgLink                      link = m_linkGraph.linkData(linkInd);
    FGASSERT(link != 0);
    FgTimer     timer;
    link(srcList,snkList);
    s_linkTimes[linkInd] += timer.readMs();
    for (size_t ss=0; ss<sinks.size(); ++ss)
        m_linkGraph.nodeData(sinks[ss]).dirty = false;
}

static
bool
ltCmp(const FgLinkTime & l,const FgLinkTime & r)
{return (l.percentTime > r.percentTime); }

vector<FgLinkTime>
FgDepGraphSt::linkTimes() const
{
    vector<FgLinkTime>  ret;
    uint64              sum = fgSum(s_linkTimes);
    string              bn = "L";
    for (uint ii=0; ii<s_linkTimes.size(); ++ii) {
        FgLinkTime      lt;
        lt.linkNodeNames = bn + fgToString(ii);
        lt.percentTime = s_linkTimes[ii] * 100.0 / double(sum);
        ret.push_back(lt);
    }
    std::sort(ret.begin(),ret.end(),ltCmp);
    return fgHead(ret,10);
}

void
FgDepGraphSt::dirtyNode(uint nodeInd)
{
    // All dirty nodes have been up-propagated:
    if (m_linkGraph.nodeData(nodeInd).dirty)
        return;
    // Dirty this node and propagate up:
    m_linkGraph.nodeData(nodeInd).dirty = true;
    const vector<uint> &    dirtyLinks = m_linkGraph.outgoingLinks(nodeInd);
    for (size_t ii=0; ii<dirtyLinks.size(); ii++)
        dirtyLink(dirtyLinks[ii]);
}

void
FgDepGraphSt::dirtyLink(uint linkInd)
{
    const vector<uint> &    sinks = m_linkGraph.linkSinks(linkInd);
    for (size_t kk=0; kk<sinks.size(); kk++)
        dirtyNode(sinks[kk]);
}

void
FgDepGraphSt::updateNode(uint nodeIdx)
{
    Node &          node = m_linkGraph.m_nodes[nodeIdx];
    if (!node.data.dirty)
        return;
    if (node.incomingLink.valid()) {
        uint        linkIdx = node.incomingLink.val();
        Link        link = m_linkGraph.m_links[linkIdx];
        for (size_t ii=0; ii<link.sources.size(); ++ii)
            updateNode(link.sources[ii]);
        executeLink(linkIdx);       // marks this node (and any other output nodes) as non-dirty
    }
    else
        node.data.dirty = false;
}

// */
