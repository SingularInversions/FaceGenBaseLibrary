//
// Copyright (c) 2025 Singular Inversions Inc. (facegen.com)
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

DfInput::~DfInput()
{
    if (uncaught_exceptions() == 0) {       // Don't attempt to save state if we're in an exception
        if (data.has_value() && onDestruct) {
            try {onDestruct(data);}
            catch (...) {}                  // Destructors cannot throw
        }
    }
}

any const &         DfInput::getDataCref() const
{
    // Don't check if data is initialized here since client needs to be able to check:
    return data;
}

void                DfInput::addSink(const DfDPtr & snk) {sinks.push_back(snk); }

void                DfInput::makeDirty() const
{
//fgout << fgnl << "Dirty DfInput: " << cSignature(data) << fgpush;
    for (const DfDPtr & snk : sinks)
        // Structure is dynamic so some of the sinks may have expired (all cannot be or this node won't exist):
        if (!snk.expired())
            snk.lock()->markDirty();
//fgout << fgpop;
}

any &               DfInput::getDataRef() const
{
    makeDirty();
    return data;
}

void                DfInput::setToDefault() const
{
    if (dataDefault.has_value()) {
        data = dataDefault;     // Might re-allocate but what can you do
        makeDirty();
    }
}

bool                 DfOutput::printTime = false;

DfOutput::~DfOutput()
{
    if (printTime) {
        // Printing times doesn't take much CPU but might expose non-console users to unecessary exceptions
        if ((timeUsedMs > 1) && (isConsoleProgram())) {   // Cannot throw
            string      sig;
            for (DfNPtr const & source : sources)
                sig += cSignature(source->getDataCref()) + " ";
            fgout << fgnl << timeUsedMs << "ms : " << sig ;
        }
    }
}

// The update algorithm does not need to track visited nodes to avoid exponential re-visiting
// for highly connected graphs because the dirty bit already handles that:
void                DfOutput::update() const
{
    if (!dirty)
        return;
    // Change flag here because we want to mark clean even if there is an exception so that we
    // don't keep throwing the same exception:
    dirty = false;
//fgout << fgnl << "Update DfOutput: " << cSignature(data) << fgpush;
    for (const DfNPtr & src : sources)
        src->update();      // Ensure sources updated
//fgout << fgpop;
    try {
        Timer               timer;
        func(sources,data);
        timeUsedMs += timer.elapsedMilliseconds();
    }
    catch(FgException & e)
    {
        e.contexts.emplace_back("Executing DfOutput link",cSignature(data));
        throw;
    }
    catch(std::exception const & e)
    {
        FgException     ex("std::exception",e.what());
        ex.contexts.emplace_back("Executing DfOutput link",cSignature(data));
        throw ex;
    }
    catch(...)
    {
        fgThrow("Unknown exception executing DfOutput link",cSignature(data));
    }
}
void                DfOutput::markDirty() const
{
    // If an DfOutput is dirty, all of its dependents must be dirty too since markDirty() marks all
    // dependents and update() on any one of them would have forced an update on this one:
    if (dirty)
        return;
    dirty = true;
//fgout << fgnl << "Dirty DfOutput: " << cSignature(data) << fgpush;
    for (const DfDPtr & snk : sinks)
        // Structure is dynamic so some of the sinks may have expired (all cannot be or this node won't exist):
        if (!snk.expired())
            snk.lock()->markDirty();
//fgout << fgpop;
}

any const &         DfOutput::getDataCref() const
{
    update();
    return data;
}
void                DfOutput::clearSources()
{
    sources.clear();
    markDirty();
}
void                DfOutput::addSource(const DfNPtr & src)
{
    sources.push_back(src);
    markDirty();
}
void                DfOutput::addSink(const DfDPtr & snk)
{
    sinks.push_back(snk);
}

//void                DfSelect::update() const
//{
//    if (!dirty)
//        return;
//    dirty = false;
////fgout << fgnl << "Update DfSelect: " << fgpush;
//    size_t              sel = selN->getCref<size_t>();
//    FGASSERT(sel < sources.size());
//    sources[sel]->update();
////fgout << fgpop;
//}
//void                DfSelect::markDirty() const
//{
//    if (dirty)
//        return;
//    dirty = true;
////fgout << fgnl << "Dirty DfSelect: " << fgpush;
//    for (DfDPtr const & snk : sinks)
//        // Structure is dynamic so some of the sinks may have expired (all cannot be or this node won't exist):
//        if (!snk.expired())
//            snk.lock()->markDirty();
////fgout << fgpop;
//}
//any const &         DfSelect::getDataCref() const
//{
//    dirty = false;
//    size_t              sel = selN->getCref<size_t>();
//    FGASSERT(sel < sources.size());
//    return sources[sel]->getDataCref();
//}
//void                DfSelect::addSink(const DfDPtr & snk)
//{
//    sinks.push_back(snk);
//}

void                DfReceptor::update() const
{
    FGASSERT(src);
    if (!dirty)
        return;
//fgout << fgnl << "Update DfReceptor:" << fgpush;
    dirty = false;
    src->update();
//fgout << fgpop;
}
void                DfReceptor::markDirty() const
{
    if (dirty)
        return;
    dirty = true;
//fgout << fgnl << "Dirty DfReceptor:" << fgpush;
    for (const DfDPtr & snk : sinks)
        if (!snk.expired())
            snk.lock()->markDirty();
//fgout << fgpop;
}
any const &         DfReceptor::getDataCref() const
{
    update();
    return src->getDataCref();
}
void                DfReceptor::addSink(const DfDPtr & snk)
{
    sinks.push_back(snk);
}
void                DfReceptor::setSource(DfNPtr const & nptr)
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
        for (const DfNPtr & src : sources)
            src->update();
//fgout << fgpop;
        return true;
    }
    else
        return false;
}

void                addLink(const DfNPtr & src,const DfOPtr & snk)
{
    src->addSink(snk);
    snk->addSource(src);
}

DfFPtr             cUpdateFlag(DfNPtrs const & nptrs)
{
    DfFPtr        ret = std::make_shared<DirtyFlag>(nptrs);
    for (const DfNPtr & nptr : nptrs)
        nptr->addSink(ret);
    return ret;
}

void                setInputsToDefault(DfNPtrs const & nptrs)
{
    for (const DfNPtr & nptr : nptrs) {
        if (const DfInput * iptr = dynamic_cast<const DfInput*>(nptr.get()))
            iptr->setToDefault();
        else if (const DfOutput *optr = dynamic_cast<const DfOutput*>(nptr.get()))
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
    NPT<int>        n2 = linkN(Svec<NPT<int>>{n0,n1},cSum<int>);
    FGASSERT(n2.val() == 11);
    n0.ref() = 6;
    FGASSERT(n2.val() == 12);
    NPT<int>        n3 = link1(n2,[](int x){return x+2;});
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
