//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     May 24, 2005
//

#ifndef FGLIGHT_HPP
#define FGLIGHT_HPP

#include "FgStdLibs.hpp"
#include "FgMatrix.hpp"
#include "FgImage.hpp"

struct  FgLight
{
    FgVect3F       m_colour;        // RGB. INVARIANT: All components > 0
    FgVect3F       m_direction;     // INVARIANT: Unit vector

    FG_SERIALIZE2(m_colour,m_direction);

    FgLight() :
    m_colour(0.6f,0.6f,0.6f),
    m_direction(0.0f,0.0f,1.0f)     // At infinity behind camera (OECS)
    {}
};

struct  FgLighting
{
    FgVect3F            m_ambient;   // RGB. INVARIANT: All components > 0
    vector<FgLight>     m_lights;

    FG_SERIALIZE2(m_ambient,m_lights);

    FgLighting() :
    m_ambient(0.4f,0.4f,0.4f)
    {m_lights.resize(1); }

    FgImgRgbaUb
    createSpecularMap() const;
};

#endif
