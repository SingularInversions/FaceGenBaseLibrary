//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Like std::any but with reference semantics, and doesn't require T to have copy constructor.
//
// * Includes 'is' and 'as' members for handy if-then dispatch.
// * In case where performance is an issue, the boilerplate can be added later to those functions.
// * A weak pointer version is also provided in case two-way links are needed.

#ifndef FGANY_HPP
#define FGANY_HPP

#include "FgStdLibs.hpp"
#include "FgException.hpp"
#include "FgString.hpp"

namespace Fg {

struct AnyPolyBase
{
    virtual ~AnyPolyBase() {};

    virtual
    std::string 
    typeName() const = 0;
};

template<class T>
struct AnyPoly : AnyPolyBase
{
    T           object;

    explicit AnyPoly(T const & val) : object(val) {}
    explicit AnyPoly(T && val) : object(std::forward<T>(val)) {}

    virtual
    std::string 
    typeName() const 
    {return typeid(T).name(); }
};

class AnyWeak
{
    std::weak_ptr<AnyPolyBase>    objPtr;

    std::shared_ptr<AnyPolyBase>
    asShared() const
    {
        if (objPtr.expired())
            fgThrow("AnyWeak expired dereference");
        return objPtr.lock();
    }

public:
    AnyWeak() {}      // Need this for vector storage

    explicit AnyWeak(const std::shared_ptr<AnyPolyBase> & v) : objPtr(v) {}

    std::string
    typeName() const
    {return asShared()->typeName(); }

    template<class T>
    bool
    is() const
    {
        const AnyPoly<T> * ptr = dynamic_cast<AnyPoly<T>*>(asShared().get());
        return (ptr != nullptr);
    }

    template<class T>
    const T &
    as() const
    {
        const AnyPoly<T> * ptr = dynamic_cast<AnyPoly<T>*>(asShared().get());
        if (ptr == nullptr)
            fgThrow("AnyWeak.as incompatible type dereference",asShared()->typeName()+"->"+typeid(T).name());
        return ptr->object;
    }

    // Use at your own risk as this violates copy semantics:
    template<class T>
    T &
    ref()
    {
        AnyPoly<T> *      ptr = dynamic_cast<AnyPoly<T>*>(asShared().get());
        if (ptr == nullptr)
            fgThrow("AnyWeak.ref incompatible type dereference",asShared()->typeName()+"->"+typeid(T).name());
        return ptr->object;
    }
};

typedef Svec<AnyWeak>      AnyWeaks;

class Any
{
    std::shared_ptr<AnyPolyBase>   objPtr;

public:
    Any() {}

    // Implicit conversion is very useful here, for example 'Any x = 5;' or argument passing:
    template<class T>
    Any(const T & val) : objPtr(std::make_shared<AnyPoly<T> >(val)) {}

    Any(const Any & rhs) : objPtr(rhs.objPtr) {}

    explicit operator bool() const
    {return bool(objPtr); }

    std::string
    typeName() const
    {
        if (!objPtr)
            fgThrow("Any.typeName null dereference");
        return objPtr->typeName();
    }

    template<class T>
    void
    operator=(const T & val)
    {objPtr = std::make_shared<AnyPoly<T> >(val); }

    void
    operator=(const Any & var)
    {objPtr = var.objPtr; }

    template<class T>
    bool
    is() const
    {
        if (!objPtr)
            fgThrow("Any.is null dereference",typeid(T).name());
        const AnyPoly<T> * ptr = dynamic_cast<AnyPoly<T>*>(objPtr.get());
        return (ptr != nullptr);
    }

    template<class T>
    const T &
    as() const
    {
        if (!objPtr)
            fgThrow("Any.as null dereference",typeid(T).name());
        const AnyPoly<T> * ptr = dynamic_cast<AnyPoly<T>*>(objPtr.get());
        if (ptr == nullptr)
            fgThrow("Any.as incompatible type dereference",objPtr->typeName()+"->"+typeid(T).name());
        return ptr->object;
    }

    // Follows the idiom of returning a null pointer if the type differs:
    template<class T>
    const T *
    asp() const
    {
        if (!objPtr)
            return nullptr;
        const AnyPoly<T> * ptr = dynamic_cast<AnyPoly<T>*>(objPtr.get());
        if (ptr == nullptr)
            return nullptr;
        return &ptr->object;
    }

    // Use at your own risk as this violates copy semantics:
    template<class T>
    T &
    ref()
    {
        if (!objPtr)
            fgThrow("Any.ref null dereference",typeid(T).name());
        AnyPoly<T> *      ptr = dynamic_cast<AnyPoly<T>*>(objPtr.get());
        if (ptr == nullptr)
            fgThrow("Any.ref incompatible type dereference",objPtr->typeName()+"->"+typeid(T).name());
        return ptr->object;
    }

    AnyWeak
    weak()
    {return AnyWeak(objPtr); }

    bool
    empty() const
    {return !bool(objPtr); }

    template<class T>
    void
    reset(T const & val)
    {objPtr.reset(new AnyPoly<T>(val)); }

    template<class T>
    void
    reset(T && val)
    {objPtr.reset(new AnyPoly<T>(std::forward<T>(val))); }
};

typedef Svec<Any>  Anys;

}

#endif
