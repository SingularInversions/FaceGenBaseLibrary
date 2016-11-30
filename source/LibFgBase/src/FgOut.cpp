//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Oct 2, 2005
//

#include "stdafx.h"
#include "FgDiagnostics.hpp"
#include "FgException.hpp"
#include "FgOut.hpp"

using namespace std;

// Only this single global instance should ever be instantiated.
// Note that 'fgout' can't be used in global variable constructors since it's
// not guaranteed to be constructed yet itself:
FgOut      fgout;

ostream &
fgnl(ostream & ss)
{
    ss << endl;                         // includes a flush
    uint    il = fgout.indentLevel();
    for (uint ii=0; ii<il; ii++)
        ss << "    ";
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
    return ss << endl;
}

bool
FgOut::setCout(bool b)
{
    m_mutex.lock();
    bool    ret = (m_stream != 0);
    if (b) {
        m_stream = &std::cout;
        std::cout.precision(9); }
    else
        m_stream = 0;
    m_mutex.unlock();
    return ret;
}

void
FgOut::logFile(const FgString & fname,bool append,bool prependDate)
{
    m_mutex.lock();
    if (m_ofstream.is_open())
        m_ofstream.close();
    m_ofstream.open(fname,append,true);
    m_ofstream.precision(9);
    if (prependDate)
        m_ofstream << '\n' << fgDateTimeString() << endl;
    m_mutex.unlock();
}

void
FgOut::flush() 
{ 
    m_mutex.lock();
    if (m_ofstream)
        m_ofstream.flush();
    if (m_stream && *m_stream)
        m_stream->flush();
    m_mutex.unlock();
}

void
FgOut::setIndentLevel(uint l)
{
    m_mutex.lock();
    m_indent = l;
    m_mutex.unlock();
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
            if (m_stream)
                *m_stream << endl;
            if (m_ofstream.is_open())
                m_ofstream << endl;
            for (uint ii=0; ii<indentLevel(); ii++) {
                if (m_stream)
                    *m_stream << "    ";
                if (m_ofstream.is_open())
                    m_ofstream << "    "; }
        }
        else {
            if (m_stream)
                *m_stream << manip;
            if (m_ofstream.is_open())
                m_ofstream << manip; }
    }
    return *this;
}
