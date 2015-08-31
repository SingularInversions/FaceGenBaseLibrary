//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Dec 7, 2006
//
// Dependency graph - lazy updates in a dependency graph (acyclic) of calculations.
//
// USE:
//
// There are 2 versions; a multithreaded version (FgDepGraph) and a single-threaded version
// (FgDepGraphSt).
// Structure is added to the model incrementally and cannot be removed.
// Link functions must be of type FgLink
// Nodes are stored as FgVariants
// Updating is done lazily when a desired output value is requested
// Updates are fully multithreaded using the number of virtual cores by default
//
// INVARIANTS:
//
// * Every node is the sink for either 0 or 1 links.
// * Every link has 1 or more sinks (>1 isn't strictly necessary but can be handy).
// * If a node is dirty all of its dependent nodes are also dirty.
// * Using the API as commented below will always give a fully updated value for any sink node.
// * A link can have 0 or more inputs.
// * A 'const' member function will never alter the apparent state of this object (although
//   it may involve a lazy update calculation).
//

#ifndef FGDEPGRAPH_HPP
#define FGDEPGRAPH_HPP

#include "FgStdVector.hpp"
#include "FgLinkGraph.hpp"
#include "FgVariant.hpp"
#include "FgValidVal.hpp"
#include "FgCluster.hpp"
#include "FgSmartPtr.hpp"

typedef
void (*FgLink)(
    const std::vector<const FgVariant*> &,
    const std::vector<FgVariant*> &);

template<class T>
struct  FgDgn
{
    FgValid<uint>       m_idx;

    FgDgn() {}

    explicit
    FgDgn(uint idx)
    : m_idx(idx)
    {}

    // Convenient way to declare and create without duplication:
    template<class DepGraph>
    FgDgn(DepGraph & dg,const string & label)
    : m_idx(dg.addNode(T(),label))
    {}

    operator uint () const
    {return m_idx.val(); }

    uint
    idx() const
    {return m_idx.val(); }

    bool
    valid() const
    {return m_idx.valid(); }
};

#define FGLINK(func)                                        \
    void func(                                              \
        const std::vector<const FgVariant*> &   inputs,     \
        const std::vector<FgVariant*> &         outputs)

#define FGLINKARGS(in,out)                                  \
        FGASSERT((inputs.size() == in) && (outputs.size() == out))

template<class I0,class O0,O0 (*F)(const I0 &)>
FGLINK(fgLink11)
{
    FGLINKARGS(1,1);
    const I0 &  i0 = inputs[0]->valueRef();
    O0 &        o0 = outputs[0]->valueRef();
    o0 = F(i0);
}

template<class I0,class I1,class O0,O0 (*F)(const I0 &,const I1 &)>
FGLINK(fgLink21)
{
    FGLINKARGS(2,1);
    const I0 &  i0 = inputs[0]->valueRef();
    const I1 &  i1 = inputs[1]->valueRef();
    O0 &        o0 = outputs[0]->valueRef();
    o0 = F(i0,i1);
}

struct  FgDepNode
{
    FgDepNode() : dirty(true) {}

    explicit
    FgDepNode(const FgVariant & val) : value(val), dirty(true) {}

    template<class T>
    FgDepNode(const std::string & lab,const T & val)
    : label(lab), value(val), dirty(true)
    {}

    string
    name(uint idx) const
    {
        if (label.empty())
            return fgToString(idx);
        else
            return fgToString(idx)+": "+label;
    }

    string              label;
    mutable FgVariant   value;
    mutable bool        dirty;
};

class   FgDepGraph
{
public:
    explicit
    FgDepGraph(uint num_threads=0);

    template<class T>
    FgDgn<T>
    addNode(
        const T &           val,
        const std::string & label="")
    {return FgDgn<T>(m_linkGraph.addNode(FgDepNode(label,val))); }

    void
    addLink(
        FgLink                      func,
        const std::vector<uint> &   sources,
        const std::vector<uint> &   sinks)
    {dirtyLink(m_linkGraph.addLink(func,sources,sinks)); }

    void
    changeLink(
        FgLink  func,
        uint    linkIdx)
    {m_linkGraph.linkData(linkIdx) = func; dirtyLink(linkIdx); }

    void
    appendSource(uint sourceInd,uint sinkInd);

    void
    setUserCancelCallback(int(*cancelCheck)())
    {
        FGASSERT(cancelCheck != 0);
        m_cancelCheck = cancelCheck;
    }

    uint
    numNodes() const
    {return m_linkGraph.numNodes(); }

    uint
    numLinks() const
    {return m_linkGraph.numLinks(); }

    bool
    isDirty(uint nodeInd) const
    {return m_linkGraph.nodeData(nodeInd).dirty; }

    // Retrieving a value triggers the lazy update of that value, which is invisible
    // to the client, thus this appears as a const member:
    const FgVariant &
    getNode(uint nodeInd) const
    {
        updateNode(nodeInd);
        return m_linkGraph.nodeData(nodeInd).value; 
    }

    void
    setNode(uint nodeIdx,const FgVariant & val)
    {
        // A value with incoming links should not be modified directly:
        FGASSERT(!m_linkGraph.hasIncomingLink(nodeIdx));
        m_linkGraph.nodeData(nodeIdx).value = val;
        dirtyNode(nodeIdx);
    }

    // Use 'valueRef' when type can be inferred. The const version must be explicit
    // since non-const access sets dirty bits so should not be used by default:
    FgVariant::ConstValueProxy
    valueCRef(uint nodeIdx) const
    {return FgVariant::ConstValueProxy(&getNode(nodeIdx)); }

    FgVariant::ValueProxy
    valueRef(uint nodeIdx)
    {
        // A value with incoming links should not be modified directly:
        FGASSERT(!m_linkGraph.hasIncomingLink(nodeIdx));
        dirtyNode(nodeIdx);
        return FgVariant::ValueProxy(&m_linkGraph.nodeData(nodeIdx).value);
    }

    template<class T>
    const T &
    nodeVal(FgDgn<T> node) const
    {
        updateNode(node.idx());
        const FgVariant &   val =  m_linkGraph.nodeData(node.idx()).value;
        return val.getCRef<T>();
    }

    // WARNING: Not safe to retain returned reference:
    template<class T>
    T &
    nodeValRef(FgDgn<T> node)
    {
        // A value with incoming links should not be modified directly:
        FGASSERT(!m_linkGraph.hasIncomingLink(node.idx()));
        FgVariant &     val =  m_linkGraph.nodeData(node.idx()).value;
        dirtyNode(node.idx());
        return val.getRef<T>();
    }

    // Update node value safely.
    template<class T>
    void
    setNodeVal(uint nodeInd,const T & val)
    {
        FGASSERT(!m_linkGraph.hasIncomingLink(nodeInd));
        T & ptr = m_linkGraph.nodeData(nodeInd).value.getRef<T>();
        ptr = val;
        dirtyNode(nodeInd);
    }

    void
    traverseDown(
        uint                nodeIdx,
        std::vector<bool> & nodesTouched,       // MODIFIED
        std::vector<bool> & linksTouched)       // MODIFIED
        const;

    void
    traverseUp(
        uint                nodeIdx,
        std::vector<bool> & nodesTouched,       // MODIFIED
        std::vector<bool> & linksTouched)       // MODIFIED
        const;

    const std::vector<uint> &
    linkSources(uint linkIdx) const
    {return m_linkGraph.linkSources(linkIdx); }

    const std::vector<uint> &
    linkSinks(uint linkIdx) const
    {return m_linkGraph.linkSinks(linkIdx); }

    uint
    incomingLink(uint nodeIdx) const
    {return m_linkGraph.incomingLink(nodeIdx); }

    FgLink
    getLink(uint linkIdx) const
    {return m_linkGraph.getLink(linkIdx); }

    std::string
    nodeName(uint nodeIdx) const
    {return m_linkGraph.nodeData(nodeIdx).name(nodeIdx); }

    uint
    numThreads() const
    {return (fgClusterThreads > 0) ? fgClusterThreads : uint(boost::thread::hardware_concurrency()); }

    const FgLinkGraph<FgDepNode,FgLink> &
    linkGraph() const
    {return m_linkGraph; }

private:
    FgLinkGraph<FgDepNode,FgLink>   m_linkGraph;
    FgValidVal<int(*)()>            m_cancelCheck;  // If valid and returns non-zero, cancel calculations

    void
    dirtyNode(uint nodeInd);

    void
    dirtyLink(uint linkInd);

    void
    executeLink(uint linkInd) const;

    struct  Sync
    {
        Sync() : traversed(false) {}
        bool        traversed;              // Initial scheduling traverse flag
        FgSinglePtr<boost::mutex> mtxPtr;   // Guard following member:
        int         incomingRemaining;      // How many input nodes need to be updated before this link runs ?
    };
    struct  Update
    {
        Update() :
            guardException(new boost::mutex),
            flag(false),
            exception(""),
            guardQueue(new boost::mutex),
            done(false),
            userCancelled(false)
            {}
        std::auto_ptr<boost::mutex> guardException; // Guard 2 members below:
        bool                        flag;           // Error or cancellation exception has occurred
        FgException                 exception;
        std::auto_ptr<boost::mutex> guardQueue;
        char                        cachePad0[64-sizeof(std::auto_ptr<boost::mutex>)];
        std::vector<uint>           queue;
        char                        cachePad1[64-sizeof(size_t)];
        uint                        lastLink;
        bool                        done;
        bool                        userCancelled;

        void set(const FgException &,uint);
        bool cancelCheck(int(*)());                 // Not MT safe, only call from 1 thread.
    };

    void
    updateNode(uint nodeInd) const;

    // Returns: true if links were scheduled that ultimately will update 'nodeIdx':
    std::vector<uint>
    leafLinks(
        uint                nodeIdx,
        std::vector<Sync> & linksTraversed) const;

    void
    executeLinkTask(std::vector<Sync> *,Update *,bool) const;
};

struct  FgLinkTime
{
    std::string     linkNodeNames;
    float           percentTime;
};

std::ostream &
operator<<(std::ostream & os,const std::vector<FgLinkTime> & ltv);

// Single-threaded depGraph
class   FgDepGraphSt
{
public:
    template<class T>
    FgDgn<T>
    addNode(
        const T &           val,
        const std::string & label="")
    {return FgDgn<T>(m_linkGraph.addNode(FgDepNode(label,val))); }

    void
    addLink(
        FgLink                      func,
        const std::vector<uint> &   sources,
        const std::vector<uint> &   sinks)
    {dirtyLink(m_linkGraph.addLink(func,sources,sinks)); }

    void
    appendSource(uint sourceInd,uint sinkInd);

    uint
    numNodes() const
    {return m_linkGraph.numNodes(); }

    uint
    numLinks() const
    {return m_linkGraph.numLinks(); }

    bool
    isDirty(uint nodeIdx) const
    {return m_linkGraph.nodeData(nodeIdx).dirty; }

    // Returns true if the node was dirty and needed to be updated:
    bool
    update(uint nodeIdx)
    {
        bool    ret = isDirty(nodeIdx);
        updateNode(nodeIdx);
        return ret;
    }

    // Retrieving a value triggers the lazy update of that value, which is invisible
    // to the client, thus this appears as a const member:
    const FgVariant &
    getNode(uint nodeInd)
    {
        updateNode(nodeInd);
        return m_linkGraph.nodeData(nodeInd).value; 
    }

    void
    setNode(uint nodeIdx,const FgVariant & val)
    {
        // A value with incoming links should not be modified directly:
        FGASSERT(!m_linkGraph.hasIncomingLink(nodeIdx));
        m_linkGraph.nodeData(nodeIdx).value = val;
        dirtyNode(nodeIdx);
    }

    // Use 'valueRef' when type can be inferred. The const version must be explicit
    // since non-const access sets dirty bits so should not be used by default:
    FgVariant::ConstValueProxy
    valueCRef(uint nodeIdx)
    {return FgVariant::ConstValueProxy(&getNode(nodeIdx)); }

    FgVariant::ValueProxy
    valueRef(uint nodeIdx)
    {
        // A value with incoming links should not be modified directly:
        FGASSERT(!m_linkGraph.hasIncomingLink(nodeIdx));
        dirtyNode(nodeIdx);
        return FgVariant::ValueProxy(&m_linkGraph.nodeData(nodeIdx).value);
    }

    template<class T>
    const T &
    nodeVal(FgDgn<T> node)
    {
        updateNode(node.idx());
        const FgVariant &   val =  m_linkGraph.nodeData(node.idx()).value;
        return val.getCRef<T>();
    }

    // WARNING: Not safe to retain returned reference:
    template<class T>
    T &
    nodeValRef(FgDgn<T> node)
    {
        // A value with incoming links should not be modified directly:
        FGASSERT(!m_linkGraph.hasIncomingLink(node.idx()));
        FgVariant &     val =  m_linkGraph.nodeData(node.idx()).value;
        dirtyNode(node.idx());
        return val.getRef<T>();
    }

    // Update node value safely.
    template<class T>
    void
    setNodeVal(uint nodeInd,const T & val)
    {
        FGASSERT(!m_linkGraph.hasIncomingLink(nodeInd));
        T & ptr = m_linkGraph.nodeData(nodeInd).value.getRef<T>();
        ptr = val;
        dirtyNode(nodeInd);
    }

    const std::vector<uint> &
    linkSources(uint linkIdx) const
    {return m_linkGraph.linkSources(linkIdx); }

    const std::vector<uint> &
    linkSinks(uint linkIdx) const
    {return m_linkGraph.linkSinks(linkIdx); }

    uint
    incomingLink(uint nodeIdx) const
    {return m_linkGraph.incomingLink(nodeIdx); }

    FgLink
    getLink(uint linkIdx) const
    {return m_linkGraph.getLink(linkIdx); }

    std::string
    nodeName(uint nodeIdx) const
    {return m_linkGraph.nodeData(nodeIdx).name(nodeIdx); }

    uint
    numThreads() const
    {return (fgClusterThreads > 0) ? fgClusterThreads : uint(boost::thread::hardware_concurrency()); }

    std::vector<FgLinkTime>
    linkTimes() const;

    bool
    sinkNode(uint nodeIdx) const
    {return m_linkGraph.m_nodes[nodeIdx].incomingLink.valid(); }

    const FgLinkGraph<FgDepNode,FgLink> &
    linkGraph() const
    {return m_linkGraph; }

private:
    FgLinkGraph<FgDepNode,FgLink>   m_linkGraph;

    typedef FgLinkGraph<FgDepNode,FgLink>::Node  Node;
    typedef FgLinkGraph<FgDepNode,FgLink>::Link  Link;

    void
    dirtyNode(uint nodeInd);

    void
    dirtyLink(uint linkInd);

    void
    executeLink(uint linkInd) const;

    struct  Sync
    {
        Sync() : traversed(false) {}
        bool        traversed;              // Initial scheduling traverse flag
        int         incomingRemaining;      // How many input nodes need to be updated before this link runs ?
    };
    struct  Update
    {
        Update() :
            done(false)
            {}
        std::vector<uint>           queue;
        uint                        lastLink;
        bool                        done;

        void set(uint);
    };

    void
    updateNode(uint nodeInd);

    // Returns: true if links were scheduled that ultimately will update 'nodeIdx':
    std::vector<uint>
    leafLinks(
        uint                nodeIdx,
        std::vector<Sync> & linksTraversed) const;

    void
    executeLinkTask(std::vector<Sync> *,Update *) const;
};

// Forces type conversion into uint and easier than typeing 'fgSvec<uint>' every time:
inline vector<uint> fgUints(uint a) {return fgSvec(a); }
inline vector<uint> fgUints(uint a,uint b) {return fgSvec(a,b); }
inline vector<uint> fgUints(uint a,uint b,uint c) {return fgSvec(a,b,c); }
inline vector<uint> fgUints(uint a,uint b,uint c,uint d) {return fgSvec(a,b,c,d); }
inline vector<uint> fgUints(uint a,uint b,uint c,uint d,uint e) {return fgSvec(a,b,c,d,e); }
inline vector<uint> fgUints(uint a,uint b,uint c,uint d,uint e,uint f)
{return fgSvec(a,b,c,d,e,f); }
inline vector<uint> fgUints(uint a,uint b,uint c,uint d,uint e,uint f,uint g)
{return fgSvec(a,b,c,d,e,f,g); }
inline vector<uint> fgUints(uint a,uint b,uint c,uint d,uint e,uint f,uint g,uint h)
{return fgSvec(a,b,c,d,e,f,g,h); }
inline vector<uint> fgUints(uint a,uint b,uint c,uint d,uint e,uint f,uint g,uint h,uint i)
{return fgSvec(a,b,c,d,e,f,g,h,i); }

template<class T>
FGLINK(linkCollate)
{
    FGASSERT(outputs.size() == 1);
    std::vector<T> &        out = outputs[0]->valueRef();
    out.clear();
    for (size_t ii=0; ii<inputs.size(); ++ii)
    {
        const T &           in = inputs[ii]->valueRef();
        out.push_back(in);
    }
}

template<class T>
FgDgn<std::vector<T> >
fgDgCollate(
    FgDepGraphSt &                   dg,
    const std::vector<FgDgn<T> > &  nodes)
{
    FgDgn<std::vector<T> >  ret = dg.addNode(std::vector<T>());
    std::vector<uint>       inp(nodes.size());
    for (size_t ii=0; ii<inp.size(); ++ii)
        inp[ii] = nodes[ii].idx();
    dg.addLink(linkCollate<T>,inp,fgUints(ret));
    return ret;
}

FGLINK(fgLinkNoop);

// The 'dot' executable from Graphviz must be in the path for this to work:
void
fgDepGraph2Pdf(
    const FgLinkGraph<FgDepNode,FgLink> &   lg,
    const std::string &                     rootName,
    const std::vector<uint> &               paramInds=std::vector<uint>());

#endif

// */
