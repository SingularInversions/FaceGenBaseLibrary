//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
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
// Clients can make intermediate exception catches to add context to problem descriptions:
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
        std::string     msg;            // In english
        std::string     dataUtf8;       // Non-translatable UTF-8

        Context(const std::string & m,const std::string & d) : msg(m), dataUtf8(d) {}

        std::string                 // UTF-8
        trans() const;

        std::string                 // UTF-8
        noTrans() const;
    };
    std::vector<Context>    m_ct;

    virtual
    ~FgException()
    {}

    FgException() {}
        /**
           Construct an exception with a message and append a second
           string to a translation of the message. For example:

           \begincode
             throw FgException("Could not open file", some_file_name);
           \endcode

           The second argument is appended with a " : " separator. So
           in the above instance, the final message looks like:

           Could not open file : some_file_name

           Where the first part of the phrase is appropriately translated.
        */
    FgException(const std::string & msg,const std::string & dataUtf8)
        : m_ct(1,Context(msg,dataUtf8))
    {}

        /**
            Push some more exception information onto this exception
            instance. The usage is similar to that of the constructors.
        */
    void
    pushMsg(
        const std::string & msg,
        const std::string & dataUtf8 = std::string())
    {
        m_ct.push_back(Context(msg,dataUtf8));
    }

    std::string
    tr_message() const;

    std::string
    no_tr_message() const;
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

inline void fgThrow(const std::string & msg)
{throw FgException(msg,std::string()); }

// Only way to avoid VS warnings since some configs warn for unreachable code (returns),
// others for not returning a value ...
#if defined(__GNUC__) || (defined(_MSC_VER) && defined(_DEBUG))
#define FG_UNREACHABLE_RETURN(T) return T;
#else
#define FG_UNREACHABLE_RETURN(T)
#endif

}

#endif      // #ifndef FGEXCEPTION_HPP
