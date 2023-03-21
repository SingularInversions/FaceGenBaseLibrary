//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgDataflow.hpp"
#include "FgCommand.hpp"
#include "FgTime.hpp"

using namespace std;

namespace Fg {

String              cSignature(any const & data)
{
    String              ret = data.type().name();
    if (beginsWith(ret,"struct "))
        return cRest(ret,7);
    else if (beginsWith(ret,"class "))
        return cRest(ret,6);
    else
        return ret;
}

DfgInput::~DfgInput()
{
    // boost serialization doesn't work properly in an exception and we shouldn't call another
    // function that could throw (ie. the file save) in this context anyway:
    if (uncaught_exceptions() == 0) {
        if (data.has_value() && onDestruct) {
            try {onDestruct(data);}
            catch (...) {}      // Destructors cannot throw
        }
    }
}

any const &         DfgInput::getDataCref() const
{
    // Don't check if data is initialized here since client needs to be able to check:
    return data;
}

void                DfgInput::addSink(const DfgDPtr & snk) {sinks.push_back(snk); }

void                DfgInput::makeDirty() const
{
//fgout << fgnl << "Dirty DfgInput: " << cSignature(data) << fgpush;
    for (const DfgDPtr & snk : sinks)
        // Structure is dynamic so some of the sinks may have expired (all cannot be or this node won't exist):
        if (!snk.expired())
            snk.lock()->markDirty();
//fgout << fgpop;
}

any &               DfgInput::getDataRef() const
{
    makeDirty();
    return data;
}

void                DfgInput::setToDefault() const
{
    if (dataDefault.has_value()) {
        data = dataDefault;     // Might re-allocate but what can you do
        makeDirty();
    }
}

bool                 DfgOutput::printTime = false;

DfgOutput::~DfgOutput()
{
    if (printTime) {
        // Printing times doesn't take much CPU but might expose non-console users to unecessary exceptions
        if ((timeUsedMs > 1) && (isConsoleProgram())) {   // Cannot throw
            string      sig;
            for (DfgNPtr const & source : sources)
                sig += cSignature(source->getDataCref()) + " ";
            fgout << fgnl << timeUsedMs << " : " << sig ;
        }
    }
}

// The update algorithm does not need to track visited nodes to avoid exponential re-visiting
// for highly connected graphs because the dirty bit already handles that:
void                DfgOutput::update() const
{
    if (!dirty)
        return;
    // Change flag here because we want to mark clean even if there is an exception so that we
    // don't keep throwing the same exception:
    dirty = false;
//fgout << fgnl << "Update DfgOutput: " << cSignature(data) << fgpush;
    for (const DfgNPtr & src : sources)
        src->update();      // Ensure sources updated
//fgout << fgpop;
    try {
        Timer               timer;
        func(sources,data);
        timeUsedMs += timer.elapsedMilliseconds();
    }
    catch(FgException & e)
    {
        e.addContext("Executing DfgOutput link",cSignature(data));
        throw;
    }
    catch(std::exception const & e)
    {
        FgException     ex("std::exception",e.what());
        ex.addContext("Executing DfgOutput link",cSignature(data));
        throw ex;
    }
    catch(...)
    {
        fgThrow("Unknown exception executing DfgOutput link",cSignature(data));
    }
}
void                DfgOutput::markDirty() const
{
    // If an DfgOutput is dirty, all of its dependents must be dirty too since markDirty() marks all
    // dependents and update() on any one of them would have forced an update on this one:
    if (dirty)
        return;
    dirty = true;
//fgout << fgnl << "Dirty DfgOutput: " << cSignature(data) << fgpush;
    for (const DfgDPtr & snk : sinks)
        // Structure is dynamic so some of the sinks may have expired (all cannot be or this node won't exist):
        if (!snk.expired())
            snk.lock()->markDirty();
//fgout << fgpop;
}

any const &         DfgOutput::getDataCref() const
{
    update();
    return data;
}
void                DfgOutput::clearSources()
{
    sources.clear();
    markDirty();
}
void                DfgOutput::addSource(const DfgNPtr & src)
{
    sources.push_back(src);
    markDirty();
}
void                DfgOutput::addSink(const DfgDPtr & snk)
{
    sinks.push_back(snk);
}

void                DfgReceptor::update() const
{
    FGASSERT(src);
    if (!dirty)
        return;
//fgout << fgnl << "Update DfgReceptor:" << fgpush;
    dirty = false;
    src->update();
//fgout << fgpop;
}
void                DfgReceptor::markDirty() const
{
    if (dirty)
        return;
    dirty = true;
//fgout << fgnl << "Dirty DfgReceptor:" << fgpush;
    for (const DfgDPtr & snk : sinks)
        if (!snk.expired())
            snk.lock()->markDirty();
//fgout << fgpop;
}
any const &         DfgReceptor::getDataCref() const
{
    update();
    return src->getDataCref();
}
void                DfgReceptor::addSink(const DfgDPtr & snk)
{
    sinks.push_back(snk);
}
void                DfgReceptor::setSource(DfgNPtr const & nptr)
{
    FGASSERT(!src);
    src = nptr;
    markDirty();
}

void                DirtyFlag::markDirty() const {dirty = true; }

bool                DirtyFlag::checkUpdate() const
{
    if (dirty) {
//fgout << fgnl << "Update Flag:" << fgpush;
        // Update flag first in case sources throw an exception - avoids repeated throws:
        dirty = false;
        for (const DfgNPtr & src : sources)
            src->update();
//fgout << fgpop;
        return true;
    }
    else
        return false;
}

void                addLink(const DfgNPtr & src,const DfgOPtr & snk)
{
    src->addSink(snk);
    snk->addSource(src);
}

DfgFPtr             makeUpdateFlag(DfgNPtrs const & nptrs)
{
    DfgFPtr        ret = std::make_shared<DirtyFlag>(nptrs);
    for (const DfgNPtr & nptr : nptrs)
        nptr->addSink(ret);
    return ret;
}

void                setInputsToDefault(DfgNPtrs const & nptrs)
{
    for (const DfgNPtr & nptr : nptrs) {
        if (const DfgInput * iptr = dynamic_cast<const DfgInput*>(nptr.get()))
            iptr->setToDefault();
        else if (const DfgOutput *optr = dynamic_cast<const DfgOutput*>(nptr.get()))
            setInputsToDefault(optr->sources);
        else if (const DirtyFlag *dptr = dynamic_cast<const DirtyFlag*>(nptr.get()))
            setInputsToDefault(dptr->sources);
        else
            fgThrow("setInputsToDefault unhandled type");
    }
}

void                testDataflow(CLArgs const &)
{
    IPT<int>        n0 = makeIPT(5),
                    n1 = makeIPT(6);
    NPT<int>        n2 = linkN<int,int>(svec<NPT<int> >(n0,n1),cSum<int>);
    FGASSERT(n2.cref() == 11);
    n0.ref() = 6;
    FGASSERT(n2.val() == 12);
    NPT<int>        n3 = link1<int,int>(n2,[](int x){return x+2;});
    FGASSERT(n3.val() == 14);
}

// Old code for turning DAG into DOT into PDF:

//string
//fgDepGraph2Dot(
//    const ... lg,
//    string const &                          label,
//    const Uints &                    paramInds)
//{
//    ostringstream    ret;
//    ret << "digraph DepGraph\n{\n";
//    ret << "  graph [label=\"" << label << "\"];\n  {\n    node [shape=box]\n";
//    for (size_t ii=0; ii<paramInds.size(); ++ii)
//        ret << "    \"" << lg.nodeData(paramInds[ii]).name(paramInds[ii]) << "\" [shape=doubleoctagon]\n";
//    for (uint ii=0; ii<lg.numLinks(); ii++) {
//        Uints    sources = lg.linkSources(ii);
//        Uints    sinks = lg.linkSinks(ii);
//        // Skip stubs (used by GUI) as they obsfucate:
//        if ((sinks.size() == 1) && (lg.nodeData(sinks[0]).label == "stub"))
//            continue;
//        ret << "    L" << ii << " [shape=oval];\n";
//        for (uint jj=0; jj<sources.size(); jj++)
//            ret << "    \"" << lg.nodeData(sources[jj]).name(sources[jj]) 
//                << "\" -> L" << ii << ";\n";
//        for (uint jj=0; jj<sinks.size(); jj++)
//            ret << "    L" << ii << " -> \""
//                << lg.nodeData(sinks[jj]).name(sinks[jj]) << "\";\n";
//    }
//    ret << "  }\n";
//    ret << "}\n";
//    return ret.str();
//}
//
//void
//fgDotToPdf(
//    const std::string &     dotFile,
//    const std::string &     pdfFile)
//{
//    // Uses 'dot.exe' from Graphviz (2.38 as of last test use):
//    string      cmd = "dot -Tpdf -o" + pdfFile + " " + dotFile;
//#ifndef FG_SANDBOX
//    if (system(cmd.c_str()) != 0)
//        fgout << fgnl << "Command failed.";
//#endif
//}
//
//void
//fgDepGraph2Pdf(
//    const ...  lg,
//    const std::string &                     rootName,
//    const Uints &                    paramInds)
//{
//    Ofstream      ofs(rootName+".dot");
//    ofs << fgDepGraph2Dot(lg,"",paramInds);
//    ofs.close();
//    fgDotToPdf(rootName+".dot",rootName+".pdf");
//}

}

// */
