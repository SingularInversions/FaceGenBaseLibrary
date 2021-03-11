//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgException.hpp"
#include "FgDiagnostics.hpp"
#include "FgMain.hpp"
#include "FgString.hpp"

using namespace std;

namespace Fg {

static string const   state1 = "State1";
static String8 const state2("State2");
// TODO: Check for a specific language translation here once implemented:
static string const   tr_correct = 
    "While executing func1 with state : State1\n"
    "Problem in func2 with state : State2";
static string const   no_tr_correct = 
    "While executing func1 with state : State1\n"
    "Problem in func2 with state : State2";

static void func2()
{
    fgThrow("Problem in func2 with state",state2);
}

static void func1()
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

struct FgMyException : public FgException
{
    FgMyException(std::string const & s) : FgException(s,string()) {}
    FgMyException(std::string const & s, std::string const & m) : FgException(s,m) {}
};

void
testExceptions(CLArgs const &)
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
}

}
