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
#include "FgCommand.hpp"

FgTempFile::FgTempFile(const std::string & filename):
    m_filename(filename)
{}

FgTempFile::~FgTempFile()
{
    if(!fgKeepTempFiles())
        std::remove(m_filename.c_str());
}

const std::string &
FgTempFile::filename() const
{
    return m_filename;
}
