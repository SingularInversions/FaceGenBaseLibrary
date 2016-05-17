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

FgDepGraph::FgDepGraph(uint num_threads)
{
    if (num_threads > 0)
        m_numThreads = num_threads;
    else
        m_numThreads = uint(boost::thread::hardware_concurrency());
}

void
FgDepGraph::appendSource(
    uint    sourceInd,
    uint    sinkInd)
{
    uint    linkIdx = m_linkGraph.incomingLink(sinkInd);
    m_linkGraph.appendSource(sourceInd,linkIdx);
    dirtyLink(linkIdx);
}

void
FgDepGraph::traverseDown(
    uint                nodeIdx,
    vector<bool> &      nodesTouched,       // MODIFIED
    vector<bool> &      linksTouched)       // MODIFIED
    const
{
    nodesTouched.resize(m_linkGraph.numNodes(),false);
    linksTouched.resize(m_linkGraph.numLinks(),false);
    fgTraverseDown(m_linkGraph,nodeIdx,nodesTouched,linksTouched);
}

void
FgDepGraph::traverseUp(
    uint                nodeIdx,
    vector<bool> &      nodesTouched,       // MODIFIED
    vector<bool> &      linksTouched)       // MODIFIED
    const
{
    nodesTouched.resize(m_linkGraph.numNodes(),false);
    linksTouched.resize(m_linkGraph.numLinks(),false);
    fgTraverseUp(m_linkGraph,nodeIdx,nodesTouched,linksTouched);
}

void
FgDepGraph::executeLink(
    uint    linkInd)
    const
{
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
    FgLink      link = m_linkGraph.linkData(linkInd);
    FGASSERT(link != 0);
    link(srcList,snkList);
    for (size_t ss=0; ss<sinks.size(); ++ss)
        m_linkGraph.nodeData(sinks[ss]).dirty = false;
}

void
FgDepGraph::dirtyNode(uint nodeInd)
{
    // All dirty nodes have been up-propagated
    if (m_linkGraph.nodeData(nodeInd).dirty)
        return;
    // Dirty this node and propagate up
    m_linkGraph.nodeData(nodeInd).dirty = true;
    const vector<uint> &    dirtyLinks = m_linkGraph.outgoingLinks(nodeInd);
    for (size_t ii=0; ii<dirtyLinks.size(); ii++) {
        const vector<uint> &    dirtyNodes = m_linkGraph.linkSinks(dirtyLinks[ii]);
        for (size_t jj=0; jj<dirtyNodes.size(); jj++)
            dirtyNode(dirtyNodes[jj]);
    }
}

void
FgDepGraph::dirtyLink(uint linkInd)
{
    const vector<uint> &    sinks = m_linkGraph.linkSinks(linkInd);
    for (size_t kk=0; kk<sinks.size(); kk++)
        dirtyNode(sinks[kk]);
}

void
FgDepGraph::updateNode(uint nodeIdx) const
{
    Update          update;
    vector<Sync>    linksSync(m_linkGraph.numLinks());
    // Traverse to find dirty leaves and set up scheduling on dirty non-leaves
    update.queue = leafLinks(nodeIdx,linksSync);
    if (update.queue.empty())
        return;
    update.lastLink = m_linkGraph.incomingLink(nodeIdx);
    vector<FgSinglePtr<boost::thread> > threads(numThreads());
    for (size_t tt=0; tt<threads.size(); ++tt)
        threads[tt] = new boost::thread(boost::bind(
            &FgDepGraph::executeLinkTask,this,&linksSync,&update,(tt == 0)));
    do {
        boost::thread::yield();
    }
    while (!update.done);
    for (size_t tt=0; tt<threads.size(); ++tt)
        threads[tt]->join();
    if (update.flag) {
        if (update.userCancelled)
            throw FgExceptionUserCancel();
        else
            fgThrow(update.exception);
    }
}

vector<uint>
FgDepGraph::leafLinks(
    uint            nodeIdx,
    vector<Sync> &  linksSync) const
{
    const FgDepNode &    nd = m_linkGraph.nodeData(nodeIdx);
    if (!nd.dirty)
        return vector<uint>();
    if (!m_linkGraph.hasIncomingLink(nodeIdx)) {
        nd.dirty = false;
        return vector<uint>();
    }
    uint                linkIdx = m_linkGraph.incomingLink(nodeIdx);
    Sync &              sync = linksSync[linkIdx];
    if (sync.traversed)
        return vector<uint>();
    sync.traversed = true;
    // Find all links that this link depends on:
    const vector<uint> &    srcNodes = m_linkGraph.linkSources(linkIdx);
    // Count the number of input nodes that will be updated by parent links before
    // this link triggers. Note that although a single parent link can update more
    // than one input node, that link will also multply decrement this count:
    int                     incomingRemaining = 0;
    for (size_t ii=0; ii<srcNodes.size(); ++ii) {
        const FgDepNode &    snd = m_linkGraph.nodeData(srcNodes[ii]);
        if (snd.dirty)
            if (m_linkGraph.hasIncomingLink(srcNodes[ii]))
                ++incomingRemaining;
    }
    vector<uint>            ret;
    if (incomingRemaining == 0)     // This is a leaf link for the update calc:
        ret.push_back(linkIdx);
    else {
        // Set up scheduling for this link:
        sync.incomingRemaining = incomingRemaining;
        sync.mtxPtr = new boost::mutex;
    }
    // Continue the traverse (even for leaf nodes since we need to mark sources clean):
    for (size_t ii=0; ii<srcNodes.size(); ++ii)
        fgAppend(ret,leafLinks(srcNodes[ii],linksSync));
    return ret;
}

void
FgDepGraph::executeLinkTask(
    vector<Sync> *  syncPtr,
    Update *        updPtr,
    bool            doCancelCheck) const
{
    uint        linkInd = 0;
    try {
        while (!updPtr->done) {
            // This check cannot depend on whether there are currently jobs
            // in the queue, since only one thread is doing the checking:
            if (doCancelCheck)
                if (m_cancelCheck.valid())
                    if (updPtr->cancelCheck(m_cancelCheck.val()))
                        return;
            updPtr->guardQueue->lock();
            bool    empty = updPtr->queue.empty();
            if (!empty) {
                linkInd = updPtr->queue.back();
                updPtr->queue.pop_back();
            }
            updPtr->guardQueue->unlock();
            if (empty) {
                boost::thread::yield();
                continue;
            }
            bool    followNext = false;
            // This loop follows the last output link (if any) and schedules the
            // rest. This optimization saves about a per-cent by reducing contention
            // on the queue:
            do {
                executeLink(linkInd);
                if (linkInd == updPtr->lastLink) {
                    updPtr->done = true;
                    return;
                }
                vector<uint>            todo;
                const vector<uint> &    sinkNodes = m_linkGraph.linkSinks(linkInd);
                for (size_t ii=0; ii<sinkNodes.size(); ++ii) {
                    uint    nodeIdx = sinkNodes[ii];
                    const vector<uint> &    depLinks = m_linkGraph.outgoingLinks(nodeIdx);
                    for (size_t jj=0; jj<depLinks.size(); ++jj) {
                        Sync &  sync = (*syncPtr)[depLinks[jj]];
                        if (sync.traversed) {
                            sync.mtxPtr->lock();
                            int     ir = --sync.incomingRemaining;
                            sync.mtxPtr->unlock();
                            FGASSERT(ir >= 0);
                            if (ir == 0)
                                todo.push_back(depLinks[jj]);
                        }
                    }
                }
                if (!todo.empty()) {
                    if (updPtr->done)
                        return;
                    if (doCancelCheck)
                        if (m_cancelCheck.valid())
                            if (updPtr->cancelCheck(m_cancelCheck.val()))
                                return;
                    linkInd = todo.back();
                    followNext = true;
                    todo.pop_back();
                    if (!todo.empty()) {
                        updPtr->guardQueue->lock();
                        fgAppend(updPtr->queue,todo);
                        updPtr->guardQueue->unlock();
                    }
                }
                else
                    followNext = false;
            } while (followNext);
        }
    }
    catch(FgException const & e) {
        updPtr->set(e,linkInd);
    }
    catch(std::exception const & e) {
        updPtr->set(FgException("Standard library exception",e.what()),linkInd);
    }
    catch(...) {
        updPtr->set(FgException("Unknown exception type"),linkInd);
    }
}

void
FgDepGraph::Update::set(
    const FgException &     e,
    uint                    linkIdx)
{
    guardException->lock();
    bool prior = flag;
    flag = true;
    guardException->unlock();
    if (!prior) {   // If another thread hasn't already reported an exception:
        done = true;
        exception = e;
        exception.pushMsg(
            "A computation within an FgDepGraph has generated an exception on link",
            fgToString(linkIdx));
    }
}

bool
FgDepGraph::Update::cancelCheck(int(*func)())
{
    if (func() != 0) {
        guardException->lock();
        bool prior = flag;
        flag = true;
        guardException->unlock();
        if (!prior) {   // If another thread hasn't already reported an exception:
            done = true;
            userCancelled = true;
        }
        return true;
    }
    return false;
}

// */
