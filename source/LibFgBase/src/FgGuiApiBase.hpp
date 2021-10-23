//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// API DESIGN:
//
// Functions creating windows with associated dataflow input node prefer to accept those
// nodes as an argument, rather than create them, for 2 reasons:
// * IPTs can be created in a number of ways (saved? default?) and we don't want to add
//   that complexity to each window creation funciton.
// * IPTs created in the function must be returned along with the window pointer, making
//   for inconvenient return-by-structures.
// * IPTs are easy to create by the client inline in the function arguments and can be
//   passed forward for assignment because the dataflow node is allocated by default construction.

#ifndef FGGUIAPIBASE_HPP
#define FGGUIAPIBASE_HPP

#include "FgDataflow.hpp"
#include "FgMetaFormat.hpp"
#include "FgImageBase.hpp"

namespace Fg {

// Returns true if GUI is supported on this platform (GUI calls do nothing otherwise):
bool
isGuiSupported();

// Set up this data structure for application error handling (eg. report to server):
struct  GuiExceptHandler
{
    String8                        appNameVer;     // Full name of application plus version
    // Client-defined error reporting. Can be null.
    // Accepts error message, returns true if reported, false otherwise (so default dialog can be shown):
    std::function<bool(String8)> reportError;
    // Prepended to error message and displayed if 'reportError' == NULL or 'reportError' returns false:
    String8                        reportFailMsg;
};

extern GuiExceptHandler         g_guiDiagHandler;

struct  GuiBaseImpl;

typedef std::shared_ptr<GuiBaseImpl>    GuiImplPtr;
typedef Svec<GuiImplPtr>         GuiImplPtrs;

struct  GuiBase
{
    virtual ~GuiBase() {};         // Don't leak

    // Originally used the CTRP to avoid pasting a typed implementation of this in each subclass,
    // but this didn't work in a namespace since the forward declaration had to be within the
    // templated function and MSVC has a compiler bug that puts said declaration in the global
    // namespace. Plus CRTP is ugly. Then discovered that a global template function doesn't work
    // either because the specialization must be defined before use. So now each class declares
    // its specific global get instance function and instantiates this member using that:
    virtual
    std::shared_ptr<GuiBaseImpl>    getInstance() = 0;
};

typedef std::shared_ptr<GuiBase>    GuiPtr;
typedef Svec<GuiPtr>                GuiPtrs;
typedef Svec<GuiPtrs>               GuiPtrss;

template<class T>
std::shared_ptr<GuiBase>
guiMakePtr(T const & stackVal)
{return std::make_shared<T>(stackVal); }

struct  GuiEvent
{
    void *              handle;     // OS-specific handle to event for triggering main event-driven loop
    Sfun<void()>       handler;    // Function to handle event
};

struct  GuiKeyHandle
{
    char                key;        // Only visible keys handled for now
    Sfun<void()>       handler;
};

struct  GuiOptions
{
    Svec<GuiEvent>      events;
    Svec<GuiKeyHandle>  keyHandlers;
    Sfun<void()>        onUpdate;       // Used to store to undo stack
};

template<class T>
struct  GuiVal           // Combine a window and a related node
{
    NPT<T>              valN;
    GuiPtr              win;

    GuiVal() {}
    GuiVal(const NPT<T> & v,const GuiPtr & w) : valN(v), win(w) {}
};

// Defined in OS-specific code:
void
guiStartImpl(
    NPT<String8>                titleN,
    GuiPtr                      gui,
    String8 const &             store,          // Directory in which to store state
    GuiOptions const &          options=GuiOptions());

// Send message to terminate GUI. Defined in OS-specific code. TODO: make it not global:
void
guiQuit();

}

#endif
