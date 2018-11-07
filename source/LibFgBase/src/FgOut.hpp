//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Oct 2, 2005
//
// Global multi-redirectable pretty-print output stream for diagnostic feedback.
// Output (not ordering) is threadsafe but modification of output selections is not.
// Default output is 'cout' for systems supporting CLI, 'stringstream' otherwise (Android).
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
#include "FgString.hpp"

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
    // TODO: Use std::atomic instead of mutexes around this:
    bool                m_mute = false;     // Mute all output temporarily. This flag is not thread-safe.

    FgOut();

    // This is a unique global object:
    FgOut(const FgOut &) = delete;
    FgOut & operator=(const FgOut &) = delete;

    bool    setDefOut(bool b);          // Returns true if default output was initially enabled

    bool    defOutEnabled();            // As above. Non-const only for technical reasons.

    void
    logFile(const FgString & fname,bool append=true,bool prependDate=true);

    void logFileClose();

    void
    push()
    {
        m_mutex.lock();
        ++m_indent;
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

    FgOut & flush();

    template<typename T>
    FgOut &
    operator<<(const T & arg)
    {
        if (notMute())
            for (auto s : m_streams)
                (*s) << arg;
        return *this;
    }

    FgOut &
    operator<<(std::ostream& (*manip)(std::ostream&));

    std::string
    getStringStream() const
    {return m_stringStream.str() + "\n"; }

private:
    std::vector<std::ostream *> m_streams;  // Defaults to point to 'cout' unless no CLI, then 'm_stringStream'.
    std::ostringstream  m_stringStream;     // Only used per 'm_stream' above
    std::mutex        m_mutex;            // Guard m_indent to keep it thread-safe:
    uint                m_indent = 0;

    bool
    notMute()
    {return (!m_mute && !m_streams.empty()); }

    std::ostream *
    defOut();
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

struct  FgOutPush
{
    explicit
    FgOutPush(const std::string & label)
    {fgout << fgnl << label << fgpush; }

    ~FgOutPush()
    {fgout << fgpop; }
};

#define FG_HI fgout << fgnl << "HI ! (" << __FILE__ << ": " << __LINE__ << ")" << std::flush

#define FG_HI1(X) fgout << fgnl << #X ": " << (X) << std::flush

#define FG_HI2(X,Y) fgout << fgnl << #X ": " << (X) << " " << #Y ": " << (Y) << std::flush

#define FG_HI3(X,Y,Z) fgout << fgnl << #X ": " << (X) << " " << #Y ": " << (Y) << " " << #Z ": " << (Z) << std::flush

#define FG_HI4(X,Y,Z,A) fgout << fgnl << #X ": " << (X) << " "     \
         << #Y ": " << (Y) << " "                                   \
         << #Z ": " << (Z) << " "                                   \
         << #A ": " << (A) << std::flush

#endif
