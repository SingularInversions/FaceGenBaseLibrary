//
// Copyright (c) 2025 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Directed acyclic computation graph with lazy updates for automatically minimizing recomputation.
//
// USE:
// 
// * Automate minimal recomputation of values for fixed structure. Structural updates require re-creation of the graph.
// 
// DESIGN:
//
// * Types used as data must have a default constructor
// * No need to check for valid data - all nodes should always contain a valid instance of their
//   type after proper dataflow graph setup.
// * Not multithread safe.
// * Originally considered a bipartite graph of Values and Links but if Links have more than one
//   Value output then they can get invalidated outputs as the DAG changes which is a pain to deal
//   with. Constraining functions to only 1 output solves this, and can be then be more simply
//   designed with just a single node type.
//
// INVARIANTS:
//
// * Calling 'val' will always return an updated value reflecting the current DAG inputs
// * If a Node is dirty all of its dependent nodes are also dirty.
// * If a Node is not dirty all of the nodes it depends on are also not dirty.
// * Const member functions of Node will never change the DAG structure but the data values
//   may be updated.
//

#ifndef FGDATAFLOW_HPP
#define FGDATAFLOW_HPP

#include "FgFile.hpp"

namespace Fg {

String              cSignature(std::any const &);

struct      DfDependent;
typedef std::weak_ptr<DfDependent> DfDPtr;

// Conceptually this is two interfaces; a dependency ('update') and a data container ('getData*'):
struct      DfNode
{
    virtual ~DfNode() {}
    virtual void            update() const = 0;
    virtual std::any const & getDataCref() const = 0;
    virtual void            addSink(const DfDPtr &) = 0;
    template<class T>
    T const &               getCref() const
    {
        try {return std::any_cast<T const &>(getDataCref()); }
        catch (std::exception const & e) {fgThrow(e.what(),typeid(T).name()); }
        return std::any_cast<T const &>(getDataCref());     // never executed; avoid gcc warning
    }
};
typedef Sptr<DfNode>        DfNPtr;
typedef Svec<DfNPtr>        DfNPtrs;

struct      DfDependent
{
    virtual ~DfDependent() {}
    virtual void            markDirty() const = 0;
};
typedef Svec<DfDPtr>        DfDPtrs;

struct      DfInput : DfNode
{
private:
    mutable std::any        data;
    std::any                dataDefault;        // Can be empty if no default
    DfDPtrs                 sinks;              // Can be empty
public:
    // Called with 'data' on destruct only if non-empty and 'data' non-empty. Can be used to save state:
    Sfun<void(std::any const&)> onDestruct;

    DfInput() {}
    template<class T> explicit DfInput(T const & v) : data(v) {}

    virtual                 ~DfInput();
    virtual void            update() const {}
    virtual std::any const & getDataCref() const;
    virtual void            addSink(const DfDPtr & snk);

    void                    makeDirty() const;
    std::any &              getDataRef() const;
    template<class T>
    void                    init(T const & val,bool setDefault=false)
    {
        data = val;
        if (setDefault)
            dataDefault = val;
        makeDirty();
    }
    void                    setToDefault() const;      // Reset value to default if exists
};
typedef Sptr<DfInput>      DfIPtr;

typedef Sfun<void(DfNPtrs const &,std::any &)> DfFunc;

struct      DfOutput : DfNode, DfDependent
{
    DfNPtrs                     sources;        // Empty only if function takes no args
    DfDPtrs                     sinks;          // Empty if this value is a final output
    DfFunc                      func;           // Must be defined. Calculate sinks from sources
    mutable std::any            data;
    // Has data we depend on anywhere above this node in the graph been modified since 'func' last run:
    mutable bool                dirty {true};
    mutable uint64              timeUsedMs {0};
    static bool                 printTime;
    DfOutput() {}
    explicit DfOutput(DfNPtrs const & s) : sources{s} {}

    virtual ~DfOutput();
    virtual void update() const;
    virtual void markDirty() const;
    virtual std::any const & getDataCref() const;
    virtual void addSink(const DfDPtr & snk);

    DfNPtrs const &     getSources() const {return sources; }
    void                clearSources();
    void                addSource(const DfNPtr & src);
};
typedef Sptr<DfOutput>     DfOPtr;

//struct      DfSelect : DfNode, DfDependent
//{
//    DfNPtr                      selN;           // must point to a node containing size_t to select between the below:
//    DfNPtrs                     sources;        // Must have size >= 1
//    DfDPtrs                     sinks;          // Empty if this value is a final output
//    // Has data we depend on anywhere above this node in the graph been modified since 'func' last run:
//    mutable bool                dirty {true};
//    DfSelect() {}
//    explicit DfSelect(DfNPtrs const & s) : sources{s} {}
//
//    virtual ~DfSelect() {};
//    virtual void update() const;
//    virtual void markDirty() const;
//    virtual std::any const & getDataCref() const;
//    virtual void addSink(const DfDPtr & snk);
//
//    DfNPtrs const &     getSources() const {return sources; }
//};
//typedef Sptr<DfOutput>     DfOPtr;

// Receptors are handles that can point to different node types. They make dataflow creation code
// simpler since they can be passed as arguments (already allocated) without knowing what type
// of node they will be connected to. They can also be held by function objects for dynamic
// updates of the source (but they are not useful for dynamic updates within a DFG).
struct      DfReceptor : DfNode, DfDependent
{
private:
    DfNPtr                      src;            // Null until connected to Node
    DfDPtrs                     sinks;          // Empty if nothing depends on this
    mutable bool                dirty = true;

public:
    virtual ~DfReceptor() {}
    virtual void            update() const;
    virtual void            markDirty() const;
    virtual std::any const & getDataCref() const;
    virtual void            addSink(const DfDPtr & snk);
    void                    setSource(DfNPtr const & nptr);
};
typedef Sptr<DfReceptor>   DfRPtr;

// Allows client objects to keep track of their own dirty state based on one or more sources:
struct      DirtyFlag : DfDependent
{
    DfNPtrs                    sources;        // Cannot be empty
    mutable bool                dirty = true;

    explicit DirtyFlag(DfNPtrs const & srcs) : sources(srcs) {}
    virtual ~DirtyFlag() {}
    virtual void            markDirty() const;

    // Ensure dependencies updated and returns true if update calls were required:
    bool                    checkUpdate() const;
};

typedef Sptr<DirtyFlag>     DfFPtr;

void                addLink(const DfNPtr & src,const DfOPtr & snk);

// Typed versions for static type checking. Client should always use this form:

// Type-safe inputs:
// * All constructors allocate DfInput and contained data so we can never dereference null
//   (although we may of course need to check if the contained data is empty).
//   This way we don't have to pass back loads of pointers from the functions creating the GUI parts
//   and assign them all to the right places; but can just pass them forward instead.
// * Use 'init' or 'initSaved' to initialize the DfInput data & default later if default constructed.
// * Use value initialization or list initializers (to get around vexing parse) to define them fully
//   in a single statement.
template<class T>
struct      IPT
{
    typedef T           Type;
    DfIPtr              ptr;                         // Never null and contained data never null
    // we cannot have a ctor from type 'DfIPtr' as it will convert integers to an sptr.
    IPT() : ptr(std::make_shared<DfInput>(T{})) {}
    explicit IPT(T const & val) : ptr{std::make_shared<DfInput>(val)} {}
    explicit IPT(T const & val,T const & defaultVal) : ptr{std::make_shared<DfInput>(val,defaultVal)} {}
    // Must use one of these two before attempting to access values as they will allocate the
    // 'any' with the value and also set the default value. If it's an input for a dynamic window
    // then it's possible it could be initialized more than once:
    void                init(T const & val,bool setDefault=false) const {ptr->init(val,setDefault); }
    void                initSaved(
        T const &           defaultVal,             // Will be the initial value if no valid one is stored
        String8 const &     storeFile,
        bool                binary=false)           // Store to binary format rather than XML for efficiency
    {
        FGASSERT(!storeFile.empty());
        ptr->init(defaultVal,true);
        if (binary) {
            String8         fname = storeFile + ".bin";
            try {
                T           tmp = loadMessage<T>(fname);        // two-stage to avoid corruption if error
                set(tmp);
            }
            catch (...) {}
            ptr->onDestruct = [fname](std::any const & v)
            {
                if (v.has_value()) {
                    try {saveMessage(std::any_cast<T const &>(v),fname); }
                    catch (...) {}
                }
                else
                    fgWarn("IPT onDestruct save with empty data",cSignature(v));
            };
        }
        else {
            String8         fname = storeFile + ".txt";
            try {
                T           tmp = dsrlzText<T>(loadRawString(fname));
                set(tmp);
            }
            catch (...) {}
            ptr->onDestruct = [fname](std::any const & v)
            {
                if (v.has_value()) {
                    try {saveRaw(srlzText(std::any_cast<T const &>(v)),fname); }
                    catch (...) {}
                }
                else
                    fgWarn("IPT onDestruct binary save with empty data",cSignature(v));
            };
        }
    }
    T const &           val() const {return std::any_cast<T const &>(ptr->getDataCref()); }
    // Value modification is still const because it's the value pointed to not the smart
    // pointer that is being modified. This is useful because we sometimes need this
    // object to be const - for example in a lambda capture:
    T &                 ref() const {return std::any_cast<T&>(ptr->getDataRef()); }
    inline void         set(T const & val) const {ref() = val; }
};

// Can use instead of constructor for type deduction from argument:
template<class T>
IPT<T>              makeIPT(T const & v) {return IPT<T>{v}; }
template<class T>
Svec<IPT<T>>        makeIPTs(Svec<T> const & v) {return mapCall(v,makeIPT<T>); }

template<class T>
IPT<T>              makeSavedIPT(
    T const &           defaultVal,             // Will be the initial value if no valid one is stored
    String8 const &     storeFile,
    bool                binary=false)           // Store to binary format rather than XML for efficiency
{
    IPT<T>          ret;
    ret.initSaved(defaultVal,storeFile,binary);
    return ret;
}

template<class T>
IPT<T>              makeSavedIPTEub(
    T const &           defaultVal,             // Will be the initial value if no valid one is stored
    String8 const &     storeFile,
    T                   eub)
{
    IPT<T>          ret;
    ret.initSaved(defaultVal,storeFile);
    if (!(ret.val() < eub))
        ret.set(defaultVal);
    return ret;
}

// handy for function arguments to bind:
template<class T>
void                setIPT(IPT<T> ipt,T val) {ipt.set(val); }
template<class T>
void                setIPTsConst(Svec<IPT<T>> ipts,T val)
{
    for (IPT<T> & ipt : ipts)
        ipt.set(val);
}
template<class T>
void                setIPTs(Svec<IPT<T>> ipts,Svec<T> vals)
{
    FGASSERT(ipts.size() == vals.size());
    for (size_t ii=0; ii<ipts.size(); ++ii)
        ipts[ii].set(vals[ii]);
}

template<class T>
struct  OPT
{
    typedef T           Type;
    DfOPtr              ptr;
    OPT() {}
    explicit OPT(const DfOPtr & o) : ptr(o) {}
    T const &       val() const {return std::any_cast<T const &>(ptr->getDataCref()); }
};

// Receptors are allocated automatically and 'ptr' should never be changed:
template<class T>
struct  RPT
{
    typedef T           Type;
    DfRPtr              ptr;                            // Never null but what the DfReceptor points to may be null.
    RPT() {ptr = std::make_shared<DfReceptor>(); }     // Always allocated
    T const &           val() const {return std::any_cast<T const &>(ptr->getDataCref()); }
};

template<class T>
struct  NPT
{
    typedef T           Type;
    DfNPtr              ptr;
    NPT() {}
    NPT(const IPT<T> & ipt) : ptr(ipt.ptr) {}
    NPT(const OPT<T> & opt) : ptr(opt.ptr) {}
    NPT(const RPT<T> & rpt) : ptr(rpt.ptr) {}
    explicit NPT(const DfNPtr & n) : ptr(n) {}
    T const &       val() const {return std::any_cast<T const &>(ptr->getDataCref()); }
    // This will only return a valid pointer if the Node happens to be an DfInput, otherwise, nullptr:
    T*              valPtr() const
    {
        DfInput *     iptr = dynamic_cast<DfInput*>(ptr.get());
        if (iptr)
            return &std::any_cast<T&>(iptr->getDataRef());
        else
            return nullptr;
    }
    bool                valid() const {return bool{ptr}; }
};

template<class T>
void                connect(RPT<T> const & rpt,NPT<T> const & npt)
{
    rpt.ptr->setSource(npt.ptr);
    npt.ptr->addSink(rpt.ptr);
}

DfFPtr             cUpdateFlag(DfNPtrs const & nptrs);
template<class T>
DfFPtr             cUpdateFlagT(T const & n) {return cUpdateFlag(DfNPtrs{n.ptr}); }
template<class T,class U>
DfFPtr             cUpdateFlagT(T const & t,U const & u) {return cUpdateFlag(DfNPtrs{t.ptr,u.ptr}); }

template<class T>
struct      NPTF
{
    NPT<T>          node;
    DfFPtr         flag;

    NPTF() {}
    NPTF(NPT<T> const & n) : node{n}, flag{cUpdateFlagT(n)} {}

    void            operator=(NPT<T> const & n)
    {
        node = n;
        flag = cUpdateFlagT(n);
    }

    bool            checkUpdate() const {return flag->checkUpdate(); }

    T const &       val() const
    {
        flag->checkUpdate();
        return node.val();
    }
};

// Traverses up the tree to set all inputs to the default value:
void                setInputsToDefault(DfNPtrs const &);

// in all the linkX classes below, type safety is assured, but also the types must be present in order to
// properly specify the function call in the lambda that is stored with each node:

template<class I>
auto                linkCollate(Svec<I> const & ins)
{
    typedef typename I::Type        T;
    Sptr<DfOutput>     op = std::make_shared<DfOutput>();
    op->data = Svec<T>{};
    op->func = [](DfNPtrs const & srcs,std::any & snk)
    {
        Svec<T> &           out = std::any_cast<Svec<T> &>(snk);
        out.resize(srcs.size());
        for (size_t ii=0; ii<out.size(); ++ii)
            out[ii] = std::any_cast<T const &>(srcs[ii]->getDataCref());
    };
    op->sources.reserve(ins.size());
    for (auto const & in : ins) {
        op->sources.push_back(in.ptr);
        in.ptr->addSink(op);
    }
    return OPT<Svec<T>>(op);
}

// makes copies of each input, don't use for big objects:
template<class I,class C>
auto                linkN(Svec<I> const & ins,C const & fn)
{
    typedef typename I::Type        T;
    typedef decltype(fn(Svec<T>{})) R;
    Sptr<DfOutput>     op = std::make_shared<DfOutput>();
    op->func = [fn](DfNPtrs const & srcs,std::any & snk)
    {
        Svec<T>             args; args.reserve(srcs.size());
        for (DfNPtr const & s : srcs)
            args.push_back(s->getCref<T>());
        snk = fn(args);
    };
    op->sources.reserve(ins.size());
    for (auto const & in : ins) {
        op->sources.push_back(in.ptr);
        in.ptr->addSink(op);
    }
    return OPT<R>(op);
}

// as above but passes pointers so better for big objects:
template<class I,class C>
auto                linkNPtrs(Svec<I> const & ins,C const & fn)
{
    typedef typename I::Type        T;
    typedef decltype(fn(Svec<T const *>{})) R;
    Sptr<DfOutput>      op = std::make_shared<DfOutput>();
    op->func = [fn](DfNPtrs const & srcs,std::any & snk)
    {
        Svec<T const *>     args; args.reserve(srcs.size());
        for (DfNPtr const & s : srcs)
            args.push_back(&(s->getCref<T>()));
        snk = fn(args);
    };
    op->sources.reserve(ins.size());
    for (auto const & in : ins) {
        op->sources.push_back(in.ptr);
        in.ptr->addSink(op);
    }
    return OPT<R>(op);
}

template<class I0,class C>
auto                link1(const I0 & i0,C const & fn)
{
    typedef typename I0::Type T0;
    typedef decltype(fn(T0{})) R;
    Sptr<DfOutput>     op = std::make_shared<DfOutput>(DfNPtrs{i0.ptr});
    op->func = [fn](DfNPtrs const & srcs,std::any & snk)
    {
        FGASSERT(srcs.size() == 1);
        T0 const &          i0 = srcs[0]->getCref<T0>();
        snk = fn(i0);
    };
    i0.ptr->addSink(op);
    return OPT<R>(op);
}

template<class I0,class I1,class C>
auto                link2(const I0 & i0,const I1 & i1,C const & fn)
{
    typedef typename I0::Type T0;
    typedef typename I1::Type T1;
    typedef decltype(fn(T0{},T1{})) R;
    Sptr<DfOutput>     op = std::make_shared<DfOutput>(DfNPtrs{i0.ptr,i1.ptr});
    op->func = [fn](DfNPtrs const & srcs,std::any & snk)
    {
        FGASSERT(srcs.size() == 2);
        T0 const &          i0 = srcs[0]->getCref<T0>();
        T1 const &          i1 = srcs[1]->getCref<T1>();
        snk = fn(i0,i1);
    };
    i0.ptr->addSink(op);
    i1.ptr->addSink(op);
    return OPT<R>(op);
}

template<class I0,class I1,class I2,class C>
auto                link3(const I0 & i0,const I1 & i1,const I2 & i2,C const & fn)
{
    typedef typename I0::Type T0;
    typedef typename I1::Type T1;
    typedef typename I2::Type T2;
    typedef decltype(fn(T0{},T1{},T2{})) R;
    Sptr<DfOutput>     op = std::make_shared<DfOutput>(DfNPtrs{i0.ptr,i1.ptr,i2.ptr});
    op->func = [fn](DfNPtrs const & srcs,std::any & snk)
    {
        FGASSERT(srcs.size() == 3);
        T0 const &          i0 = srcs[0]->getCref<T0>();
        T1 const &          i1 = srcs[1]->getCref<T1>();
        T2 const &          i2 = srcs[2]->getCref<T2>();
        snk = fn(i0,i1,i2);
    };
    i0.ptr->addSink(op);
    i1.ptr->addSink(op);
    i2.ptr->addSink(op);
    return OPT<R>(op);
}

template<class I0,class I1,class I2,class I3,class C>
auto                link4(const I0 & i0,const I1 & i1,const I2 & i2,const I3 & i3,C const & fn)
{
    typedef typename I0::Type T0;
    typedef typename I1::Type T1;
    typedef typename I2::Type T2;
    typedef typename I3::Type T3;
    typedef decltype(fn(T0{},T1{},T2{},T3{})) R;
    Sptr<DfOutput>     op = std::make_shared<DfOutput>(DfNPtrs{i0.ptr,i1.ptr,i2.ptr,i3.ptr});
    op->func = [fn](DfNPtrs const & srcs,std::any & snk)
    {
        FGASSERT(srcs.size() == 4);
        T0 const &          i0 = srcs[0]->getCref<T0>();
        T1 const &          i1 = srcs[1]->getCref<T1>();
        T2 const &          i2 = srcs[2]->getCref<T2>();
        T3 const &          i3 = srcs[3]->getCref<T3>();
        snk = fn(i0,i1,i2,i3);
    };
    i0.ptr->addSink(op);
    i1.ptr->addSink(op);
    i2.ptr->addSink(op);
    i3.ptr->addSink(op);
    return OPT<R>(op);
}

template<class I0,class I1,class I2,class I3,class I4,class C>
auto                link5(const I0 & i0,const I1 & i1,const I2 & i2,const I3 & i3,const I4 & i4,C const & fn)
{
    typedef typename I0::Type T0;
    typedef typename I1::Type T1;
    typedef typename I2::Type T2;
    typedef typename I3::Type T3;
    typedef typename I4::Type T4;
    typedef decltype(fn(T0{},T1{},T2{},T3{},T4{})) R;
    Sptr<DfOutput>     op = std::make_shared<DfOutput>(DfNPtrs{i0.ptr,i1.ptr,i2.ptr,i3.ptr,i4.ptr});
    op->func = [fn](DfNPtrs const & srcs,std::any & snk)
    {
        FGASSERT(srcs.size() == 5);
        T0 const &          i0 = srcs[0]->getCref<T0>();
        T1 const &          i1 = srcs[1]->getCref<T1>();
        T2 const &          i2 = srcs[2]->getCref<T2>();
        T3 const &          i3 = srcs[3]->getCref<T3>();
        T4 const &          i4 = srcs[4]->getCref<T4>();
        snk = fn(i0,i1,i2,i3,i4);
    };
    i0.ptr->addSink(op);
    i1.ptr->addSink(op);
    i2.ptr->addSink(op);
    i3.ptr->addSink(op);
    i4.ptr->addSink(op);
    return OPT<R>(op);
}

template<class T>
OPT<T>              linkSelect(Svec<NPT<T>> const & inNs,NPT<size_t> selN)
{
    Sptr<DfOutput>     op = std::make_shared<DfOutput>();
    op->func = [](DfNPtrs const & srcs,std::any & snk)
        {
            FGASSERT(srcs.size()>1);
            size_t              sz = srcs.size()-1,
                                idx = std::any_cast<size_t>(srcs[0]->getDataCref());
            FGASSERT(idx < sz);
            // We don't need to touch the other inputs as they will be updated when needed,
            // and the top-down dirty propogation continues from any of the inputs regardless
            // of the selection value (although this could perhaps be optimized if required):
            snk = std::any_cast<T const &>(srcs[idx+1]->getDataCref());
        };
    op->sources.reserve(inNs.size()+1);
    op->sources.push_back(selN.ptr);
    selN.ptr->addSink(op);
    for (NPT<T> const & in : inNs) {
        op->sources.push_back(in.ptr);
        in.ptr->addSink(op);
    }
    return OPT<T>(op);
}

// If option is true, returns 'val', otherwise return empty T():
template<class T>
OPT<T>              linkOptional(NPT<T> valN,NPT<bool> optN)
{
    return link2(valN,optN,[](T const & val,bool const & opt){return opt ? val : T{}; });
}

}

#endif

// */
