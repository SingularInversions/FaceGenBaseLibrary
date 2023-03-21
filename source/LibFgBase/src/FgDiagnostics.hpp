//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// USE:
//
// fgThrow("Unique translateable english message",state_information);
//
//      USE - CUSTOM TYPE (only when client must handle separately):
//
// struct FgExcCustom : public FgException
//    {explicit FgExcCustom(const String& m,const std::wstring& s) : FgException(m,s) {}; };
//    ...
// fgThrow<FgExcCustom>("Unique translateable english message",state_information);
//
//      USE - CHAINING CONTEXT:
//
// Clients can catch exceptions to add context to problem descriptions:
//
// catch (FgException & e)
// {
//     e.pushMsg("Another unique translatable english message",more_state_information);
//     throw;
// }
// 
// All exceptions should be caught by reference (see More Effective C++, Meyers).
//
// FGASSERT is retained release builds, FG_ASSERT_FAST is not.
// Use FGASSERT for unexpected errors (use 'fgThrow' otherwise)
//

#ifndef FGDIAGNOSTICS_HPP
#define FGDIAGNOSTICS_HPP

#include "FgTypes.hpp"

namespace Fg {

// code base version:
inline String       getSdkVersion(String const & sep) {return "3" + sep + "U" + sep + "0"; }

//! The FaceGen API exception type.
//! All exceptions raised by FG code are of this type, inherit from this type,
//! or inherit from std::exception.
struct  FgException
{
    struct  Context
    {
        // error messages are compile-time-known strings in english and translations can be looked up in language data files:
        String         msgEnglish;     // error message in english (ASCII)
        String         msgNative;      // error message in looked up local language (UTF8) (can be same as above)
        String         dataUtf8;       // Non-translatable UTF-8 data (eg. file names)

        Context(String const & m,String const & d) :
            msgEnglish(m), dataUtf8(d)
        {
            //TODO: look up msgNative from msgEnglish (if found) using application message dictionary
        }
        Context(String const & e,String const & n,String const & d) :
            msgEnglish(e), msgNative(n), dataUtf8(d) {}
    };
    Svec<Context>    contexts;       // From lowest stack level to to highest stack level context

    FgException() {}
    explicit FgException(Svec<Context> const & c) : contexts{c} {}
    FgException(const String & msg,const String & dataUtf8) : contexts(1,Context(msg,dataUtf8)) {}

    virtual ~FgException() {}

    bool                empty() const {return contexts.empty(); }
    //! add context to err when translated message needs to be looked up (if current lang not english):
    void                addContext(String const & english,String const & data = String());
    //! add context to err when translated message is already known (or empty if none needed):
    void                addContext(String const & english,String const & foreign,String const data)
    {
        contexts.emplace_back(english,foreign,data);
    }
    String             tr_message() const;
};

// Should be caught in an end-user context as a failed operation, rather than reported
// as an internal error. Examples include reading from a corrupt file, lack of drive space, etc.
struct  FgExceptionUserError : public FgException
{
    explicit
    FgExceptionUserError(const String & msg,const String & dataUtf8) : FgException(msg,dataUtf8)
    {}
};

struct  FgExceptionUserCancel : public FgException
{
    FgExceptionUserCancel()
    : FgException("User cancellation",String())
    {}
};

struct FgExceptionNotImplemented : public FgException
{
    FgExceptionNotImplemented()
        : FgException("Functionality not implemented on this platform",String()) {}

    explicit
    FgExceptionNotImplemented(const String & data)
        : FgException("Functionality not implemented on this platform",data) {}
};

inline void         fgThrow(const String & msg)
{
    throw FgException(msg,String());
}

// Only way to avoid VS warnings since some configs warn for unreachable code (returns),
// others for not returning a value ...
#if defined(__GNUC__) || (defined(_MSC_VER) && defined(_DEBUG))
#define FG_UNREACHABLE_RETURN(T) return T;
#else
#define FG_UNREACHABLE_RETURN(T)
#endif

// With visual studio the __FILE__ macro always includes the full path in release compiles,
// there is no way to specify otherwise. When this includes unicode, the literal becomes
// a wchar_t* instead of char*. Macros can be used to cast to wchar_t* in both cases, then
// dealt with but I haven't bothered; currently the source will not compile properly in a
// non-ascii path.

String         toFilePosString(char const *fname,int line);
// If you're trying to pass a UTF-8 'msg' here, you should probably be using 'fgThrow' instead:
void                fgAssert(char const * fname,int line,const String & msg = "");
// Crude warning system outputs to cout. Use when we don't want to throw in release distros.
// Currently just outputs to fgout but could add telemetry, special dev behaviour:
void                fgWarn(char const * fname,int line,const String & msg="");
void                fgWarn(const String & msg,const String & dataUtf8="");

// FgOut : Global multi-redirectable pretty-print output stream for diagnostic feedback:
// * Use 'fgout' instead of 'cout' everywhere.
// * Use 'fgnl' instead of 'endl' or "\n" everywhere you use 'fgout', and use at the BEGINNING
//   of each output line rather than the end.
// * Not threadsafe so use ostringstream output option for threads.
// * Default output is 'cout' for systems supporting CLI, 'ostringstream' otherwise (Android).
// * Use 'fgpop' and 'fgpush' to adjust the pretty-print indent level.
// * Will not work across DLL boundaries

// Pretty-print newline manipulators:
std::ostream &          fgnl(std::ostream& ss);
std::ostream &          fgpush(std::ostream& ss);
std::ostream &          fgpop(std::ostream& ss);
std::ostream &          fgreset(std::ostream& ss);  // Reset indent to zero (useful for exception handling)

// ADL won't find this for FgOut::operator<< below so it must be visible up front for clang:
template<class T>
std::ostream &          operator<<(std::ostream &,Svec<T> const &);

struct  FgOut
{
    FgOut();
    ~FgOut();

    // This is a unique global object:
    FgOut(FgOut const &) = delete;
    void operator=(FgOut const &) = delete;

    bool            setDefOut(bool b);          // Returns true if default output was initially enabled
    bool            defOutEnabled();            // As above. Non-const only for technical reasons.
    void            logFile(const String & fnameUtf8,bool appendFile=true,bool prependDate=true);
    void            logFileClose();
    void            push()
    {
        for (OStr & o : m_streams)
            ++o.indent;
    }
    void            pop()
    {
        for (OStr & o : m_streams)
            if (o.indent > 0)
                --o.indent;
    }
    uint            indentLevel() const
    {
        if (m_streams.empty())
            return 0;
        return uint(m_streams[0].indent);
    }
    void            setIndentLevel(uint);
    void            reset()
    {
        for (OStr & o : m_streams)
            o.indent = 0;
    }
    FgOut &         flush();
    template<typename T>
    FgOut &         operator<<(T const & arg)
    {
        // In this approach we stringize each arg for each output stream, which is perhaps
        // inefficent but also allows for ostreams with different settings:
        for (OStr & ostr : m_streams)
            (*ostr.pOStr) << arg;
        return *this;
    }
    // Manipulators are just passed through to each ostream:
    FgOut &         operator<<(std::ostream & (*manip)(std::ostream&));
    // add this ostream to the output list. Ignored if already in the list:
    void            addStream(std::ostream *,size_t indentLevel=0);
    // remove this ostream from the output list. Ignored if not in the list. Returns its current indent level,
    // or 0 if not in list:
    size_t          delStream(std::ostream *);

private:
    struct      OStr
    {
        std::ostream        *pOStr;
        size_t              indent = 0;     // Note that atomic cannot be copy constructed

        explicit OStr(std::ostream * p) : pOStr(p) {}
        OStr(std::ostream * os,size_t i) : pOStr{os}, indent{i} {}

        bool operator==(std::ostream const * r) const {return (pOStr == r); }
    };
    Svec<OStr>   m_streams;          // Defaults to point to 'cout' unless no CLI, then 'm_stringStream'.
    std::ostringstream  m_stringStream;     // Only used per 'm_stream' above

    std::ostream *      defOut();
};

extern FgOut      fgout;

struct  PushIndent
{
    size_t              depth = 1;

    explicit PushIndent(String const & label = String{})
    {
        if (label.empty())
            fgout << fgpush;
        else
            fgout << fgnl << label << std::flush << fgpush;
    }

    ~PushIndent() {pop(); }

    void                next(const String & nextLabel) const
    {
        if (depth > 0)
            fgout << fgpop;
        fgout << fgnl << nextLabel << std::flush;
        if (depth > 0)
            fgout << fgpush;
    }

    void                pop()
    {
        if (depth > 0) {
            fgout << fgpop;
            --depth;
        }
    }
};

}

// We use an 'if' 'else' structure for the macro to avoid the dangling 'else' bug.
// Leave off the semi-colon on the second line to force a compile error:
#define FGASSERT(X)                                                     \
    if(X) (void) 0;                                                     \
    else Fg::fgAssert(__FILE__,__LINE__)

#define FGASSERT1(X,msg)                                            \
    if(X) (void) 0;                                                     \
    else Fg::fgAssert(__FILE__,__LINE__,msg)

#ifdef _DEBUG

#define FGASSERT_FAST(X)                                                \
    if(X) (void) 0;                                                     \
    else Fg::fgAssert(__FILE__,__LINE__)

#define FGASSERT_FAST2(X,Y)                     \
    if(X) (void) 0;                             \
    else Fg::fgAssert(__FILE__,__LINE__,Y)          \

#else

#define FGASSERT_FAST(X)
#define FGASSERT_FAST2(X,Y)

#endif

// Use this instead of FGASSERT(false) to avoid warnings for constant conditionals:
#define FGASSERT_FALSE                                                  \
    Fg::fgAssert(__FILE__,__LINE__)

#define FGASSERT_FALSE1(msg)                                         \
    Fg::fgAssert(__FILE__,__LINE__,msg)

// printf-style debugging:

#define FG_HI std::cout << "\nHI! (" << __FILE__ << ": " << __LINE__ << ")" << std::flush

#define FG_HI1(X) std::cout << "\nHI! " << #X ": " << (X) << std::flush

#define FG_HI2(X,Y) std::cout << "\nHI! " << #X ": " << (X) << " " << #Y ": " << (Y) << std::flush

#define FG_HI3(X,Y,Z) std::cout << "\nHI! " << #X ": " << (X) << " "       \
         << #Y ": " << (Y) << " "                                           \
         << #Z ": " << (Z) << std::flush

#define FG_HI4(X,Y,Z,A) std::cout << "\nHI! " << #X ": " << (X) << " "     \
         << #Y ": " << (Y) << " "                                           \
         << #Z ": " << (Z) << " "                                           \
         << #A ": " << (A) << std::flush

#define FGWARN Fg::fgWarn(__FILE__,__LINE__)
#define FGWARN1(X) Fg::fgWarn(__FILE__,__LINE__,X)

#define FGOUT1(X) fgout << fgnl << #X ": " << fgpush << (X) << fgpop;
#define FGOUT2(X,Y) FGOUT1(X) FGOUT1(Y)
#define FGOUT3(X,Y,Z) FGOUT2(X,Y) FGOUT1(Z)
#define FGOUT4(X,Y,Z,A) FGOUT2(X,Y) FGOUT2(Z,A)

#endif
