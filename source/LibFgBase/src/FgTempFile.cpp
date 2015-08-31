//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Sohail Somani
//

#include "stdafx.h"

#include "FgTempFile.hpp"
#include "FgOut.hpp"
#include "FgFileSystem.hpp"

static bool g_keep_temp_files = false;

FgTempFile::FgTempFile(const std::string & filename):
    m_filename(filename)
{}

FgTempFile::~FgTempFile()
{
    if(!g_keep_temp_files)
        std::remove(m_filename.c_str());
}

const std::string &
FgTempFile::filename() const
{
    return m_filename;
}

void
FgTempFile::setKeepTempFiles(bool b)
{
    g_keep_temp_files = b;
}

bool
FgTempFile::getKeepTempFiles()
{
    return g_keep_temp_files;
}

FgPushTempDir::FgPushTempDir(const std::string & name)
: m_new(name)
{
    m_original = fgGetCurrentDir();
    fgCreateDirectory(m_new);                 // Do nothing if already exists
    fgSetCurrentDir(FgString(m_new));
}

FgPushTempDir::~FgPushTempDir()
{
    if (fgSetCurrentDir(m_original,false))  // Can't do anything if this fails
        fgRemoveDirectory(FgString(m_new));       // Won't delete if non-empty
}
