//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Oct 14, 2005
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
#include "FgString.hpp"

//! The FaceGen API exception type.
//! All exceptions raised by FG code are of this type, inherit from this type,
//! or inherit from std::exception.
struct  FgException
{
    struct  Context
    {
        std::string     msg;        // In english
        FgString        data;       // Non-translatable

        Context(const std::string & m,const FgString & d)
            : msg(m), data(d)
        {}

        FgString
        trans() const;

        FgString
        noTrans() const;
    };
    std::vector<Context>    m_ct;

    virtual
    ~FgException()
    {}

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
    FgException(
        const std::string & msg,
        const FgString &    data=FgString())
        : m_ct(1,Context(msg,data))
    {}

        /**
            Push some more exception information onto this exception
            instance. The usage is similar to that of the constructors.
        */
    void
    pushMsg(
        const std::string & msg,
        const FgString &    data=FgString())
    {
        m_ct.push_back(Context(msg,data));
    }

    FgString
    tr_message() const;

    FgString
    no_tr_message() const;
};

struct  FgExceptionUserCancel : public FgException
{
    FgExceptionUserCancel()
    : FgException("User cancellation")
    {}
};

struct FgExceptionNotImplemented : public FgException
{
    FgExceptionNotImplemented() :
        FgException("Functionality not implemented on this platform") {}
};

inline 
void 
fgThrow(const std::string & msg)
{ 
    throw FgException(msg);
}

inline
void
fgThrow(FgException const & e)
{
    throw e;
}

template<typename FgExType>
inline
void
fgThrow(const std::string & msg)
{
    throw FgExType(msg);
}

inline 
void 
fgThrow(const std::string & msg,
        const FgString &str) 
{
    throw FgException(msg,str); 
}

template<typename FgExType>
inline
void
fgThrow(const std::string & msg,
        const FgString & str)
{
    throw FgExType(msg,str);
}

inline
void
fgThrowNotImplemented()
{
    throw FgExceptionNotImplemented();
}

#endif      // #ifndef FGEXCEPTION_HPP
