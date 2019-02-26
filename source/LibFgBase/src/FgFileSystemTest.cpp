//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     March 11, 2009
//

#include "stdafx.h"

#include "FgPlatform.hpp"
#include "FgFileSystem.hpp"
#include "FgException.hpp"
#include "FgOut.hpp"
#include "FgStdStream.hpp"
#include "FgTestUtils.hpp"
#include "FgScopeGuard.hpp"
#include "FgMetaFormat.hpp"
#include "FgCommand.hpp"

using namespace std;

static
void
testCurrentDirectory(const FgArgs & args)
{
    FGTESTDIR
    try
    {
        char32_t        ch = 0x00004EE5;            // A Chinese character
        FgString        chinese(ch);
        FgString        oldDir = fgGetCurrentDir();
        FgString        dirName = chinese + fgDirSep();
        fgCreateDirectory(dirName);
        fgSetCurrentDir(dirName);
        FgString        newDir = fgGetCurrentDir();
        FgString        expected = oldDir + dirName;
        fgSetCurrentDir(oldDir);
        FgString        restored = fgGetCurrentDir();
        FGASSERT(fgRemoveDirectory(dirName));
        fgout << fgnl << "Original directory:    " << oldDir.as_utf8_string();
        fgout << fgnl << "New current directory: " << newDir.as_utf8_string();
        fgout << fgnl << "Expected directory:    " << expected.as_utf8_string();
        fgout << fgnl << "Restored directory:    " << restored.as_utf8_string();
        FGASSERT(expected == newDir);
    }
    catch (FgExceptionNotImplemented const & e) 
    {
        fgout << e.no_tr_message();
    }
}

static
void
testOfstreamUnicode(const FgArgs & args)
{
    FGTESTDIR
    char32_t        cent = 0x000000A2;              // The cent sign
    FgString        test = FgString(cent);
    FgOfstream      ofs(test);
    FGASSERT(ofs);
    ofs.close();
    fgRemoveFile(test);
}

static
void
testReadableFile(const FgArgs & args)
{
    FGTESTDIR
    std::ofstream ofs("testReadableFile.txt");
    FGASSERT(ofs);
    ofs << "Hi";
    ofs.close();
    FGASSERT(fgFileReadable("testReadableFile.txt"));
    FGASSERT(!fgFileReadable("This file does not exist"));
}

static
void
testDeleteDirectory(const FgArgs & args)
{
    FGTESTDIR
    char32_t        ch = 0x000000A2;              // The cent sign
    FgString        cent = FgString(ch)+"/";
    FgString        name = "testDeleteDirectory/";
    fgCreateDirectory(name);
    FGASSERT(fgExists(name));
    fgCreateDirectory(name+cent);
    fgSaveXml(name+cent+"a",42);
    fgSaveXml(name+"b",21);
    fgRemoveDirectoryRecursive(name);
    FGASSERT(!fgExists(name));
}

static
void
testRecursiveCopy(const FgArgs & args)
{
    FGTESTDIR
    string          path = "silly-v3.4.7/subdir/";
    fgCreatePath("tst1/"+path);
    FgOfstream      ofs("tst1/"+path+"file");
    ofs << "hello";
    ofs.close();
    fgCopyRecursive("tst1","tst2");
    FgIfstream      ifs("tst2/"+path+"file");
    string          hello;
    ifs >> hello;
    FGASSERT(hello == "hello");
}

static
void
testExists(const FgArgs &)
{
    FGASSERT(!fgExists("//doesNotExists"));
}

void
fgFileSystemTest(const FgArgs & args)
{
    vector<FgCmd>   cmds;
    cmds.push_back(FgCmd(testCurrentDirectory,"curDir"));
    cmds.push_back(FgCmd(testOfstreamUnicode,"ofsUni"));
    cmds.push_back(FgCmd(testReadableFile,"readable"));
    cmds.push_back(FgCmd(testDeleteDirectory,"delDir"));
    cmds.push_back(FgCmd(testRecursiveCopy,"recurseCopy"));
    cmds.push_back(FgCmd(testExists,"exists"));
    fgMenu(args,cmds,true,true,true);
}
