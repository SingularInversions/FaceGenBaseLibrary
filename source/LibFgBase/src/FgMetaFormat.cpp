//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

//

#include "stdafx.h"
#include "FgMetaFormat.hpp"
#include "FgDiagnostics.hpp"
#include "FgTestUtils.hpp"
#include "FgCommand.hpp"

using namespace std;

namespace Fg {

struct AA
{
    int a;
    AA():a(0){}

    template<typename Archive>
    void serialize(Archive & ar, unsigned int)
    { ar & BOOST_SERIALIZATION_NVP(a); }
};

template<typename IArchive, typename OArchive>
void
fgTestArchive()
{
    AA a;
    FGASSERT(a.a == 0);
    ++a.a;
    std::ostringstream os;
    {
        OArchive oa(os);
        oa << BOOST_SERIALIZATION_NVP(a);
    }

    std::istringstream is(os.str());
    {
        IArchive ia(is);
        ia >> BOOST_SERIALIZATION_NVP(a);
    }
    FGASSERT(a.a == 1);
}

void
fgBoostSerializationTest(const CLArgs &)
{
    using namespace boost::archive;

    fgTestArchive<binary_iarchive,binary_oarchive>();
    fgTestArchive<xml_iarchive,xml_oarchive>();
    fgTestArchive<text_iarchive,text_oarchive>();
}

void
fgMetaFormatTest(const CLArgs &)
{
    FgTestDir   td("MetaFormat");
    int     a = 42;
    fgSaveBin("test.fgbin",a);
    int     b;
    fgLoadBin("test.fgbin",b);
    FGASSERT(a == b);
}

using namespace boost::archive;

struct A
{
    size_t  m0;
    FG_SERIALIZE1(m0);
};
template<class ArIn,class ArOut>
struct B
{
    size_t  m0;
    size_t  m1;
    FG_SER_MIGRATE1(ArIn,m0);
    FG_SER_MIGRATE2(ArOut,m0,m1);
};
struct C
{
    size_t  m0;
    size_t  m1;
    FG_SERIALIZE2(m0,m1);
};

void
fgSerializeTest(const CLArgs &)
{
    FgTestDir   td("Serialize");
    size_t                                  sz = 5;
    vector<A>                               va(sz);
    vector<B<text_iarchive,text_oarchive> > vb;
    vector<C>                               vc;
    for (size_t ii=0; ii<sz; ++ii)
        va[ii].m0 = ii*ii;
    fgSaveText("t0",va);
    fgLoadText("t0",vb);
    fgSaveText("t1",vb);
    fgLoadText("t1",vc);
    for (size_t ii=0; ii<sz; ++ii)
        FGASSERT(va[ii].m0 == vc[ii].m0);
}

}

// */
