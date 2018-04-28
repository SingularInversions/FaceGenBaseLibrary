//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Sohail Somani
//

#include "stdafx.h"
#include "FgSharedPtr.hpp"
#include "FgTestUtils.hpp"
#include "FgMain.hpp"

// Note that the types are now complete!
struct incomplete_type
{
    incomplete_type(){++counter;}
    virtual ~incomplete_type(){--counter;}

    static std::size_t counter;
};
std::size_t incomplete_type::counter = 0;

struct incomplete_child :
    public incomplete_type
{
    incomplete_child(){++counter;}
    ~incomplete_child(){--counter;}

    static std::size_t counter;
};
std::size_t incomplete_child::counter = 0;

FgPtr<incomplete_type>
make_it()
{
    return fgnew<incomplete_child>();
}

std::size_t
get_incomplete_type_instances()
{
    return incomplete_type::counter;
}

std::size_t
get_incomplete_child_instances()
{
    return incomplete_child::counter;
}

struct a_struct
{
    static int counter;
    a_struct()
    {
        ++counter;
    }
    ~a_struct()
    {
        --counter;
    }
};
int a_struct::counter = 0;

typedef FgPtr<a_struct> ptr_t;

static void test_deref()
{
    {
        ptr_t p1(fgnew<a_struct>());
        *p1 = a_struct();
    }
    FGASSERT(a_struct::counter==0);
}

static void test_destructor()
{
    FGASSERT(a_struct::counter==0);
    {
        ptr_t p1(fgnew<a_struct>());
        FGASSERT(p1.get());
        {
            FGASSERT(a_struct::counter==1);
            {
                ptr_t p2(fgnew<a_struct>());
                FGASSERT(p2.get());
                FGASSERT(a_struct::counter==2);
            }
            FGASSERT(a_struct::counter==1);
        }
    }
    FGASSERT(a_struct::counter==0);
}

static void test_copy()
{
    FGASSERT(a_struct::counter==0);
    {
        ptr_t p1(fgnew<a_struct>());
        FGASSERT(p1.get());
        FGASSERT(a_struct::counter==1);
        {
            ptr_t p2(p1); // copy constructor
            FGASSERT(p2.get());
            FGASSERT(p2.use_count() == 2);
            FGASSERT(p1.use_count() == 2);
            FGASSERT(a_struct::counter==1);
        }
        FGASSERT(a_struct::counter==1);
    }
    FGASSERT(a_struct::counter==0);
}

static void test_assign()
{
    FGASSERT(a_struct::counter==0);
    {
        ptr_t p1(fgnew<a_struct>());
        FGASSERT(p1.get());
        FGASSERT(a_struct::counter==1);
        {
            ptr_t p2(fgnew<a_struct>());
            FGASSERT(p2.get());
            FGASSERT(a_struct::counter==2);
            p2 = p1;
                // The use count is 2
            FGASSERT(p2.use_count() == 2);
            FGASSERT(p1.use_count() == 2);
                // But p2 should have been destructed
            FGASSERT(a_struct::counter==1);
        }
        FGASSERT(a_struct::counter==1);
    }
    FGASSERT(a_struct::counter==0);
}

struct base
{
    static int counter;
    base()
    {
        ++counter;
    }
    virtual ~base()
    {
        --counter;
    }
    virtual void make_abc()=0;
};
int base::counter = 0;

struct child : public base
{
    static int counter;
    child()
    {
        ++counter;
    }
    ~child()
    {
        --counter;
    }
    void make_abc(){}
};
int child::counter = 0;

typedef FgPtr<base> base_ptr_t;
typedef FgPtr<child> child_ptr_t;

FgPtr<base>
foo()
{
    return fgnew<child>();
}

static void test_poly()
{
    {
        child_ptr_t c1(fgnew<child>());
        {
            FGASSERT(c1.get());
            FGASSERT(child::counter == 1);
            FGASSERT(base::counter == 1);
            FGASSERT(c1.use_count() == 1);
            {
                base_ptr_t b1(c1);
                FGASSERT(b1.get());
                FGASSERT(child::counter == 1);
                FGASSERT(base::counter == 1);
                FGASSERT(b1.use_count() == 2);
                FGASSERT(c1.use_count() == 2);
                FGASSERT(dynamic_cast<child*>(b1.get()));
            }
            FGASSERT(child::counter == 1);
            FGASSERT(base::counter == 1);
        }
    }
    FGASSERT(child::counter == 0);
    FGASSERT(base::counter == 0);
}

static void test_vector()
{
    base_ptr_t p1(fgnew<child>());
    {
        std::vector<base_ptr_t> v(9,p1);
        v.resize(10,p1);
        std::vector<base_ptr_t> v2(v);
        FGASSERT(p1.use_count() == (2*v.size())+1);
        FGASSERT(base::counter == 1);
        FGASSERT(child::counter == 1);
    }
    FGASSERT(p1.use_count()==1);
}

static void test_null()
{
    {
        FGASSERT(child::counter == 0);
        FGASSERT(base::counter == 0);
        {
            base_ptr_t b1;
            {
                FGASSERT(b1.use_count() == 0);
                child_ptr_t c1(fgnew<child>());
                b1 = c1;
                FGASSERT(c1.use_count() == 2);
            }
            FGASSERT(b1.use_count() == 1);
        }
        FGASSERT(child::counter == 0);
        FGASSERT(base::counter == 0);
    }
    {
        base_ptr_t b1;
        FGASSERT( b1 ? false : true );
        FGASSERT( !b1 );

        child_ptr_t c1(fgnew<child>());
        b1 = c1;
        FGASSERT( c1 );
        FGASSERT( b1 );
    }
}

static void test_reset()
{
    {
        base_ptr_t b1(fgnew<child>());
        {
            FGASSERT(b1.use_count() == 1);
            FGASSERT(child::counter == 1);
            b1.reset();
            FGASSERT(b1.use_count() == 0);
            FGASSERT(child::counter == 0);
        }
    }
    {
        base_ptr_t b;
        {
            FGASSERT(b.use_count() == 0);
            FGASSERT(!b);
            b.reset(new child);
            FGASSERT(b.use_count() == 1);
            FGASSERT(b);
            FGASSERT(child::counter == 1);
        }
        b.reset();
        FGASSERT(!b && child::counter==0);
    }
}

struct f
{
    f(){}
    f(int){}
    f(int,int){}
    f(int,int,int){}
    f(int,int,int,int){}
    f(int,int,int,int,int){}
    f(int,int,int,int,int,int){}
    f(int,int,int,int,int,int,int){}
};

static void test_fgnew()
{
    int a=5;
    FGASSERT(fgnew<f>());
    FGASSERT(fgnew<f>(a));
    FGASSERT(fgnew<f>(a,a));
    FGASSERT(fgnew<f>(a,a,a));
    FGASSERT(fgnew<f>(a,a,a,a));
    FGASSERT(fgnew<f>(a,a,a,a,a));
    FGASSERT(fgnew<f>(a,a,a,a,a,a));

    const int b=5;
    FGASSERT(fgnew<f>(b));
}

struct child2 : public base
{
    void make_abc(){}
};
typedef FgPtr<child2> child2_ptr_t;

// Defined in incomplete_type.cpp
struct incomplete_type;
typedef FgPtr<incomplete_type> incomplete_type_ptr_t;

incomplete_type_ptr_t
make_it();

size_t
get_incomplete_type_instances();

size_t
get_incomplete_child_instances();

// This test tests that incomplete types can be deleted
// safely so long as the delete is instantiated when we
// have a complete object
static void test_incomplete_type()
{
    FGASSERT(get_incomplete_type_instances() == 0);
    FGASSERT(get_incomplete_child_instances() == 0);
    {
        incomplete_type_ptr_t p(make_it());
        FGASSERT(get_incomplete_child_instances() == 1);
        FGASSERT(get_incomplete_type_instances() == 1);
    }
    FGASSERT(get_incomplete_type_instances() == 0);
    FGASSERT(get_incomplete_child_instances() == 0);
}

static void test_copy_default_constructed()
{
    FgPtr<base> p;
    FgPtr<base> p2(p);
}

void
fgSharedPtrTest(const FgArgs &)
{
    FgMemoryLeakDetector m;
    {
        test_deref();
        test_destructor();
        test_copy();
        test_assign();
        test_poly();
        test_vector();
        test_null();
        test_reset();
        test_fgnew();
        test_incomplete_type();
        test_copy_default_constructed();
    }
    return m.throw_if_leaked("SharedPtr");
}
