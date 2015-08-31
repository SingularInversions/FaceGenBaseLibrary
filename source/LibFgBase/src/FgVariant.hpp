//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Nov 16, 2005
//
// Runtime polymorphic encapsulation of any copy-constructible type for C++.
//
//      USE:
//
// See Libfgen/tests/FgVariant.cpp
//
// Uses the copy-on-copy idiom for heap object ownership.
// Types must have a copy constructor.
// In retrospect, could have used boost::any I suppose.

#ifndef FGVARIANT_HPP
#define FGVARIANT_HPP

#include "FgStdLibs.hpp"

#include "FgTypes.hpp"
#include "FgException.hpp"
#include "FgOut.hpp"
#include "FgStdPair.hpp"
#include "FgSharedPtr.hpp"

// Default print statement to avoid forcing all variant types to define one. Use macro 
// below to link fgPrint to each class which supports operator<<(ostream &,const T &):
template<class T>
std::ostream &
fgPrint(std::ostream & os,const T &)
{return os << " fgPrint() not defined."; }

#define FG_VARIANT_PRINT(T)                     \
    inline                                      \
    std::ostream &                              \
    fgPrint(std::ostream & os,const T & v)      \
    {return os << v; }

class FgVariant
{
public:
    FgVariant()
    {}

    template<class T>
    explicit
    FgVariant(const T & val)
    : m_poly(new Poly<T>(val))
    {}

    // Copy constructor is a deep copy:
    FgVariant(const FgVariant &var)
    {
        if (var.m_poly)
            m_poly = var.m_poly->clone();
    };

    template<class T>
    void
    operator=(const T & val)
    {
        m_poly.reset(new Poly<T>(val));
    }

    template<class T>
    void
    set(const T & val)
    {
        T &     tv = getRef<T>();
        tv = val;
    }

    // Assigning from an FgVariant copies the value within:
    FgVariant &
    operator=(const FgVariant & var);

    template<class T>
    bool
    isType() const
    {
        Poly<T> * ptr = dynamic_cast<Poly<T>*>(m_poly.get());
        return (ptr != NULL);
    }

    // Use for explicit value access, for instance when usage context type is ambiguous:
    template<class T>
    const T &
    getCRef() const
    {
        if (!m_poly)
            fgThrow("Variant NULL dereferenced as",typeid(T).name());
        Poly<T> * ptr = dynamic_cast<Poly<T>*>(m_poly.get());
        if (!ptr)
            fgThrow("Variant incompatible type dereference from/to",
                    m_poly->typeName() + " / " + typeid(T).name());
        return ptr->getCRef();
    }

    // Use for explicit value modification:
    template<class T>
    T &
    getRef()
    {return const_cast<T&>(getCRef<T>()); }

    // A [Const]ValueProxy class is used as an opaque type to allow automatic conversions.
    // Direct conversion was not possible as GCC could not support the const version,
    // and VS2010 std::vector has a bug that performs a char& conversion on elements.
    // This class should not be used by clients except through the valueRef functions.
    struct ValueProxy
    {
        explicit
        ValueProxy(FgVariant*v):
            m_variant(v)
        {}

        template<typename T>
        operator T & ()
        { 
            // T can be const here, so remove the const from the
            // type otherwise multiple registrations can occur.
            typedef typename boost::remove_const<T>::type type;
            return m_variant->getRef<type>(); 
        }

    private:
        FgVariant *m_variant;
    };

    struct ConstValueProxy
    {
        explicit
        ConstValueProxy(const FgVariant *v):
            m_variant(v)
        {}

        template<typename T>
        operator T () const
        {                                                       
            typedef typename boost::remove_const<T>::type type;
            return m_variant->getCRef<type>(); 
        }

    private:
        const FgVariant *m_variant;
    };

    ConstValueProxy
    valueRef() const 
    { return ConstValueProxy(this); }

    ValueProxy
    valueRef()
    { return ValueProxy(this); }
    
    std::ostream &
    print(std::ostream & ss) const;

    std::string
    typeName() const
    {return m_poly->typeName(); }

private:
    class PolyBase
    {
    public:
        virtual ~PolyBase() {};

            // Returns a pointer to a *new* copy of the object
        virtual 
        FgSharedPtr<PolyBase>          
        clone() const = 0;

        virtual std::string 
        typeName() const = 0;

        virtual std::ostream&
        print(std::ostream& s) const = 0;
    };

    // The data container makes use of the default copy constructor and (virtual) destructor.
    template<class T>
    class Poly : public PolyBase
    {
    public:
        Poly() {};
        explicit Poly(const T& val) : m_data(val) {};

        virtual 
        FgSharedPtr<PolyBase>
        clone() const 
        {
            return FgSharedPtr<PolyBase>(new Poly(m_data));
        }

        virtual 
        std::string 
        typeName() const 
        {
            return typeid(T).name(); 
        }

        virtual 
        std::ostream&   
        print(std::ostream & os) const
        {return fgPrint(os,m_data); }

        const T &
        getCRef() const
        {return m_data; };

    private:
        // This is a temporary(?) workaround for gcc 4.0 which is
        // used with OSX: Somewhere, a combination of temporaries,
        // returning by value and overloaded operators cause a
        // const type to be put here which should generally never
        // be the case.
        typename boost::remove_const<T>::type    m_data;
    };

    FgSharedPtr<PolyBase> m_poly;
};

inline std::ostream &
operator<<(std::ostream & ss,const FgVariant & vv)
{return vv.print(ss); }

FG_VARIANT_PRINT(float)
FG_VARIANT_PRINT(int)
FG_VARIANT_PRINT(double)

#endif
