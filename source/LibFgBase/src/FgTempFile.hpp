//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Sohail Somani
//
// Automatically delete the file of the given name when this object goes out of scope.
//

#ifndef INCLUDED_FGTEMPFILE_HPP
#define INCLUDED_FGTEMPFILE_HPP

#include "FgStdLibs.hpp"
#include "FgNonCopyable.hpp"
#include "FgString.hpp"

struct FgTempFile :
    private FgNonCopyable
{
        // Create a temporary file with the given name
    explicit
    FgTempFile(const std::string & filename);

    ~FgTempFile();

    const std::string &
    filename() const;
    
private:
    std::string m_filename;
};

#endif // FGTEMPFILE_HPP
