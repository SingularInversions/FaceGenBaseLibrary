//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Oct 2, 2005
//

#include "stdafx.h"

#include "FgOut.hpp"
#include "FgStdStream.hpp"
#include "FgDiagnostics.hpp"
#include "FgException.hpp"
#include "FgTime.hpp"

using namespace std;

// Only this single global instance should ever be instantiated.
// Note that 'fgout' can't be used in global variable constructors since it's
// not guaranteed to be constructed yet itself:
FgOut               fgout;

// Keep this here to avoid excess header dependencies:
static FgOfstream   s_ofs;

ostream &
fgnl(ostream & ss)
{
    ss << '\n';
    uint    il = fgout.indentLevel();
    for (uint ii=0; ii<il; ii++)
        ss << "|   ";
    return ss;
}

ostream &
fgpush(ostream & ss)
{
    fgout.push();
    return ss;
}

ostream &
fgpop(ostream & ss)
{
    fgout.pop();
    return ss;
}

ostream &
fgreset(ostream & ss)
{
    fgout.reset();
    return ss << '\n';
}

FgOut::FgOut()
{
    m_streams.push_back(defOut());
    m_streams.back()->precision(9);
}

bool
FgOut::setDefOut(bool b)
{
    auto        it = find(m_streams.begin(),m_streams.end(),defOut());
    if (it == m_streams.end()) {
        if (b)
            m_streams.push_back(defOut());
        return false;
    }
    else {
        if (!b)
            m_streams.erase(it);
        return true;
    }
}

bool
FgOut::defOutEnabled()
{
    auto        it = find(m_streams.begin(),m_streams.end(),defOut());
    return (it != m_streams.end());
}

void
FgOut::logFile(const FgString & fname,bool append,bool prependDate)
{
    if (s_ofs.is_open())
        s_ofs.close();
    s_ofs.open(fname,append,true);
    s_ofs.precision(9);
    if (prependDate)
        s_ofs << '\n' << fgDateTimeString() << '\n';
    auto    it = find(m_streams.begin(),m_streams.end(),&s_ofs);
    if (it == m_streams.end())
        m_streams.push_back(&s_ofs);
}

void
FgOut::logFileClose()
{
    if (s_ofs.is_open())
        s_ofs.close();
    auto    it = find(m_streams.begin(),m_streams.end(),&s_ofs);
    if (it != m_streams.end())
        m_streams.erase(it);
}

void
FgOut::setIndentLevel(uint l)
{
    m_mutex.lock();
    m_indent = l;
    m_mutex.unlock();
}

FgOut &
FgOut::flush()
{
    if (!m_mute)
        for (auto s : m_streams)
            (*s) << std::flush;
    return *this;
}

FgOut &
FgOut::operator<<(std::ostream& (*manip)(std::ostream&))
{
    if (notMute())
    {
        // Handle the case of fgpush and fgpop explicitly since otherwise they
        // may be passed on to both streams and double called resulting in twice
        // the indenting:
        if (manip == fgpush) {
            m_mutex.lock();
            m_indent++;
            m_mutex.unlock(); }
        else if (manip == fgpop) {
            m_mutex.lock();
            if (m_indent > 0)       // Don't throw or warn (output issues), just ignore
                m_indent--;
            m_mutex.unlock(); }
        else if (manip == fgnl) {
            for (auto s : m_streams) {
                (*s) << '\n';
                for (uint ii=0; ii<indentLevel(); ii++)
                    (*s) << "|   ";
            }
        }
        else {
            for (auto s : m_streams)
                (*s) << manip;
        }
    }
    return *this;
}

std::ostream *
FgOut::defOut()
{
#ifdef __ANDROID__
    // std::cout not supported by Android (supposed to pipe to /dev/null but in fact crashes with invalid pointer)
    return &m_stringStream;
#else
    return &std::cout;
#endif
}
