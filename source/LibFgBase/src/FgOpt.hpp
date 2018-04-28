//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     May 21, 2009
//
// Optional-value type extender.
//
// * Simpler than std::optional - carries around a default constructed value when not valid.
//
// * To avoid additional memory use for numerical types, use 'FgValid' but note that
//   numeric_limits<T>::max() is the special 'invalid' value.
//
//  * Otherwise, use 'FgOpt' which adds a bool to keep track.
//

#ifndef FGOPT_HPP
#define FGOPT_HPP

#include "FgStdLibs.hpp"
#include "FgSerialize.hpp"
#include "FgDiagnostics.hpp"

template<typename T>
class   FgOpt
{
public:
    FgOpt()
    : m_valid(false)
    {}

    FgOpt(const T & v)
    : m_valid(true), m_val(v)
    {}

    FgOpt &
    operator=(const T & v)
    {m_val=v; m_valid=true; return *this; }

    bool
    valid() const
    {return m_valid; }

    void
    invalidate()
    {m_valid = false; }

    T &
    ref()
    {FGASSERT(m_valid); return m_val; }

    const T &
    val() const
    {FGASSERT(m_valid); return m_val; }

    template<typename U>
    FgOpt<U>
    cast()
    {
        if (m_valid)
            return FgOpt<U>(U(m_val));
        return FgOpt<U>();
    }

    bool
    operator==(const FgOpt<T> & rhs) const
    {
        if (m_valid && rhs.m_valid)
            return (m_val == rhs.m_val);
        return (!m_valid && !rhs.m_valid);
    }

    bool
    operator!=(const FgOpt<T> & rhs) const
    {
        if (m_valid && rhs.m_valid)
            return (m_val != rhs.m_val);
        return (m_valid || rhs.m_valid);
    }

private:
    bool        m_valid;
    T           m_val;
};

template<typename T>
std::ostream &
operator<<(std::ostream & os,const FgOpt<T> & v)
{
    if (v.valid())
        os << v.val();
    else
        os << "Invalid";
    return os;
}

template<typename T>
struct FgValid
{
    T       m_val;      // = numeric_limits<T>::max() if not valid

    FgValid()
    : m_val(std::numeric_limits<T>::max())
    {}

    explicit
    FgValid(const T & v)
    : m_val(v)
    {}

    FgValid &
    operator=(const T & v)
    {m_val = v; return *this; }

    bool
    valid() const
    {return (m_val != std::numeric_limits<T>::max()); }

    void
    invalidate()
    {m_val = std::numeric_limits<T>::max(); }

    // Implicit conversion caused inexplicable errors with gcc and explicit conversion
    // is required in many cases anyway:
    T
    val() const
    {FGASSERT(valid()); return m_val; }

    // The constPtr() and ptr() functions below couldn't be overloads of operator&() since
    // this doesn't play nice with standard library containers:
    const T *
    constPtr() const
    {FGASSERT(valid()); return &m_val; }

    // You're on your own if you use this one, NO CHECKING, since it may be used to set the val,
    // so do a manual check using valid() if you need one along with non-const pointer access:
    T *
    ptr()
    {return &m_val; }

    FG_SERIALIZE1(m_val)
};

template<typename T>
std::ostream &
operator<<(std::ostream & os,const FgValid<T> & v)
{
    if (v.valid())
        return (os << v.val());
    else
        return (os << "<invalid>");
}

#endif
