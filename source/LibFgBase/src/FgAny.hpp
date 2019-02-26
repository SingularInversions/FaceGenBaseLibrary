//
// Copyright (c) 2018 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     June 19, 2018
//
// Like std::any but copy semantics achieved with const pointer to const rather than copy-on-copy,
// so it can be used by recursive data structures, eg. AST or DAG.
//
// Also includes handy 'is' and 'as' members. If-then dispatch is preferred due to its great
// simplicity in both understanding the code and avoiding all the boilerplate required to
// combine multiple dispatch and/or inheritance dispatch. In the unlikely case where performance
// is an issue, the boilerplate can be added later to those functions.
//
// A weak pointer version is also provided in case two-way links are needed.

#ifndef FGANY_HPP
#define FGANY_HPP

#include "FgStdLibs.hpp"
#include "FgException.hpp"
#include "FgString.hpp"

struct FgAnyPolyBase
{
    virtual ~FgAnyPolyBase() {};

    virtual
    std::string 
    typeName() const = 0;
};

template<class T>
struct FgAnyPoly : public FgAnyPolyBase
{
    T           object;

    // virtual destructor created automatically
    explicit
    FgAnyPoly(const T & val) : object(val) {}

    virtual
    std::string 
    typeName() const 
    {return typeid(T).name(); }
};

class FgAnyWeak
{
    std::weak_ptr<FgAnyPolyBase>    objPtr;

    std::shared_ptr<FgAnyPolyBase>
    asShared() const
    {
        if (objPtr.expired())
            fgThrow("FgAnyWeak expired dereference");
        return objPtr.lock();
    }

public:
    FgAnyWeak() {}      // Need this for vector storage

    explicit FgAnyWeak(const std::shared_ptr<FgAnyPolyBase> & v) : objPtr(v) {}

    std::string
    typeName() const
    {return asShared()->typeName(); }

    template<class T>
    bool
    is() const
    {
        const FgAnyPoly<T> * ptr = dynamic_cast<FgAnyPoly<T>*>(asShared().get());
        return (ptr != nullptr);
    }

    template<class T>
    const T &
    as() const
    {
        const FgAnyPoly<T> * ptr = dynamic_cast<FgAnyPoly<T>*>(asShared().get());
        if (ptr == nullptr)
            fgThrow("FgAnyWeak.as incompatible type dereference",asShared()->typeName()+"->"+typeid(T).name());
        return ptr->object;
    }

    // Use at your own risk as this violates copy semantics:
    template<class T>
    T &
    ref()
    {
        FgAnyPoly<T> *      ptr = dynamic_cast<FgAnyPoly<T>*>(asShared().get());
        if (ptr == nullptr)
            fgThrow("FgAnyWeak.ref incompatible type dereference",asShared()->typeName()+"->"+typeid(T).name());
        return ptr->object;
    }
};

typedef std::vector<FgAnyWeak>      FgAnyWeaks;

class FgAny
{
    std::shared_ptr<FgAnyPolyBase>   objPtr;

public:
    FgAny() {}

    // Implicit conversion is very useful here, for example 'FgAny x = 5;' or argument passing:
    template<class T>
    FgAny(const T & val) : objPtr(std::make_shared<FgAnyPoly<T> >(val)) {}

    FgAny(const FgAny & rhs) : objPtr(rhs.objPtr) {}

    explicit operator bool() const
    {return bool(objPtr); }

    std::string
    typeName() const
    {
        if (!objPtr)
            fgThrow("FgAny.typeName null dereference");
        return objPtr->typeName();
    }

    template<class T>
    void
    operator=(const T & val)
    {objPtr = std::make_shared<FgAnyPoly<T> >(val); }

    void
    operator=(const FgAny & var)
    {objPtr = var.objPtr; }

    template<class T>
    bool
    is() const
    {
        if (!objPtr)
            fgThrow("FgAny.is null dereference",typeid(T).name());
        const FgAnyPoly<T> * ptr = dynamic_cast<FgAnyPoly<T>*>(objPtr.get());
        return (ptr != nullptr);
    }

    template<class T>
    const T &
    as() const
    {
        if (!objPtr)
            fgThrow("FgAny.as null dereference",typeid(T).name());
        const FgAnyPoly<T> * ptr = dynamic_cast<FgAnyPoly<T>*>(objPtr.get());
        if (ptr == nullptr)
            fgThrow("FgAny.as incompatible type dereference",objPtr->typeName()+"->"+typeid(T).name());
        return ptr->object;
    }

    // Follows the idiom of returning a null pointer if the type differs:
    template<class T>
    const T *
    asp() const
    {
        if (!objPtr)
            return nullptr;
        const FgAnyPoly<T> * ptr = dynamic_cast<FgAnyPoly<T>*>(objPtr.get());
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
            fgThrow("FgAny.ref null dereference",typeid(T).name());
        FgAnyPoly<T> *      ptr = dynamic_cast<FgAnyPoly<T>*>(objPtr.get());
        if (ptr == nullptr)
            fgThrow("FgAny.ref incompatible type dereference",objPtr->typeName()+"->"+typeid(T).name());
        return ptr->object;
    }

    FgAnyWeak
    weak()
    {return FgAnyWeak(objPtr); }
};

typedef std::vector<FgAny>  FgAnys;

#endif
