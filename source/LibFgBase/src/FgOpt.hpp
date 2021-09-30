//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Optional-value type extender.
//
// * std::optional not available until C++17.
// * Simpler than boost::optional - carries around a default constructed value when not valid.
// * To avoid additional memory use for numerical types, use 'Valid' but note that
//   numeric_limits<T>::max() is the special 'invalid' value.
//  * Otherwise, use 'Opt' which adds a bool to keep track.
//

#ifndef FGOPT_HPP
#define FGOPT_HPP

#include "FgStdLibs.hpp"
#include "FgSerialize.hpp"
#include "FgDiagnostics.hpp"

namespace Fg {

template<typename T>
class   Opt
{
    bool        m_valid;
    T           m_val;

public:
    Opt() : m_valid(false) {}
    Opt(T const & v) : m_valid(true), m_val(v) {}

    Opt &           operator=(T const & v) {m_val=v; m_valid=true; return *this; }
    bool            valid() const {return m_valid; }
    void            invalidate() {m_valid = false; }
    T &             ref() {FGASSERT(m_valid); return m_val; }
    T const &       val() const {FGASSERT(m_valid); return m_val; }
    template<typename U>
    Opt<U>
    cast()
    {
        if (m_valid)
            return Opt<U>(U(m_val));
        return Opt<U>();
    }
    bool
    operator==(const Opt<T> & rhs) const
    {
        if (m_valid && rhs.m_valid)
            return (m_val == rhs.m_val);
        return (!m_valid && !rhs.m_valid);
    }
    bool
    operator!=(const Opt<T> & rhs) const
    {
        if (m_valid && rhs.m_valid)
            return (m_val != rhs.m_val);
        return (m_valid || rhs.m_valid);
    }
};

template<typename T>
std::ostream &
operator<<(std::ostream & os,const Opt<T> & v)
{
    if (v.valid())
        os << v.val();
    else
        os << "Invalid";
    return os;
}

template<typename T>
struct Valid
{
    T       m_val;      // = numeric_limits<T>::max() if not valid

    Valid() : m_val(std::numeric_limits<T>::max()) {}
    explicit Valid(T const & v) : m_val(v) {}

    Valid &         operator=(T const & v) {m_val = v; return *this; }
    bool            valid() const {return (m_val != std::numeric_limits<T>::max()); }
    void            invalidate() {m_val = std::numeric_limits<T>::max(); }
    // Implicit conversion caused inexplicable errors with gcc and explicit conversion
    // is required in many cases anyway:
    T               val() const {FGASSERT(valid()); return m_val; }
    // The constPtr() and ptr() functions below couldn't be overloads of operator&() since
    // this doesn't play nice with standard library containers:
    T const *       constPtr() const {FGASSERT(valid()); return &m_val; }
    // You're on your own if you use this one, NO CHECKING, since it may be used to set the val,
    // so do a manual check using valid() if you need one along with non-const pointer access:
    T *             ptr() {return &m_val; }

    FG_SERIALIZE1(m_val)
};

template<typename T>
std::ostream &
operator<<(std::ostream & os,const Valid<T> & v)
{
    if (v.valid())
        return (os << v.val());
    else
        return (os << "<invalid>");
}

}

#endif
