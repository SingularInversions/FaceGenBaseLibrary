//
// Coypright (c) 2022 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
//      USE:
//
// fgThrow("Unique translateable english message",state_information);
//
//      USE - CUSTOM TYPE (only when client must handle separately):
//
// struct FgExcCustom : public FgException
//    {explicit FgExcCustom(const std::string& m,const std::wstring& s) : FgException(m,s) {}; };
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

#ifndef FGEXCEPTION_HPP
#define FGEXCEPTION_HPP

#include "FgStdLibs.hpp"

namespace Fg {

//! The FaceGen API exception type.
//! All exceptions raised by FG code are of this type, inherit from this type,
//! or inherit from std::exception.
struct  FgException
{
    struct  Context
    {
        std::string         msgEnglish;     // Translatable english message (ASCII)
        std::string         msgNative;      // If language setting not english, translated message (UTF8)
        std::string         dataUtf8;       // Non-translatable UTF-8 data (eg. names)

        Context(std::string const & m,std::string const & d) :
            msgEnglish(m), dataUtf8(d)
        {
            //TODO: look up msgNative from msgEnglish (if found) using application message dictionary
        }
        Context(std::string const & e,std::string const & n,std::string const & d) :
            msgEnglish(e), msgNative(n), dataUtf8(d) {}
    };
    std::vector<Context>    contexts;       // From lowest stack level to to highest stack level context

    FgException() {}
    explicit FgException(std::vector<Context> const & c) : contexts{c} {}
    FgException(const std::string & msg,const std::string & dataUtf8)
        : contexts(1,Context(msg,dataUtf8))
    {}

    virtual ~FgException() {}

    bool                empty() const {return contexts.empty(); }
    //! add context to err when translated message needs to be looked up (if current lang not english):
    void                addContext(std::string const & english,std::string const & data = std::string());
    //! add context to err when translated message is already known (or empty if none needed):
    void                addContext(std::string const & english,std::string const & foreign,std::string const data)
    {
        contexts.emplace_back(english,foreign,data);
    }
    std::string             tr_message() const;
};

// Should be caught in an end-user context as a failed operation, rather than reported
// as an internal error. Examples include reading from a corrupt file, lack of drive space, etc.
struct  FgExceptionUserError : public FgException
{
    explicit
    FgExceptionUserError(const std::string & msg,const std::string & dataUtf8) : FgException(msg,dataUtf8)
    {}
};

struct  FgExceptionUserCancel : public FgException
{
    FgExceptionUserCancel()
    : FgException("User cancellation",std::string())
    {}
};

struct FgExceptionNotImplemented : public FgException
{
    FgExceptionNotImplemented()
        : FgException("Functionality not implemented on this platform",std::string()) {}

    explicit
    FgExceptionNotImplemented(const std::string & data)
        : FgException("Functionality not implemented on this platform",data) {}
};

inline void         fgThrow(const std::string & msg)
{
    throw FgException(msg,std::string());
}

// Only way to avoid VS warnings since some configs warn for unreachable code (returns),
// others for not returning a value ...
#if defined(__GNUC__) || (defined(_MSC_VER) && defined(_DEBUG))
#define FG_UNREACHABLE_RETURN(T) return T;
#else
#define FG_UNREACHABLE_RETURN(T)
#endif

}

#endif      // #ifndef FGEXCEPTION_HPP
