//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Global multi-redirectable pretty-print output stream for diagnostic feedback.
// Not threadsafe so use ostringstream output option for threads.
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
#include "FgTypes.hpp"

namespace Fg {

std::ostream &
fgnl(std::ostream& ss);

std::ostream &
fgpush(std::ostream& ss);

std::ostream &
fgpop(std::ostream& ss);

std::ostream &
fgreset(std::ostream& ss);  // Reset indent to zero (useful for exception handling)

// ADL won't find this for FgOut::operator<< below so it must be visible up front for clang:
template<class T>
std::ostream &
operator<<(std::ostream &,std::vector<T> const &);

struct  FgOut
{
    bool                m_mute = false;     // Mute all output temporarily. This flag is not thread-safe.

    FgOut();
    ~FgOut();

    // This is a unique global object:
    FgOut(const FgOut &) = delete;
    FgOut & operator=(const FgOut &) = delete;

    bool    setDefOut(bool b);          // Returns true if default output was initially enabled

    bool    defOutEnabled();            // As above. Non-const only for technical reasons.

    void
    logFile(const std::string & fnameUtf8,bool appendFile=true,bool prependDate=true);

    void logFileClose();

    void
    push()
    {
        for (OStr & o : m_streams)
            ++o.indent;
    }

    void
    pop()
    {
        for (OStr & o : m_streams)
            if (o.indent > 0)
                --o.indent;
    }

    uint
    indentLevel() const
    {
        if (m_streams.empty())
            return 0;
        return uint(m_streams[0].indent);
    }

    void
    setIndentLevel(uint);

    void
    reset()
    {
        for (OStr & o : m_streams)
            o.indent = 0;
    }

    FgOut & flush();

    template<typename T>
    FgOut &
    operator<<(T const & arg)
    {
        if (notMute())
            for (OStr & ostr : m_streams)
                (*ostr.pOStr) << arg;
        return *this;
    }
    
    FgOut &
    operator<<(std::ostream & (*manip)(std::ostream&));

    std::string
    getStringStream() const
    {return m_stringStream.str() + "\n"; }

private:
    struct  OStr
    {
        std::ostream        *pOStr;
        size_t              indent = 0;     // Note that atomic cannot be copy constructed

        explicit OStr(std::ostream * p) : pOStr(p) {}

        bool operator==(std::ostream const * r) const {return (pOStr == r); }
    };
    std::vector<OStr>   m_streams;          // Defaults to point to 'cout' unless no CLI, then 'm_stringStream'.
    std::ostringstream  m_stringStream;     // Only used per 'm_stream' above

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

struct  PushIndent
{
    size_t          depth = 1;

    explicit
    PushIndent(const std::string & label)
    {fgout << fgnl << label << std::flush << fgpush; }

    ~PushIndent()
    {pop(); }

    void
    next(const std::string & nextLabel) const
    {
        if (depth > 0)
            fgout << fgpop;
        fgout << fgnl << nextLabel << std::flush;
        if (depth > 0)
            fgout << fgpush;
    }

    void
    pop()
    {
        if (depth > 0) {
            fgout << fgpop;
            --depth;
        }
    }
};

struct  PushLogFile
{
    explicit
    PushLogFile(std::string const & fname,bool append=false)
    {fgout.logFile(fname,append,false); }

    ~PushLogFile()
    {fgout.logFileClose(); }
};

#define FGOUT1(X) fgout << fgnl << #X ": " << (X)

#define FGOUT2(X,Y) fgout << fgnl << #X ": " << (X) << "  " << #Y ": " << (Y)

#define FGOUT3(X,Y,Z) fgout << fgnl                                 \
        << #X ": " << (X) << "  " << #Y ": " << (Y) << " " << #Z ": " << (Z)

#define FGOUT4(X,Y,Z,A) fgout << fgnl                               \
        << #X ": " << (X) << "  " << #Y ": " << (Y) << "  "         \
        << #Z ": " << (Z) << "  " << #A ": " << (A)

}

#endif
