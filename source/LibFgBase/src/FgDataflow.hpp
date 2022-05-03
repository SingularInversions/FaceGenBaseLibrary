//
// Coypright (c) 2022 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Dataflow graph with lazy updates for handling DAG data dependencies.
//
// DESIGN:
//
// * Not multithread safe.
// * Originally considered a bipartite graph of Values and Links but if Links have more than one
//   Value output then they can get invalidated outputs as the DAG changes which is a pain to deal
//   with. Constraining functions to only 1 output solves this, and can be then be more simply
//   designed with just a single node type.
// * Types used as data must have a default constructor
// * No need to check for valid data - all nodes should always contain a valid instance of their
//   type after proper dataflow graph setup.
//
// INVARIANTS:
//
// * Calling 'cref' or 'val' will always return an updated value reflecting the current DAG inputs
// * If a Node is dirty all of its dependent nodes are also dirty.
// * If a Node is not dirty all of the nodes it depends on are also not dirty.
// * Const member functions of Node will never change the DAG structure but the data values
//   may be updated.
//

#ifndef FGDATAFLOW_HPP
#define FGDATAFLOW_HPP

#include <boost/any.hpp>
#include "FgStdExtensions.hpp"
#include "FgString.hpp"
#include "FgFileSystem.hpp"

namespace Fg {

String
cSignature(boost::any const &);

struct  DfgDependent;
typedef std::weak_ptr<DfgDependent> DfgDPtr;

// Conceptually this is two interfaces; a dependency ('update') and a data container ('getData*'):
struct  DfgNode
{
    virtual ~DfgNode() {}
    virtual void update() const = 0;
    virtual boost::any const & getDataCref() const = 0;
    virtual void addSink(const DfgDPtr &) = 0;
};
typedef Sptr<DfgNode>       DfgNPtr;
typedef Svec<DfgNPtr>       DfgNPtrs;

struct  DfgDependent
{
    virtual ~DfgDependent() {}
    virtual void markDirty() const = 0;
};
typedef Svec<DfgDPtr>        DfgDPtrs;

struct  DfgInput : DfgNode
{
private:
    mutable boost::any          data;
    boost::any                  dataDefault;        // Can be empty if no default
    DfgDPtrs                    sinks;              // Can be empty
public:
    // Called with 'data' on destruct only if non-empty and 'data' non-empty. Can be used to save state:
    Sfun<void(boost::any const&)> onDestruct;

    DfgInput() {}
    template<class T> explicit DfgInput(T const & v) : data(v) {}

    virtual                 ~DfgInput();
    virtual void            update() const {}
    virtual boost::any const & getDataCref() const;
    virtual void            addSink(const DfgDPtr & snk);

    void                    makeDirty() const;
    boost::any &            getDataRef() const;
    template<class T> void  init(T const & val,bool setDefault=false)
    {data = val; if (setDefault) dataDefault = val; makeDirty(); }
    void                    setToDefault() const;      // Reset value to default if exists
};
typedef Sptr<DfgInput>      DfgIPtr;

typedef Sfun<void(DfgNPtrs const &,boost::any &)> DfgFunc;

struct  DfgOutput : DfgNode, DfgDependent
{
    DfgNPtrs                    sources;        // Empty only if function takes no args
    DfgDPtrs                    sinks;          // Empty if this value is a final output
    DfgFunc                     func;           // Must be defined. Calculate sinks from sources
    mutable boost::any          data;
    // Has data we depend on anywhere above this node in the graph been modified since 'func' last run:
    mutable bool                dirty {true};
    mutable uint64              timeUsedMs {0};

    virtual ~DfgOutput();
    virtual void update() const;
    virtual void markDirty() const;
    virtual boost::any const & getDataCref() const;
    virtual void addSink(const DfgDPtr & snk);

    DfgNPtrs const &    getSources() const {return sources; }
    void                clearSources();
    void                addSource(const DfgNPtr & src);
};
typedef Sptr<DfgOutput>     DfgOPtr;

// Receptors are handles that can point to different node types. They make dataflow creation code
// simpler since they can be passed as arguments (already allocated) without knowing what type
// of node they will be connected to. They can also be held by function objects for dynamic
// updates of the source (but they are not useful for dynamic updates within a DFG).
struct  DfgReceptor : DfgNode, DfgDependent
{
private:
    DfgNPtr                     src;            // Null until connected to Node
    DfgDPtrs                    sinks;          // Empty if nothing depends on this
    mutable bool                dirty = true;

public:
    virtual ~DfgReceptor() {}
    virtual void update() const;
    virtual void markDirty() const;
    virtual boost::any const & getDataCref() const;
    virtual void addSink(const DfgDPtr & snk);
    void setSource(DfgNPtr const & nptr);
};
typedef Sptr<DfgReceptor>   DfgRPtr;

// Allows client objects to keep track of their own dirty state based on one or more sources:
struct  DirtyFlag : DfgDependent
{
    DfgNPtrs                    sources;        // Cannot be empty
    mutable bool                dirty = true;

    explicit DirtyFlag(DfgNPtrs const & srcs) : sources(srcs) {}

    virtual ~DirtyFlag() {}
    virtual void markDirty() const;

    // Ensure dependencies updated and returns true if update calls were required:
    bool checkUpdate() const;
};

typedef Sptr<DirtyFlag>     DfgFPtr;

void addLink(const DfgNPtr & src,const DfgOPtr & snk);

// Typed versions for static type checking. Client should always use this form:

// Type-safe inputs:
// * All constructors allocate DfgInput and contained data so we can never dereference null
//   (although we may of course need to check if the contained data is empty).
//   This way we don't have to pass back loads of pointers from the functions creating the GUI parts
//   and assign them all to the right places; but can just pass them forward instead.
// * Use 'init' or 'initSaved' to initialize the DfgInput data & default later if default constructed.
// * Use value initialization or list initializers (to get around vexing parse) to define them fully
//   in a single statement.
template<class T>
struct  IPT
{
    DfgIPtr            ptr;                         // Never null and contained data never null
    IPT() : ptr(std::make_shared<DfgInput>(T{})) {}
    explicit IPT(const DfgIPtr & n) : ptr(n) {}
    explicit IPT(T const & val) : ptr(std::make_shared<DfgInput>(val)) {}
    explicit IPT(T const & val,T const & defaultVal) : ptr(std::make_shared<DfgInput>(val,defaultVal)) {}
    // Must use one of these two before attempting to access values as they will allocate the
    // 'any' with the value and also set the default value. If it's an input for a dynamic window
    // then it's possible it could be initialized more than once:
    void init(T const & val,bool setDefault=false) const {ptr->init(val,setDefault); }
    void initSaved(
        T const &           defaultVal,             // Will be the initial value if no valid one is stored
        String8 const &     storeFile,
        bool                binary=false)           // Store to binary format rather than XML for efficiency
    {
        FGASSERT(!storeFile.empty());
        ptr->init(defaultVal,true);
        if (binary) {
            if (pathExists(storeFile))
                loadBsaPBin(storeFile,ref(),false);
            ptr->onDestruct = [storeFile](boost::any const & v)
            {
                if (v.empty())
                    fgWarn("IPT onDestruct save with empty data",cSignature(v));
                else
                    saveBsaPBin(storeFile,boost::any_cast<T const &>(v),false);
            };
        }
        else {
            String8        fname = storeFile + ".xml";
            if (pathExists(fname))
                loadBsaXml(fname,ref(),false);
            ptr->onDestruct = [fname](boost::any const & v)
            {
                if (v.empty())
                    fgWarn("IPT onDestruct binary save with empty data",cSignature(v));
                else
                    saveBsaXml(fname,boost::any_cast<T const &>(v),false);
            };
        }
    }
    T const &       cref() const {return boost::any_cast<T const&>(ptr->getDataCref()); }
    T               val() const {return boost::any_cast<T>(ptr->getDataCref()); }
    // Value modification is still const because it's the value pointed to not the smart
    // pointer that is being modified. This is useful because we sometimes need this
    // object to be const - for example in a lambda capture:
    T &             ref() const {return boost::any_cast<T&>(ptr->getDataRef()); }
    void set(T const & val) const {ref() = val; }           // Prefer assignment below for visual clarity
};

template<class T>
struct  OPT
{
    DfgOPtr            ptr;
    OPT() {}
    explicit OPT(const DfgOPtr & o) : ptr(o) {}
    T const &       cref() const {return boost::any_cast<T const&>(ptr->getDataCref()); }
    T               val() const {return boost::any_cast<T>(ptr->getDataCref()); }
};

// Receptors are allocated automatically and 'ptr' should never be changed:
template<class T>
struct  RPT
{
    DfgRPtr            ptr;                            // Never null but what the DfgReceptor points to may be null.
    RPT() {ptr = std::make_shared<DfgReceptor>(); }    // Always allocated
    T const &       cref() const {return boost::any_cast<T const&>(ptr->getDataCref()); }
    T               val() const {return boost::any_cast<T>(ptr->getDataCref()); }
};

template<class T>
struct  NPT
{
    DfgNPtr            ptr;
    NPT() {}
    NPT(const IPT<T> & ipt) : ptr(ipt.ptr) {}
    NPT(const OPT<T> & opt) : ptr(opt.ptr) {}
    NPT(const RPT<T> & rpt) : ptr(rpt.ptr) {}
    explicit NPT(const DfgNPtr & n) : ptr(n) {}
    T const &       cref() const {return boost::any_cast<T const&>(ptr->getDataCref()); }
    T               val() const {return boost::any_cast<T>(ptr->getDataCref()); }
    // This will only return a valid pointer if the Node happens to be an DfgInput, otherwise, nullptr:
    T*              valPtr() const
    {
        DfgInput *     iptr = dynamic_cast<DfgInput*>(ptr.get());
        if (iptr)
            return &boost::any_cast<T&>(iptr->getDataRef());
        else
            return nullptr;
    }
};

template<class T>
void
connect(RPT<T> const & rpt,NPT<T> const & npt)
{
    rpt.ptr->setSource(npt.ptr);
    npt.ptr->addSink(rpt.ptr);
}

// Can use instead of constructor for type deduction from argument:
template<class T>
IPT<T>
makeIPT(T const & val)
{return IPT<T>(std::make_shared<DfgInput>(val)); }

template<class T>
IPT<T>
makeSavedIPT(
    T const &           defaultVal,             // Will be the initial value if no valid one is stored
    String8 const &     storeFile,
    bool                binary=false)           // Store to binary format rather than XML for efficiency
{
    IPT<T>          ret;
    ret.initSaved(defaultVal,storeFile,binary);
    return ret;
}

template<class T>
IPT<T>
makeSavedIPTEub(
    T const &           defaultVal,             // Will be the initial value if no valid one is stored
    String8 const &     storeFile,
    T                   eub)
{
    IPT<T>          ret;
    ret.initSaved(defaultVal,storeFile);
    if (!(ret.cref() < eub))
        ret.set(defaultVal);
    return ret;
}

DfgFPtr makeUpdateFlag(DfgNPtrs const & nptrs);
// Cannot make use of implicit conversion to NPT because of above overload:
template<class T> DfgFPtr makeUpdateFlag(const IPT<T> & n) {return makeUpdateFlag(svec<DfgNPtr>(n.ptr)); }
template<class T> DfgFPtr makeUpdateFlag(const OPT<T> & n) {return makeUpdateFlag(svec<DfgNPtr>(n.ptr)); }
template<class T> DfgFPtr makeUpdateFlag(const NPT<T> & n) {return makeUpdateFlag(svec<DfgNPtr>(n.ptr)); }
template<class T> DfgFPtr makeUpdateFlag(const RPT<T> & n) {return makeUpdateFlag(svec<DfgNPtr>(n.ptr)); }
template<class T0,class T1> DfgFPtr makeUpdateFlag(const T0 & n0,const T1 & n1)
{return makeUpdateFlag(svec<DfgNPtr>(n0.ptr,n1.ptr)); }

template<class T>
struct      NPTF
{
    NPT<T>          node;
    DfgFPtr         flag;

    NPTF() {}
    NPTF(NPT<T> const & n) : node{n}, flag{makeUpdateFlag(n)} {}

    void
    operator=(NPT<T> const & n)
    {
        node = n;
        flag = makeUpdateFlag(n);
    }

    bool
    checkUpdate() const
    {return flag->checkUpdate(); }

    T const &       cref() const
    {
        flag->checkUpdate();
        return node.cref();
    }
};

// Traverses up the tree to set all inputs to the default value:
void setInputsToDefault(DfgNPtrs const &);

template<class T>
void
adapterCollate(DfgNPtrs const & srcs,boost::any & snk)
{
    Svec<T> &    out = boost::any_cast<Svec<T> &>(snk);
    out.resize(srcs.size());
    for (size_t ii=0; ii<out.size(); ++ii)
        out[ii] = boost::any_cast<T const &>(srcs[ii]->getDataCref());
}

template<class T>
OPT<Svec<T>>
linkCollate(const Svec<NPT<T>> & ins = Svec<NPT<T>>())
{
    Sptr<DfgOutput>     op = std::make_shared<DfgOutput>();
    op->data = Svec<T>();
    op->func = adapterCollate<T>;
    op->sources.reserve(ins.size());
    for (const NPT<T> & in : ins) {
        op->sources.push_back(in.ptr);
        in.ptr->addSink(op);
    }
    return OPT<Svec<T>>(op);
}
template<class T>
OPT<Svec<T>>
linkCollate(const Svec<IPT<T>> & ins)
{return linkCollate(mapConvert<IPT<T>,NPT<T>>(ins)); }

template<class In,class Out>
void
adapterN(DfgNPtrs const & srcs,boost::any & snk,Sfun<Out(const Svec<In> &)> fn)
{
    Svec<In>     args;
    args.reserve(srcs.size());
    for (const DfgNPtr & src : srcs)
        args.push_back(boost::any_cast<const In &>(src->getDataCref()));
    snk = fn(args);
}

template<class In,class Out>
OPT<Out>
linkN(const Svec<NPT<In>> & ins,const Sfun<Out(const Svec<In> &)> & fn)
{
    Sptr<DfgOutput>     op = std::make_shared<DfgOutput>();
    op->func = std::bind(adapterN<In,Out>,std::placeholders::_1,std::placeholders::_2,fn);
    op->sources.reserve(ins.size());
    for (NPT<In> const & in : ins) {
        op->sources.push_back(in.ptr);
        in.ptr->addSink(op);
    }
    return OPT<Out>(op);
}
template<class In,class Out>
OPT<Out>
linkN(const Svec<IPT<In>> & ins,const Sfun<Out(const Svec<In> &)> & fn)
{return linkN(mapConvert<IPT<In>,NPT<In>>(ins),fn); }

template<class In,class Out>
OPT<Out>
link1(NPT<In> const & in,Sfun<Out(const In &)> const & fn)
{
    Sptr<DfgOutput>     op = std::make_shared<DfgOutput>();
    op->func = [fn](DfgNPtrs const & srcs,boost::any & snk)
    {
        FGASSERT(srcs.size() == 1);
        In const &          in = boost::any_cast<const In &>(srcs[0]->getDataCref());
        snk = fn(in);
    };
    op->sources.push_back(in.ptr);
    in.ptr->addSink(op);
    return OPT<Out>(op);
}

template<class In,class Out>
OPT<Out>
link1_(NPT<In> const & in,const Sfun<void(const In &,Out &)> & fn)
{
    Sptr<DfgOutput>     op = std::make_shared<DfgOutput>();
    op->data = Out();       // Must be instantiated for output by reference
    op->func = [fn](DfgNPtrs const & srcs,boost::any & snk)
    {
        FGASSERT(srcs.size() == 1);
        const In &      in = boost::any_cast<const In &>(srcs[0]->getDataCref());
        Out &           out = boost::any_cast<Out &>(snk);
        fn(in,out);
    };
    op->sources.push_back(in.ptr);
    in.ptr->addSink(op);
    return OPT<Out>(op);
}

template<class Out,class In0,class In1>
OPT<Out>
link2(const NPT<In0> & in0,const NPT<In1> & in1,const Sfun<Out(const In0 &,const In1 &)> & fn)
{
    Sptr<DfgOutput>     op = std::make_shared<DfgOutput>();
    op->func = [fn](DfgNPtrs const & srcs,boost::any & snk)
    {
        FGASSERT(srcs.size() == 2);
        const In0 &     in0 = boost::any_cast<const In0 &>(srcs[0]->getDataCref());
        const In1 &     in1 = boost::any_cast<const In1 &>(srcs[1]->getDataCref());
        snk = fn(in0,in1);
    };
    op->sources.push_back(in0.ptr);
    op->sources.push_back(in1.ptr);
    in0.ptr->addSink(op);
    in1.ptr->addSink(op);
    return OPT<Out>(op);
}

template<class Out,class In0,class In1>
OPT<Out>
link2_(
    const NPT<In0> & in0,const NPT<In1> & in1,
    const Sfun<void(const In0 &,const In1 &,Out &)> & fn)
{
    Sptr<DfgOutput>     op = std::make_shared<DfgOutput>();
    op->data = Out();       // Must be instantiated for output by reference
    op->func = [fn](DfgNPtrs const & srcs,boost::any & snk)
    {
        FGASSERT(srcs.size() == 2);
        const In0 &     in0 = boost::any_cast<const In0 &>(srcs[0]->getDataCref());
        const In1 &     in1 = boost::any_cast<const In1 &>(srcs[1]->getDataCref());
        Out &           out = boost::any_cast<Out &>(snk);
        fn(in0,in1,out);
    };
    op->sources.push_back(in0.ptr);
    op->sources.push_back(in1.ptr);
    in0.ptr->addSink(op);
    in1.ptr->addSink(op);
    return OPT<Out>(op);
}

template<class Out,class In0,class In1,class In2>
OPT<Out>
link3(
    const NPT<In0> & in0,const NPT<In1> & in1,const NPT<In2> & in2,
    const Sfun<Out(const In0 &,const In1 &,const In2 &)> & fn)
{
    Sptr<DfgOutput>     op = std::make_shared<DfgOutput>();
    op->func = [fn](DfgNPtrs const & srcs,boost::any & snk)
    {
        FGASSERT(srcs.size() == 3);
        const In0 &     in0 = boost::any_cast<const In0 &>(srcs[0]->getDataCref());
        const In1 &     in1 = boost::any_cast<const In1 &>(srcs[1]->getDataCref());
        const In2 &     in2 = boost::any_cast<const In2 &>(srcs[2]->getDataCref());
        snk = fn(in0,in1,in2);
    };
    op->sources.push_back(in0.ptr);
    op->sources.push_back(in1.ptr);
    op->sources.push_back(in2.ptr);
    in0.ptr->addSink(op);
    in1.ptr->addSink(op);
    in2.ptr->addSink(op);
    return OPT<Out>(op);
}

template<class Out,class In0,class In1,class In2>
OPT<Out>
link3_(
    const NPT<In0> & in0,const NPT<In1> & in1,const NPT<In2> & in2,
    const Sfun<void(const In0 &,const In1 &,const In2 &,Out &)> & fn)
{
    Sptr<DfgOutput>     op = std::make_shared<DfgOutput>();
    op->data = Out();       // Must be instantiated for output by reference
    op->func = [fn](DfgNPtrs const & srcs,boost::any & snk)
    {
        FGASSERT(srcs.size() == 3);
        const In0 &     in0 = boost::any_cast<const In0 &>(srcs[0]->getDataCref());
        const In1 &     in1 = boost::any_cast<const In1 &>(srcs[1]->getDataCref());
        const In2 &     in2 = boost::any_cast<const In2 &>(srcs[2]->getDataCref());
        Out &           out = boost::any_cast<Out &>(snk);
        fn(in0,in1,in2,out);
    };
    op->sources.push_back(in0.ptr);
    op->sources.push_back(in1.ptr);
    op->sources.push_back(in2.ptr);
    in0.ptr->addSink(op);
    in1.ptr->addSink(op);
    in2.ptr->addSink(op);
    return OPT<Out>(op);
}

template<class Out,class In0,class In1,class In2,class In3>
OPT<Out>
link4(
    const NPT<In0> & in0,const NPT<In1> & in1,const NPT<In2> & in2,const NPT<In3> & in3,
    const Sfun<Out(const In0 &,const In1 &,const In2 &,const In3 &)> & fn)
{
    Sptr<DfgOutput>     op = std::make_shared<DfgOutput>();
    op->func = [fn](DfgNPtrs const & srcs,boost::any & snk)
    {
        FGASSERT(srcs.size() == 4);
        const In0 &     in0 = boost::any_cast<const In0 &>(srcs[0]->getDataCref());
        const In1 &     in1 = boost::any_cast<const In1 &>(srcs[1]->getDataCref());
        const In2 &     in2 = boost::any_cast<const In2 &>(srcs[2]->getDataCref());
        const In3 &     in3 = boost::any_cast<const In3 &>(srcs[3]->getDataCref());
        snk = fn(in0,in1,in2,in3);
    };
    op->sources.push_back(in0.ptr);
    op->sources.push_back(in1.ptr);
    op->sources.push_back(in2.ptr);
    op->sources.push_back(in3.ptr);
    in0.ptr->addSink(op);
    in1.ptr->addSink(op);
    in2.ptr->addSink(op);
    in3.ptr->addSink(op);
    return OPT<Out>(op);
}

template<class Out,class In0,class In1,class In2,class In3>
OPT<Out>
link4_(
    const NPT<In0> & in0,const NPT<In1> & in1,const NPT<In2> & in2,const NPT<In3> & in3,
    const Sfun<void(const In0 &,const In1 &,const In2 &,const In3 &,Out &)> & fn)
{
    Sptr<DfgOutput>     op = std::make_shared<DfgOutput>();
    op->data = Out();       // Must be instantiated for output by reference
    op->func = [fn](DfgNPtrs const & srcs,boost::any & snk)
    {
        FGASSERT(srcs.size() == 4);
        const In0 &     in0 = boost::any_cast<const In0 &>(srcs[0]->getDataCref());
        const In1 &     in1 = boost::any_cast<const In1 &>(srcs[1]->getDataCref());
        const In2 &     in2 = boost::any_cast<const In2 &>(srcs[2]->getDataCref());
        const In3 &     in3 = boost::any_cast<const In3 &>(srcs[3]->getDataCref());
        Out &           out = boost::any_cast<Out &>(snk);
        fn(in0,in1,in2,in3,out);
    };
    op->sources.push_back(in0.ptr);
    op->sources.push_back(in1.ptr);
    op->sources.push_back(in2.ptr);
    op->sources.push_back(in3.ptr);
    in0.ptr->addSink(op);
    in1.ptr->addSink(op);
    in2.ptr->addSink(op);
    in3.ptr->addSink(op);
    return OPT<Out>(op);
}

template<class Out,class In0,class In1,class In2,class In3,class In4>
OPT<Out>
link5(
    const NPT<In0> & in0,const NPT<In1> & in1,const NPT<In2> & in2,const NPT<In3> & in3,const NPT<In4> & in4,
    const Sfun<Out(const In0 &,const In1 &,const In2 &,const In3 &,const In4 &)> & fn)
{
    Sptr<DfgOutput>     op = std::make_shared<DfgOutput>();
    op->func = [fn](DfgNPtrs const & srcs,boost::any & snk)
    {
        FGASSERT(srcs.size() == 5);
        const In0 &     in0 = boost::any_cast<const In0 &>(srcs[0]->getDataCref());
        const In1 &     in1 = boost::any_cast<const In1 &>(srcs[1]->getDataCref());
        const In2 &     in2 = boost::any_cast<const In2 &>(srcs[2]->getDataCref());
        const In3 &     in3 = boost::any_cast<const In3 &>(srcs[3]->getDataCref());
        const In4 &     in4 = boost::any_cast<const In4 &>(srcs[4]->getDataCref());
        snk = fn(in0,in1,in2,in3,in4);
    };
    op->sources.push_back(in0.ptr);
    op->sources.push_back(in1.ptr);
    op->sources.push_back(in2.ptr);
    op->sources.push_back(in3.ptr);
    op->sources.push_back(in4.ptr);
    in0.ptr->addSink(op);
    in1.ptr->addSink(op);
    in2.ptr->addSink(op);
    in3.ptr->addSink(op);
    in4.ptr->addSink(op);
    return OPT<Out>(op);
}

template<class T>
OPT<T>
linkSelect(Svec<NPT<T>> const & inNs,NPT<size_t> selN)
{
    Sptr<DfgOutput>     op = std::make_shared<DfgOutput>();
    op->func = [](DfgNPtrs const & srcs,boost::any & snk)
        {
            FGASSERT(srcs.size()>1);
            size_t              sz = srcs.size()-1,
                                idx = boost::any_cast<size_t>(srcs[0]->getDataCref());
            FGASSERT(idx < sz);
            // We don't need to touch the other inputs as they will be updated when needed,
            // and the top-down dirty propogation continues from any of the inputs regardless
            // of the selection value (although this could perhaps be optimized if required):
            snk = boost::any_cast<T const &>(srcs[idx+1]->getDataCref());
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
OPT<T>
linkOptional(NPT<T> valN,NPT<bool> optN)
{
    return link2<T,T,bool>(valN,optN,[](T const & val,bool const & opt)
        {
            if (opt)
                return val;
            else
                return T();
        });
}

}

#endif

// */
