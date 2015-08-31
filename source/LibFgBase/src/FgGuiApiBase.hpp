//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     March 17, 2011
//

#ifndef FGGUIAPIBASE_HPP
#define FGGUIAPIBASE_HPP

#include "FgSharedPtr.hpp"
#include "FgDepGraph.hpp"
#include "FgMetaFormat.hpp"
#include "FgImageBase.hpp"

struct  FgGuiOsBase;

typedef FgSharedPtr<FgGuiOsBase>    FgGuiOsPtr;
typedef vector<FgGuiOsPtr>          FgGuiOsPtrs;

struct  FgGuiApiBase
{
    virtual
    ~FgGuiApiBase()
    {};

    virtual
    FgSharedPtr<FgGuiOsBase> 
    getInstance() = 0;
};

typedef FgSharedPtr<FgGuiApiBase>   FgGuiPtr;
typedef vector<FgGuiPtr>            FgGuiPtrs;

template<class T>
FgSharedPtr<FgGuiApiBase>
fgGuiPtr(const T & stackVal)
{return FgSharedPtr<FgGuiApiBase>(new T(stackVal)); }

template<typename Child>
struct  FgGuiApi : FgGuiApiBase
{
    virtual
    FgSharedPtr<FgGuiOsBase>
    getInstance()
    {
        FgSharedPtr<FgGuiOsBase> fgGuiGetOsInstance(const Child & child);  // Declaration
        return fgGuiGetOsInstance(static_cast<const Child &>(*this));
    }
};

struct  FgGuiGraph
{
    FgDepGraphSt            dg;
    // Keep a snapshot of what's changed to avoid issues with nodes being marked clean during
    // the update process:
    FgString                m_storeBase;
    vector<boost::function<void()> > m_inputSaves;

    FgGuiGraph() {}

    explicit
    FgGuiGraph(const FgString & storeDir)
    : m_storeBase(storeDir+"guiInputState")
    {}

    template<class T>
    FgDgn<T>
    addNode(const T & val,const string & lab=string())
    {return dg.addNode(val,lab); }

    template<class T>
    FgDgn<T>
    addInput(const T & defaultVal,const std::string & uid,bool binary=false)
    {
        FgDgn<T>    node = dg.addNode(defaultVal,uid);
        readNode(node,uid,binary);
        m_inputSaves.push_back(boost::bind(&FgGuiGraph::writeNode<T>,this,node,uid,binary));
        return node;
    }

    void
    addLink(
        FgLink                  func,
        const vector<uint> &    sources,
        const vector<uint> &    sinks)
    {dg.addLink(func,sources,sinks); }

    template<class T>
    FgDgn<vector<T> >
    collate(const vector<FgDgn<T> > & nodes)
    {return fgDgCollate(dg,nodes); }

    template<class T>
    const T &
    getVal(FgDgn<T> dgn)
    {return dg.nodeVal(dgn); }

    template<class T>
    void
    setVal(FgDgn<T> dgn,const T & val)
    {dg.setNodeVal(dgn,val); }

    // Do not keep this reference outside of local scope:
    template<class T>
    T &
    getRef(FgDgn<T> dgn)
    {return dg.nodeValRef(dgn); }

    void
    saveInputs() const
    {
        for (size_t ii=0; ii<m_inputSaves.size(); ++ii)
            m_inputSaves[ii]();
    }

    // Defined in os-specific code:
    void
    updateScreen();

    // Defined in os-specfiic code:
    void
    quit();     // Close main window

    template<class T>
    void
    readNode(FgDgn<T> node,const std::string & uid,bool binary)
    {
        T       val;
        if (binary) {
            if (fgLoadBin(storeName(uid),val,false))
                dg.setNodeVal(node,val);
        }
        else {
            if (fgLoadXml(storeName(uid),val,false))
                dg.setNodeVal(node,val);
        }
    }

    template<class T>
    void
    writeNode(FgDgn<T> node,std::string uid,bool binary)
    {
        const T & val = dg.nodeVal(node);
        if (binary)
            fgSaveBin(storeName(uid),val,false);
        else
            fgSaveXml(storeName(uid),val,false);
    }

    FgString
    storeName(const std::string & uid)
    {return m_storeBase+uid+".xml"; }

    // Returns a no-op dependent node index from the given source. Provides a proxy flag for clients with
    // private state that needs to be updated from that source:
    uint
    addUpdateFlag(const vector<uint> & srcNodeInds)
    {
        uint    stubIdx = dg.addNode(0,"stub").idx();
        dg.addLink(fgLinkNoop,srcNodeInds,fgSvec(stubIdx));
        return stubIdx;
    }
    uint
    addUpdateFlag(uint srcNodeIdx)
    {return addUpdateFlag(fgSvec(srcNodeIdx)); }
};

// Global variable - very convenient as there will only ever be one GUI at a time:
extern FgGuiGraph g_gg;

typedef void(*FgFunc)();

struct  FgGuiApiEvent
{
    void *      handle;     // OS-specific handle to event for triggering main event-driven loop
    FgFunc      handler;    // Function to handle event
};

struct  FgGuiKeyHandle
{
    char        key;        // Only visible keys handled for now
    FgFunc      handler;
};

struct  FgGuiOptions
{
    vector<FgGuiApiEvent>   events;
    vector<FgGuiKeyHandle>  keyHandlers;
};

void
fgGuiImplStart(
    const FgString &        title,
    FgGuiPtr                def,
    const FgString &        storeDir,       // Directory in which to store gui state
    const FgGuiOptions &    options=FgGuiOptions());

#endif
