//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Nov. 20, 2009
//
// A directed acyclic bipartite graph in which the bipartite vertex types are 'Node' and 'Link'.
//
// INVARIANTS:
//
// * Every Node has either 0 or 1 incoming edges.
// * Every Link has 1 or more outgoing edges.
//
// DESIGN:
//
// The Nodes with outgoing edges to a Link are called its 'sources'.
// The Nodes with incoming edges from a Link are called its 'sinks'.
// Currently only used by FgDepGraph but templated to enforce abstraction of node data types.

#ifndef FGLINKGRAPH_HPP
#define FGLINKGRAPH_HPP

#include "FgOpt.hpp"
#include "FgStdString.hpp"
#include "FgString.hpp"

template<class NodeData,class LinkData>
struct  FgLinkGraph
{
    struct  Link
    {
        vector<uint>        sources;    // Indices into m_nodes
        vector<uint>        sinks;      // "
        LinkData            data;

        Link() {};
        Link(const vector<uint> &src,const vector<uint> &snk,LinkData d) : 
            sources(src), sinks(snk), data(d) {}
    };
    struct  Node
    {
        FgValid<uint>       incomingLink;
        vector<uint>        outgoingLinks;
        NodeData            data;

        Node(const NodeData & d) : data(d) {}
    };
    vector<Link>            m_links;
    vector<Node>            m_nodes;

    uint
    addNode(const NodeData & data);

    uint
    addLink(
        const LinkData &        data,
        const vector<uint> &    inputs,
        const vector<uint> &    outputs);

    void
    appendSource(uint sourceIdx,uint linkIdx);

    uint
    numNodes() const
    {return uint(m_nodes.size()); }

    uint
    numLinks() const
    {return uint(m_links.size()); }

    uint
    numSources(uint linkIdx) const
    {return uint(m_links[linkIdx].sources.size()); }

    const NodeData &
    nodeData(uint nodeIdx) const
    {return m_nodes[nodeIdx].data; }

    NodeData &
    nodeData(uint nodeIdx)
    {return m_nodes[nodeIdx].data; }

    const LinkData &
    linkData(uint linkIdx) const
    {return m_links[linkIdx].data; }

    LinkData &
    linkData(uint linkIdx)
    {return m_links[linkIdx].data; }

    const vector<uint> &
    outgoingLinks(uint nodeIdx) const
    {return m_nodes[nodeIdx].outgoingLinks; }

    bool
    hasIncomingLink(uint ind) const
    {return (m_nodes[ind].incomingLink.valid()); }

    uint
    incomingLink(uint sinkInd) const;

    const vector<uint> &
    linkSources(uint linkIdx) const
    {return m_links[linkIdx].sources; }

    const vector<uint> &
    linkSinks(uint linkIdx) const
    {return m_links[linkIdx].sinks; }

    const LinkData &
    getLink(uint linkIdx) const
    {return m_links[linkIdx].data; }
};

template<class NodeData,class LinkData>
uint
FgLinkGraph<NodeData,LinkData>::addNode(
    const NodeData &    nodeData)
{
    m_nodes.push_back(Node(nodeData));
    return static_cast<uint>(m_nodes.size()-1);
}

template<class NodeData,class LinkData>
uint
FgLinkGraph<NodeData,LinkData>::addLink(
    const LinkData &        data,
    const vector<uint> &    sources,
    const vector<uint> &    sinks)
{
    FGASSERT(sinks.size() > 0);           // A Link must have at least 1 sink
    uint        linkIdx = uint(m_links.size());
    m_links.push_back(Link(sources,sinks,data));
    // Update the node cross-referencing:
    for (size_t ii=0; ii<sinks.size(); ii++) {
        FGASSERT(sinks[ii] < uint(m_nodes.size()));
        FGASSERT(!fgContains(sources,sinks[ii]));       // Causes nasty bugs later
        Node &  node = m_nodes[sinks[ii]];
        if (node.incomingLink.valid())
            fgThrow("A FgLinkGraph node cannot be a sink for more than 1 link",fgToStr(ii));
        node.incomingLink = linkIdx;
    }
    for (size_t ii=0; ii<sources.size(); ii++) {
        FGASSERT(sources[ii] < uint(m_nodes.size()));
        Node &  node = m_nodes[sources[ii]];
        node.outgoingLinks.push_back(linkIdx);
    }
    return linkIdx;
}

template<class NodeData,class LinkData>
void
FgLinkGraph<NodeData,LinkData>::appendSource(
    uint        sourceInd,
    uint        linkIdx)
{
    FGASSERT(sourceInd < m_nodes.size());
    FGASSERT(linkIdx < m_links.size());
    m_links[linkIdx].sources.push_back(sourceInd);
    m_nodes[sourceInd].outgoingLinks.push_back(linkIdx);
}

template<class NodeData,class LinkData>
uint
FgLinkGraph<NodeData,LinkData>::incomingLink(
    uint        sinkInd)
    const
{
    FGASSERT(sinkInd < m_nodes.size());
    return m_nodes[sinkInd].incomingLink.val();
}

template<class NodeData,class LinkData>
void
fgTraverseDown(
    const FgLinkGraph<NodeData,LinkData> & lg,
    uint                nodeIdx,
    vector<bool> &      nodesTouched,       // MODIFIED
    vector<bool> &      linksTouched)       // MODIFIED
{
    nodesTouched[nodeIdx] = true;
    const vector<uint> &    outLinks = lg.outgoingLinks(nodeIdx);
    for (size_t ll=0; ll<outLinks.size(); ++ll) {
        uint    linkIdx = outLinks[ll];
        if (!linksTouched[linkIdx]) {
            linksTouched[linkIdx] = true;
            const vector<uint> &    sinks = lg.linkSinks(linkIdx);
            for (size_t nn=0; nn<sinks.size(); ++nn)
                if (!nodesTouched[sinks[nn]])
                    fgTraverseDown(lg,sinks[nn],nodesTouched,linksTouched);
        }
    }
}

template<class NodeData,class LinkData>
void
fgTraverseUp(
    const FgLinkGraph<NodeData,LinkData> & lg,
    uint                nodeIdx,
    vector<bool> &      nodesTouched,       // MODIFIED
    vector<bool> &      linksTouched)       // MODIFIED
{
    nodesTouched[nodeIdx] = true;
    if (!lg.hasIncomingLink(nodeIdx))
        return;
    uint    linkIdx = lg.incomingLink(nodeIdx);
    if (!linksTouched[linkIdx]) {
        linksTouched[linkIdx] = true;
        // We include all sink nodes of a traversed link even if they're not part
        // of the traversal, since running the link will write to all sinks:
        const vector<uint> &   sinks = lg.linkSinks(linkIdx);
        for (size_t nn=0; nn<sinks.size(); ++nn)
            nodesTouched[sinks[nn]] = true;
        // Continue traverse:
        const vector<uint> &   sources = lg.linkSources(linkIdx);
        for (size_t nn=0; nn<sources.size(); ++nn)
            if (!nodesTouched[sources[nn]])
                fgTraverseUp(lg,sources[nn],nodesTouched,linksTouched);
    }
}


#endif


// */
