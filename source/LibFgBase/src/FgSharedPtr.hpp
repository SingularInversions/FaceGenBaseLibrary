//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Sohail Somani, Andrew Beatty
//

#ifndef FGSHAREDPTR_HPP
#define FGSHAREDPTR_HPP

#include "FgStdLibs.hpp"
#include "FgDiagnostics.hpp"

// This function ensures that delete is only called on a complete type
// as sizeof can only be called on a complete type
template<typename T>
inline
void
fg_checked_delete(void *p)
{
    typedef char type_must_be_complete[ sizeof(T) ? 1: -1];
    (void) sizeof(type_must_be_complete);
    delete static_cast<T*>(p);
}

inline
void
fg_null_deleter(void *)
{}

// Like boost::shared_ptr
template<typename T>
class   FgSharedPtr
{
    typedef void(*deleter_fn_t)(void *);
    T *             m_p;
    std::size_t *   m_ref;
    deleter_fn_t    m_deleter;

public:
    template<typename Y>
    friend class FgSharedPtr;

    FgSharedPtr():
        m_p(0),
        m_ref(0),
        m_deleter(fg_null_deleter)
    {}

    explicit FgSharedPtr(T * p):
        m_p(p),
        m_ref(new std::size_t(1)),
        m_deleter(fg_checked_delete<T>)
    {}

    FgSharedPtr(FgSharedPtr<T> const & rhs)
        :m_p(0),
         m_ref(0),
         m_deleter(0)
    {
        copyFrom(rhs);
    }

    // Allow implicit conversion as long as the pointers themselves support implicit conversion.
    // For example child -> base ptr. This is one of the few cases where implicit conversion is OK.
    template<typename Y>
    FgSharedPtr(FgSharedPtr<Y> const & rhs)
        :m_p(0),
         m_ref(0),
         m_deleter(0)

    {
        copyFrom(rhs);
    }

    ~FgSharedPtr()
    {
        reset();
    }

    void reset()
    {
        if(m_ref && 0==decRef())
        {
            delete m_ref;
            m_deleter(m_p);
            m_ref = 0;
            m_p = 0;
        }
    }

    void swap(FgSharedPtr<T> & other)
    {
        std::swap(m_p,other.m_p);
        std::swap(m_ref,other.m_ref);
        std::swap(m_deleter,other.m_deleter);
    }

    template<typename Y>
    void reset(Y * p)
    {
        FGASSERT_FAST(p == 0 || p != m_p);
        FgSharedPtr<T>(p).swap(*this);
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

    FgSharedPtr<T> &
    operator=(FgSharedPtr<T> const & rhs)
    {
        copyFrom(rhs);
        return *this;
    }

    template<typename Y>
    FgSharedPtr<T> &
    operator=(FgSharedPtr<Y> const & rhs)
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
    void copyFrom(FgSharedPtr<Y> const & rhs)
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
FgSharedPtr<T>
fgsp(const T & v)
{return FgSharedPtr<T>(new T(v)); }

template<typename T>
FgSharedPtr<T>
fgnew()
{
    return FgSharedPtr<T>(new T);
}

template<typename T,
         typename A1>
FgSharedPtr<T>
fgnew(const A1 & a1)
{
    return FgSharedPtr<T>(new T(a1));
}

template<typename T,
         typename A1,
         typename A2>
FgSharedPtr<T>
fgnew(const A1 & a1,const A2 & a2)
{
    return FgSharedPtr<T>(new T(a1,a2));
}

template<typename T,
         typename A1,
         typename A2,
         typename A3>
FgSharedPtr<T>
fgnew(const A1 & a1,const A2 & a2,const A3 & a3)
{
    return FgSharedPtr<T>(new T(a1,a2,a3));
}

template<typename T,
         typename A1,
         typename A2,
         typename A3,
         typename A4>
FgSharedPtr<T>
fgnew(const A1 & a1,const A2 & a2,const A3 & a3,const A4 & a4)
{
    return FgSharedPtr<T>(new T(a1,a2,a3,a4));
}

template<typename T,
         typename A1,
         typename A2,
         typename A3,
         typename A4,
         typename A5>
FgSharedPtr<T>
fgnew(const A1 & a1,const A2 & a2,const A3 & a3,const A4 & a4,const A5 & a5)
{
    return FgSharedPtr<T>(new T(a1,a2,a3,a4,a5));
}

template<typename T,
         typename A1,
         typename A2,
         typename A3,
         typename A4,
         typename A5,
         typename A6>
FgSharedPtr<T>
fgnew(const A1 & a1,const A2 & a2,const A3 & a3,const A4 & a4,const A5 & a5,const A6 & a6)
{
    return FgSharedPtr<T>(new T(a1,a2,a3,a4,a5,a6));
}

template<typename T,typename A1,typename A2,typename A3,typename A4,typename A5,
         typename A6,typename A7>
FgSharedPtr<T>
fgnew(
    const A1 & a1,const A2 & a2,const A3 & a3,const A4 & a4,const A5 & a5,const A6 & a6,
    const A7 & a7)
{
    return FgSharedPtr<T>(new T(a1,a2,a3,a4,a5,a6,a7));
}

template<typename T,typename A1,typename A2,typename A3,typename A4,typename A5,
         typename A6,typename A7,typename A8>
FgSharedPtr<T>
fgnew(
    const A1 & a1,const A2 & a2,const A3 & a3,const A4 & a4,const A5 & a5,const A6 & a6,
    const A7 & a7,const A8 & a8)
{
    return FgSharedPtr<T>(new T(a1,a2,a3,a4,a5,a6,a7,a8));
}

template<typename T,typename A1,typename A2,typename A3,typename A4,typename A5,
         typename A6,typename A7,typename A8,typename A9>
FgSharedPtr<T>
fgnew(
    const A1 & a1,const A2 & a2,const A3 & a3,const A4 & a4,const A5 & a5,const A6 & a6,
    const A7 & a7,const A8 & a8,const A9 & a9)
{
    return FgSharedPtr<T>(new T(a1,a2,a3,a4,a5,a6,a7,a8,a9));
}

template<typename T,typename A1,typename A2,typename A3,typename A4,typename A5,
         typename A6,typename A7,typename A8,typename A9,typename A10>
FgSharedPtr<T>
fgnew(
    const A1 & a1,const A2 & a2,const A3 & a3,const A4 & a4,const A5 & a5,const A6 & a6,
    const A7 & a7,const A8 & a8,const A9 & a9,const A10 & a10)
{
    return FgSharedPtr<T>(new T(a1,a2,a3,a4,a5,a6,a7,a8,a9,a10));
}

template<typename T,typename A1,typename A2,typename A3,typename A4,typename A5,
         typename A6,typename A7,typename A8,typename A9,typename A10,typename A11>
FgSharedPtr<T>
fgnew(
    const A1 & a1,const A2 & a2,const A3 & a3,const A4 & a4,const A5 & a5,const A6 & a6,
    const A7 & a7,const A8 & a8,const A9 & a9,const A10 & a10,const A11 & a11)
{
    return FgSharedPtr<T>(new T(a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11));
}

#endif
