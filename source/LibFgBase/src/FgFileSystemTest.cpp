//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

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

namespace Fg {

static
void
testCurrentDirectory(const CLArgs & args)
{
    FGTESTDIR
    try
    {
        char32_t        ch = 0x00004EE5;            // A Chinese character
        Ustring        chinese(ch);
        Ustring        oldDir = fgGetCurrentDir();
        Ustring        dirName = chinese + fgDirSep();
        fgCreateDirectory(dirName);
        fgSetCurrentDir(dirName);
        Ustring        newDir = fgGetCurrentDir();
        Ustring        expected = oldDir + dirName;
        fgSetCurrentDir(oldDir);
        Ustring        restored = fgGetCurrentDir();
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
testOfstreamUnicode(const CLArgs & args)
{
    FGTESTDIR
    char32_t        cent = 0x000000A2;              // The cent sign
    Ustring        test = Ustring(cent);
    Ofstream      ofs(test);
    FGASSERT(ofs);
    ofs.close();
    pathRemove(test);
}

static
void
testReadableFile(const CLArgs & args)
{
    FGTESTDIR
    std::ofstream ofs("testReadableFile.txt");
    FGASSERT(ofs);
    ofs << "Hi";
    ofs.close();
    FGASSERT(fileReadable("testReadableFile.txt"));
    FGASSERT(!fileReadable("This file does not exist"));
}

static
void
testDeleteDirectory(const CLArgs & args)
{
    FGTESTDIR
    char32_t        ch = 0x000000A2;              // The cent sign
    Ustring        cent = Ustring(ch)+"/";
    Ustring        name = "testDeleteDirectory/";
    fgCreateDirectory(name);
    FGASSERT(pathExists(name));
    fgCreateDirectory(name+cent);
    fgSaveXml(name+cent+"a",42);
    fgSaveXml(name+"b",21);
    fgRemoveDirectoryRecursive(name);
    FGASSERT(!pathExists(name));
}

static
void
testRecursiveCopy(const CLArgs & args)
{
    FGTESTDIR
    string          path = "silly-v3.4.7/subdir/";
    fgCreatePath("tst1/"+path);
    Ofstream      ofs("tst1/"+path+"file");
    ofs << "hello";
    ofs.close();
    fgCopyRecursive("tst1","tst2");
    Ifstream      ifs("tst2/"+path+"file");
    string          hello;
    ifs >> hello;
    FGASSERT(hello == "hello");
}

static
void
testExists(const CLArgs &)
{
    FGASSERT(!pathExists("//doesNotExists"));
}

void
fgFileSystemTest(const CLArgs & args)
{
    vector<Cmd>   cmds;
    cmds.push_back(Cmd(testCurrentDirectory,"curDir"));
    cmds.push_back(Cmd(testOfstreamUnicode,"ofsUni"));
    cmds.push_back(Cmd(testReadableFile,"readable"));
    cmds.push_back(Cmd(testDeleteDirectory,"delDir"));
    cmds.push_back(Cmd(testRecursiveCopy,"recurseCopy"));
    cmds.push_back(Cmd(testExists,"exists"));
    fgMenu(args,cmds,true,true,true);
}

}
