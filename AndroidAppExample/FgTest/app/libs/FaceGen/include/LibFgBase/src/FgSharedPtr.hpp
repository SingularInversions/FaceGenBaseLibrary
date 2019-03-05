//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Sohail Somani, Andrew Beatty
//
// Deprecated.
//
// Reference-counted pointer similar to shared_ptr. Not multithread safe.

#ifndef FGSHAREDPTR_HPP
#define FGSHAREDPTR_HPP

#include "FgStdLibs.hpp"
#include "FgDiagnostics.hpp"

template<typename T>
inline
void
fg_checked_delete(void *p)
{
    // This mechanism ensures that delete is only called on a complete type
    // (ie defined when compiling this code, rather than just declared):
    typedef char type_must_be_complete[ sizeof(T) ? 1: -1];
    (void) sizeof(type_must_be_complete);
    delete static_cast<T*>(p);
}

inline
void
fg_null_deleter(void *)
{}

template<typename T>
class   FgPtr
{
    typedef void(*deleter_fn_t)(void *);
    T *             m_p;
    std::size_t *   m_ref;
    // This is necessary in case the client casts this pointer to a parent class, as this will
    // otherwise result in compiler warnings upon deletion, and corruption if the parent does
    // not have a virtual destructor:
    deleter_fn_t    m_deleter;

public:
    template<typename Y>
    friend class FgPtr;

    FgPtr()
    : m_p(0), m_ref(0), m_deleter(fg_null_deleter)
    {}

    // Only pass a 'new' pointer here; never a pointer into a pre-existing object:
    explicit
    FgPtr(T * p)
    : m_p(p), m_ref(new std::size_t(1)), m_deleter(fg_checked_delete<T>)
    {}

    FgPtr(FgPtr<T> const & rhs)
    : m_p(0),m_ref(0),m_deleter(0)
    {copyFrom(rhs); }

    // Allow implicit conversion as long as the pointers themselves support implicit conversion.
    // For example child -> base ptr. This is one of the few cases where implicit conversion is OK.
    template<typename Y>
    FgPtr(FgPtr<Y> const & rhs)
    : m_p(0),m_ref(0),m_deleter(0)
    {copyFrom(rhs); }

    ~FgPtr()
    {reset(); }

    void reset()
    {
        if(m_ref && 0==decRef()) {
            delete m_ref;
            m_deleter(m_p);
            m_ref = 0;
            m_p = 0;
        }
    }

    void swap(FgPtr<T> & other)
    {
        std::swap(m_p,other.m_p);
        std::swap(m_ref,other.m_ref);
        std::swap(m_deleter,other.m_deleter);
    }

    template<typename Y>
    void reset(Y * p)
    {
        FGASSERT_FAST(p == 0 || p != m_p);
        FgPtr<T>(p).swap(*this);
    }

    T * get() const
    {
        FGASSERT_FAST(m_p);
        return m_p;
    }

    T * operator->() const
    {
        FGASSERT_FAST(m_p);
        return m_p;
    }

    T & operator*() const
    {
        FGASSERT_FAST(m_p);
        return *m_p;
    }

    FgPtr<T> &
    operator=(FgPtr<T> const & rhs)
    {
        copyFrom(rhs);
        return *this;
    }

    template<typename Y>
    FgPtr<T> &
    operator=(FgPtr<Y> const & rhs)
    {
        copyFrom(rhs);
        return *this;
    }

    operator bool() const
    {
        return m_p != 0;
    }

    std::size_t use_count() const
    {
        if(m_ref)
            return *m_ref;
        else
            return 0;
    }

private:
    template<typename Y>
    void copyFrom(FgPtr<Y> const & rhs)
    {
        reset();
        this->m_ref = rhs.m_ref;
        this->m_p = rhs.m_p;
        this->m_deleter = rhs.m_deleter;
        if(this->m_ref) addRef();
    }

    std::size_t addRef()
    {
        return ++(*m_ref);
    }
    std::size_t decRef()
    {
        return --(*m_ref);
    }
};

template<typename T>
FgPtr<T>
fgsp(const T & v)
{return FgPtr<T>(new T(v)); }

template<typename T>
FgPtr<T>
fgnew()
{
    return FgPtr<T>(new T);
}

template<typename T,
         typename A1>
FgPtr<T>
fgnew(const A1 & a1)
{
    return FgPtr<T>(new T(a1));
}

template<typename T,
         typename A1,
         typename A2>
FgPtr<T>
fgnew(const A1 & a1,const A2 & a2)
{
    return FgPtr<T>(new T(a1,a2));
}

template<typename T,
         typename A1,
         typename A2,
         typename A3>
FgPtr<T>
fgnew(const A1 & a1,const A2 & a2,const A3 & a3)
{
    return FgPtr<T>(new T(a1,a2,a3));
}

template<typename T,
         typename A1,
         typename A2,
         typename A3,
         typename A4>
FgPtr<T>
fgnew(const A1 & a1,const A2 & a2,const A3 & a3,const A4 & a4)
{
    return FgPtr<T>(new T(a1,a2,a3,a4));
}

template<typename T,
         typename A1,
         typename A2,
         typename A3,
         typename A4,
         typename A5>
FgPtr<T>
fgnew(const A1 & a1,const A2 & a2,const A3 & a3,const A4 & a4,const A5 & a5)
{
    return FgPtr<T>(new T(a1,a2,a3,a4,a5));
}

template<typename T,
         typename A1,
         typename A2,
         typename A3,
         typename A4,
         typename A5,
         typename A6>
FgPtr<T>
fgnew(const A1 & a1,const A2 & a2,const A3 & a3,const A4 & a4,const A5 & a5,const A6 & a6)
{
    return FgPtr<T>(new T(a1,a2,a3,a4,a5,a6));
}

template<typename T,typename A1,typename A2,typename A3,typename A4,typename A5,
         typename A6,typename A7>
FgPtr<T>
fgnew(
    const A1 & a1,const A2 & a2,const A3 & a3,const A4 & a4,const A5 & a5,const A6 & a6,
    const A7 & a7)
{
    return FgPtr<T>(new T(a1,a2,a3,a4,a5,a6,a7));
}

template<typename T,typename A1,typename A2,typename A3,typename A4,typename A5,
         typename A6,typename A7,typename A8>
FgPtr<T>
fgnew(
    const A1 & a1,const A2 & a2,const A3 & a3,const A4 & a4,const A5 & a5,const A6 & a6,
    const A7 & a7,const A8 & a8)
{
    return FgPtr<T>(new T(a1,a2,a3,a4,a5,a6,a7,a8));
}

template<typename T,typename A1,typename A2,typename A3,typename A4,typename A5,
         typename A6,typename A7,typename A8,typename A9>
FgPtr<T>
fgnew(
    const A1 & a1,const A2 & a2,const A3 & a3,const A4 & a4,const A5 & a5,const A6 & a6,
    const A7 & a7,const A8 & a8,const A9 & a9)
{
    return FgPtr<T>(new T(a1,a2,a3,a4,a5,a6,a7,a8,a9));
}

template<typename T,typename A1,typename A2,typename A3,typename A4,typename A5,
         typename A6,typename A7,typename A8,typename A9,typename A10>
FgPtr<T>
fgnew(
    const A1 & a1,const A2 & a2,const A3 & a3,const A4 & a4,const A5 & a5,const A6 & a6,
    const A7 & a7,const A8 & a8,const A9 & a9,const A10 & a10)
{
    return FgPtr<T>(new T(a1,a2,a3,a4,a5,a6,a7,a8,a9,a10));
}

template<typename T,typename A1,typename A2,typename A3,typename A4,typename A5,
         typename A6,typename A7,typename A8,typename A9,typename A10,typename A11>
FgPtr<T>
fgnew(
    const A1 & a1,const A2 & a2,const A3 & a3,const A4 & a4,const A5 & a5,const A6 & a6,
    const A7 & a7,const A8 & a8,const A9 & a9,const A10 & a10,const A11 & a11)
{
    return FgPtr<T>(new T(a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11));
}

#endif
