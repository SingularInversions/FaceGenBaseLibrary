//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
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

#include "FgSerial.hpp"

namespace Fg {

struct      AnyPolyBase
{
    std::type_info const *  typeInfoPtr;    // get type info without dynamic cast

    AnyPolyBase() : typeInfoPtr{nullptr} {}
    AnyPolyBase(std::type_info const * t) : typeInfoPtr{t} {}
    virtual ~AnyPolyBase() {};

    String              typeName() const
    {
        if (typeInfoPtr == nullptr)
            return "nullptr";
        else
            return typeInfoPtr->name();
    }
};

template<class T>
struct      AnyPoly : AnyPolyBase
{
    T                   object;

    explicit AnyPoly(T const & val) : AnyPolyBase{&typeid(T)}, object{val} {}
    explicit AnyPoly(T && val) :  AnyPolyBase{&typeid(T)}, object(std::forward<T>(val)) {}
};

struct      AnyWeak
{
    std::weak_ptr<AnyPolyBase>    objPtr;

    AnyWeak() {}      // Need this for vector storage
    explicit AnyWeak(const std::shared_ptr<AnyPolyBase> & v) : objPtr(v) {}

    Sptr<AnyPolyBase>   asShared() const
    {
        if (objPtr.expired())
            fgThrow("AnyWeak expired dereference");
        return objPtr.lock();
    }
};

typedef Svec<AnyWeak>      AnyWeaks;

class       Any
{
    Sptr<AnyPolyBase>       objPtr;

public:
    Any() {}

    template<class T>
    explicit Any(T const & val) : objPtr(std::make_shared<AnyPoly<T> >(val)) {}

    Any(Any const &) = default;     // must be declared to avoid being overriden by above

    explicit                operator bool() const {return bool(objPtr); }

    template<class T> void  operator=(T const & val) {objPtr = std::make_shared<AnyPoly<T> >(val); }
    void                    operator=(Any const & var) {objPtr = var.objPtr; }

    template<class T>
    bool                    is() const
    {
        if (!objPtr)
            fgThrow("Any.is() null dereference",typeid(T).name());
        return (*(objPtr->typeInfoPtr) == typeid(T));
    }

    template<class T>
    T const &               as() const
    {
        if (!objPtr)
            fgThrow("Any.as() null dereference",typeid(T).name());
        AnyPoly<T> const *      ptr = dynamic_cast<AnyPoly<T>*>(objPtr.get());
        if (ptr == nullptr)
            fgThrow("Any.as() incompatible type dereference",objPtr->typeName()+"->"+typeid(T).name());
        return ptr->object;
    }

    // Follows the idiom of returning a null pointer if the type differs:
    template<class T>
    T const *               asp() const
    {
        if (!objPtr)
            return nullptr;
        const AnyPoly<T> *      ptr = dynamic_cast<AnyPoly<T>*>(objPtr.get());
        if (ptr == nullptr)
            return nullptr;
        return &ptr->object;
    }

    // Use at your own risk as this violates copy semantics:
    template<class T>
    T &                     ref()
    {
        if (!objPtr)
            fgThrow("Any.ref() null dereference",typeid(T).name());
        AnyPoly<T> *            ptr = dynamic_cast<AnyPoly<T>*>(objPtr.get());
        if (ptr == nullptr)
            fgThrow("Any.ref() incompatible type dereference",objPtr->typeName()+"->"+typeid(T).name());
        return ptr->object;
    }

    String                  typeName() const {return objPtr->typeName(); }

    AnyWeak                 weak() {return AnyWeak(objPtr); }
    bool                    valid() const {return bool(objPtr); }
    void                    reset() {objPtr.reset(); }
    template<class T> void  reset(T const & val) {objPtr.reset(new AnyPoly<T>(val)); }
    template<class T> void  reset(T && val) {objPtr.reset(new AnyPoly<T>(std::forward<T>(val))); }
};

typedef Svec<Any>  Anys;

}

#endif
