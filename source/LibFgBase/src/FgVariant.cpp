//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Nov 21, 2005
//

#include "stdafx.h"

#include "FgVariant.hpp"
#include "FgDiagnostics.hpp"
#include "FgMatrix.hpp"
#include "FgTestUtils.hpp"
#include "FgMain.hpp"

using namespace std;

void
fgVariantTest(const FgArgs &)
{
    void testVariantProxy();
    testVariantProxy();

    void testVariantB();
    testVariantB();
}

void 
testVariantB()
{
    FgVariant  v1 = FgVariant(3.14159),
                v2 = FgVariant(string("hello")),
                v3,v4,v5;

    v3 = 7;
    v4 = v3;
    v3 = v2;
    v2 = v1;

        // This bit replicates a bug found that only cropped up when an FgVariant that was
        // created by copy constructor from a NULL one went out of scope. The problem was that
        // the pointer member is NOT automatically set to NULL.
    vector<FgVariant>  vtmp;
    vtmp.push_back(FgVariant());

    double      &dd = v2.getRef<double>();
    string      &ss = v3.getRef<string>();
    int         &ii = v4.getRef<int>();

    FGASSERT(dd == 3.14159);
    FGASSERT(ss == string("hello"));
    FGASSERT(ii == 7);

        // Test 'operator const T&() const' and 'operator T&()':
    {
        FgVariant variant(FgVect2D(1234,5678));
        const FgVect2D & r1 = variant.getCRef<FgVect2D>();
        FgVect2D & r2 = variant.valueRef();
        r2[0] = 4321;
        FGASSERT(r1[0] == r2[0]);
    }
        // Test that exceptions are thrown:
    {
        FgVariant v0(5.0);
        bool fg_test_check_threw = false;
        try {
            v0.getCRef<int>();
        }
        catch(...) {
            fg_test_check_threw = true;
        }
        FGASSERT1(fg_test_check_threw,"An exception was expected but none was thrown");
    }
}

struct  VSI
{
    int     a;
};

ostream & operator<<(ostream & s,const VSI & v) {return s << v.a; }

class CountCopies
{
public:
    CountCopies(){}
    CountCopies(CountCopies const &)
    {
        ++copies;
    }
    static int copies;
};

int 
CountCopies::copies = 0;

void
testVariantProxy()
{
    FgVariant v(double(25.0));
	double d = v.valueRef();
	FGASSERT(d == 25.0);

	FgVariant const cv(double(35.0));
	double e = cv.valueRef();
	FGASSERT(e == 35.0);

    const double & f = cv.valueRef();
    FGASSERT(f == 35.0);

    double g = cv.valueRef();
    FGASSERT(g == 35.0);

    CountCopies c;
    FgVariant copies(c);
    FGASSERT(CountCopies::copies == 1);
    
    CountCopies c1(copies.valueRef());
    FGASSERT(c1.copies == 2);
}

// */
