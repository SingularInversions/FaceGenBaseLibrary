//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     April 4, 2008
//

#include "stdafx.h"

#include "FgException.hpp"
#include "FgDiagnostics.hpp"
#include "FgMain.hpp"

using namespace std;

const string   state1 = "State1";
const FgString state2("State2");
// TODO: Check for a specific language translation here once implemented:
const string   tr_correct = 
    "While executing func1 with state : State1\n"
    "Problem in func2 with state : State2";
const string   no_tr_correct = 
    "While executing func1 with state : State1\n"
    "Problem in func2 with state : State2";

void    func2()
{
    fgThrow("Problem in func2 with state",state2);
}

void    func1()
{
    try
    {
        func2();
    }
    catch (FgException& e)
    {
    	e.pushMsg("While executing func1 with state",state1);
        throw;
    }
}

void
fgExceptionTest(const FgArgs &)
{
    try
    {
        func1();
    }
    catch (FgException& e)
    {
        FGASSERT(e.tr_message() == tr_correct);
        FGASSERT(e.no_tr_message() == no_tr_correct);
    }
    
    void testException2();
    testException2();
}

struct FgMyException : public FgException
{
    FgMyException(std::string const & s):FgException(s){}
    FgMyException(std::string const & s, std::string const & m):FgException(s,m){}
    FgMyException(std::string const & s, FgString const & m):FgException(s,m){}
};

void
testException2()
{
    try
    {
        fgThrow<FgMyException>("abcd");
    }
    catch(FgMyException& e)
    {
        FGASSERT(e.tr_message() == "abcd");
        FGASSERT(e.no_tr_message() == "abcd");
    }
    catch(...)
    {
        FGASSERT_FALSE;
    }

    try
    {
        fgThrow<FgMyException>("abcd","efg");
    }
    catch(FgMyException& e)
    {
        FGASSERT(e.tr_message() == "abcd : efg");
        FGASSERT(e.no_tr_message() == "abcd : efg");
    }
    catch(...)
    {
        FGASSERT_FALSE;
    }

    try
    {
        fgThrow<FgMyException>("abcd",FgString("efg"));
    }
    catch(FgMyException& e)
    {
        FGASSERT(e.tr_message() == "abcd : efg");
        FGASSERT(e.no_tr_message() == "abcd : efg");
    }
    catch(...)
    {
        FGASSERT_FALSE;
    }
}

