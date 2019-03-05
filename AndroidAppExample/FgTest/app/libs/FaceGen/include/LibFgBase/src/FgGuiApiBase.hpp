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

#include "FgStdFunction.hpp"
#include "FgSharedPtr.hpp"
#include "FgDepGraph.hpp"
#include "FgMetaFormat.hpp"
#include "FgImageBase.hpp"

// Set up this data structure for application error handling (eg. report to server):
struct  FgGuiDiagHandler
{
    FgString                        appNameVer;     // Full name of application plus version
    // Client-defined error reporting. Can be null.
    // Accepts error message, returns true if reported, false otherwise (so default dialog can be shown):
    std::function<bool(FgString)> reportError;
    FgString                        reportSuccMsg;  // Displayed if 'reportError' returns true.
    // Prepended to error message and displayed if 'reportError' == NULL or 'reportError' returns false:
    FgString                        reportFailMsg;
};

extern FgGuiDiagHandler         g_guiDiagHandler;

struct  FgGuiOsBase;

typedef FgPtr<FgGuiOsBase>      FgGuiOsPtr;
typedef vector<FgGuiOsPtr>      FgGuiOsPtrs;

struct  FgGuiApiBase
{
    virtual ~FgGuiApiBase() {};         // Don't leak

    virtual
    FgPtr<FgGuiOsBase> 
    getInstance() = 0;
};

typedef FgPtr<FgGuiApiBase>     FgGuiPtr;
typedef vector<FgGuiPtr>        FgGuiPtrs;

template<class T>
FgPtr<FgGuiApiBase>
fgGuiPtr(const T & stackVal)
{return FgPtr<FgGuiApiBase>(new T(stackVal)); }

template<typename Child>
struct  FgGuiApi : FgGuiApiBase
{
    virtual
    FgPtr<FgGuiOsBase>
    getInstance()
    {
        FgPtr<FgGuiOsBase> fgGuiGetOsInstance(const Child & child);  // Declaration
        return fgGuiGetOsInstance(static_cast<const Child &>(*this));
    }
};

struct  FgGuiGraph
{
    struct  Input
    {
        uint                        nodeIdx;
        std::function<void()>     save;
        FgVariant                   defaultVal;
    };

    FgDepGraphSt                    dg;
    FgString                        m_storeBase;
    vector<Input>                   m_inputSaves;
    vector<uint>                    ensureUpdatedWithScreen;

    FgGuiGraph() {}

    explicit
    FgGuiGraph(const FgString & storeDir)
    : m_storeBase(storeDir+"gg_")
    {}

    template<class T>
    FgDgn<T>
    addNode(const T & val,const string & lab=string())
    {return dg.addNode(val,lab); }

    template<class T>
    FgDgn<T>
    addInput(const T & defaultVal,const FgString & guid,bool binary=false)
    {
        FgDgn<T>    node = dg.addNode(defaultVal,guid.as_ascii());
        readNode(node,guid,binary);
        Input       inp;
        inp.nodeIdx = node.idx();
        inp.save = std::bind(&FgGuiGraph::writeNode<T>,this,node,guid,binary);
        inp.defaultVal = FgVariant(defaultVal);
        m_inputSaves.push_back(inp);
        return node;
    }

    void
    addLink(
        FgLink                  func,
        const vector<uint> &    sources,
        const vector<uint> &    sinks)
    {dg.addLink(func,sources,sinks); }

    void
    addLink(
        FgLink                  func,
        uint                    source,
        const vector<uint> &    sinks)
    {dg.addLink(func,fgSvec(source),sinks); }

    void
    addLink(
        FgLink                  func,
        const vector<uint> &    sources,
        uint                    sink)
    {dg.addLink(func,sources,fgSvec(sink)); }

    void
    addLink(
        FgLink                  func,
        uint                    source,
        uint                    sink)
    {dg.addLink(func,fgSvec(source),fgSvec(sink)); }

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
            m_inputSaves[ii].save();
    }

    void
    updateScreen()
    {
        updateScreenImpl();
        for (size_t ii=0; ii<ensureUpdatedWithScreen.size(); ++ii)
            dg.update(ensureUpdatedWithScreen[ii]);
    }

    // Defined in os-specific code:
    void
    updateScreenImpl();

    // Defined in os-specfiic code:
    void
    quit();     // Close main window

    template<class T>
    void
    readNode(FgDgn<T> node,const FgString & uid,bool binary)
    {
        T       val;
        if (binary) {
            if (fgLoadPBin(m_storeBase+uid,val,false))
                dg.setNodeVal(node,val);
        }
        else {
            if (fgLoadXml(m_storeBase+uid+".xml",val,false))
                dg.setNodeVal(node,val);
        }
    }

    template<class T>
    void
    writeNode(FgDgn<T> node,FgString uid,bool binary)
    {
        FgPath      path(m_storeBase+uid);
        fgCreatePath(path.dir());
        const T & val = dg.nodeVal(node);
        if (binary)
            fgSavePBin(path.str(),val,false);
        else
            fgSaveXml(path.str()+".xml",val,false);
    }

    // Returns a no-op dependent node index from the given source. Provides a proxy flag for clients with
    // private state that needs to be updated from that source:
    uint
    addUpdateFlag(const vector<uint> & srcNodeInds)
    {
        uint    stubIdx = dg.addNode(0,"stub").idx();
        dg.addLink(fgLnkNoop,srcNodeInds,fgSvec(stubIdx));
        return stubIdx;
    }
    uint
    addUpdateFlag(uint srcNodeIdx)
    {return addUpdateFlag(fgSvec(srcNodeIdx)); }

    // Only sets input node values on which nodeIdx depends:
    void
    setInputsToDefault(uint nodeIdx);

    template<class T>
    FgDgn<T>
    lnkSelect(FgDgn<vector<T> > valsN,FgDgn<size_t> idxN)
    {
        FgDgn<T>        ret = dg.addNode(T());
        dg.addLink(fgLnkSelect<T>,fgSvec<uint>(valsN,idxN),fgSvec<uint>(ret));
        return ret;
    }

    template<class T>
    FgDgn<vector<T> >
    lnkCollate(const vector<FgDgn<T> > & nodes)
    {
        FgDgn<vector<T> >       ret = dg.addNode(vector<T>());
        dg.addLink(fgLinkCollate<T>,fgUints(nodes),fgUints(ret));
        return ret;
    }

    template<class T>
    FgDgn<vector<T> >
    lnkCollateNonempty(const vector<FgDgn<T> > & nodes)
    {
        FgDgn<vector<T> >       ret = dg.addNode(vector<T>());
        dg.addLink(fgLnkCollateNonempty<T>,fgUints(nodes),fgUints(ret));
        return ret;
    }
};

// Global variable - very convenient as there will only ever be one GUI at a time:
extern FgGuiGraph g_gg;

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

template<class T>
struct  FgGuiWinVal         // Combine a window and a related node
{
    FgGuiPtr    win;
    FgDgn<T>    valN;
};

void
fgGuiImplStart(
    const FgString &        title,
    FgGuiPtr                def,
    const FgString &        storeDir,       // Directory in which to store gui state
    const FgGuiOptions &    options=FgGuiOptions());

#endif
