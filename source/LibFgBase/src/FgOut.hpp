//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Oct 2, 2005
//
// Global multi-redirectable pretty-print output stream for diagnostic feedback.
//
// USE:
//
// Use 'fgout' instead of 'cout' everywhere.
// Use 'fgnl' instead of 'endl' or "\n" everywhere you use 'fgout', and use at the BEGINNING
//   of each output line rather than the end.
// Use 'fgpop' and 'fgpush' to adjust the pretty-print indent level.
//

#ifndef FGOUT_HPP
#define FGOUT_HPP

#include "FgStdLibs.hpp"
#include "FgTypes.hpp"
#include "FgDiagnostics.hpp"
#include "FgStdStream.hpp"

std::ostream &
fgnl(std::ostream& ss);

std::ostream &
fgpush(std::ostream& ss);

std::ostream &
fgpop(std::ostream& ss);

std::ostream &
fgreset(std::ostream& ss);  // Reset indent to zero (useful for exception handling)

// clang doesn't like vector's use of FgOut without forward declaration:
template<class T>
std::ostream &
operator<<(std::ostream & ss,const std::vector<T> & vv);

struct  FgOut
{
    boost::mutex        m_mutex;        // Guard m_indent
    uint                m_indent;
    FgOfstream          m_ofstream;     // Stream to file ?
    std::ostream *      m_stream;       // Arbitrary stream - Null if no stream
    bool                m_mute;         // Mute all output

    FgOut()
    :   m_indent(0),
        m_stream(&std::cout),
        m_mute(false)
    {std::cout.precision(9); }

    explicit
    FgOut(std::ostream & os)
    :   m_indent(0),
        m_stream(&os),
        m_mute(false)
    {}

    bool
    setCout(bool b);

    bool
    coutEnabled()
    {return ((!m_mute) && (m_stream == &std::cout)); }

    void
    logFile(const FgString & fname,bool append=true);

    void
    push()
    {
        m_mutex.lock();
        m_indent++;
        m_mutex.unlock();
    }

    void
    pop()
    {
        m_mutex.lock();
        if (m_indent > 0)   // Can't throw inside mutex lock it appears ...
            --m_indent;
        m_mutex.unlock();
    }

    uint
    indentLevel() const
    {return m_indent; }

    void
    setIndentLevel(uint);

    void
    reset()
    {
        m_mutex.lock();
        m_indent = 0;
        m_mutex.unlock();
    }

    void
    flush();

    template<typename T>
    FgOut &
    operator<<(const T & arg)
    {
        if (notMute())
        {
            if (m_stream)
                (*m_stream) << arg;
            if (m_ofstream.is_open())
                m_ofstream << arg;
        }
        return *this;
    }

    FgOut &
    operator<<(const char * str)
    {
        if (notMute())
        {
            if (m_stream)
                (*m_stream) << str;
            if (m_ofstream.is_open())
                m_ofstream << str;
        }
        return *this;
    }

    FgOut &
    operator<<(std::ostream& (*manip)(std::ostream&));

private:
    bool
    notMute()
    {return ((!m_mute) && (m_stream || m_ofstream.is_open())); }

    FgOut(const FgOut&);              // ofstream objects cannot be copy constructed or operator=
    FgOut& operator=(const FgOut&);   // so we must prevent both for this class as well.
};

extern FgOut      fgout;

struct  FgOutMute
{
    bool        m_mute;

    FgOutMute()
    {m_mute = fgout.m_mute; fgout.m_mute = true; }

    FgOutMute(bool m)
    {m_mute = fgout.m_mute; fgout.m_mute = m; }

    void
    release() const
    {fgout.m_mute = m_mute; }

    ~FgOutMute()
    {release(); }
};

struct  FgPush
{
    FgPush() {fgout << fgpush; }
    ~FgPush() {fgout << fgpop; }
};

struct  FgIndent
{
    explicit
    FgIndent(const std::string & label)
    {fgout << fgnl << label << fgpush; }

    ~FgIndent()
    {fgout << fgpop; }
};

#endif
