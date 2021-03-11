//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Global multi-redirectable pretty-print output stream for diagnostic feedback.
// Not threadsafe so use ostringstream output option for threads.
// Default output is 'cout' for systems supporting CLI, 'ostringstream' otherwise (Android).
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

// Pretty-print newline manipulators:
std::ostream &          fgnl(std::ostream& ss);
std::ostream &          fgpush(std::ostream& ss);
std::ostream &          fgpop(std::ostream& ss);
std::ostream &          fgreset(std::ostream& ss);  // Reset indent to zero (useful for exception handling)

// ADL won't find this for FgOut::operator<< below so it must be visible up front for clang:
template<class T>
std::ostream &
operator<<(std::ostream &,std::vector<T> const &);

struct  FgOut
{
    FgOut();
    ~FgOut();

    // This is a unique global object:
    FgOut(FgOut const &) = delete;
    void operator=(FgOut const &) = delete;

    bool            setDefOut(bool b);          // Returns true if default output was initially enabled
    bool            defOutEnabled();            // As above. Non-const only for technical reasons.

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

    bool            setMute(bool mute)          // Returns previous mute state
    {
        std::swap(mute,m_mute);
        return mute;
    }

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
            // In this approach we stringize each arg for each output stream, which is perhaps
            // inefficent but also allows for ostreams with different settings:
            for (OStr & ostr : m_streams)
                (*ostr.pOStr) << arg;
        return *this;
    }

    // Manipulators are just passed through to each ostream:
    FgOut &
    operator<<(std::ostream & (*manip)(std::ostream&));

    std::string
    getStringStream() const
    {return m_stringStream.str() + "\n"; }

private:
    bool                    m_mute = false;     // Mute all output temporarily. This flag is not thread-safe.
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

struct  PushMute
{
    bool        prevMute;

    PushMute() : prevMute{fgout.setMute(true)} {}
    explicit PushMute(bool m) : prevMute{fgout.setMute(m)} {}
    ~PushMute() {fgout.setMute(prevMute); }
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

#define FGOUT1(X) fgout << fgnl << #X ": " << fgpush << (X) << fgpop;

#define FGOUT2(X,Y) FGOUT1(X) FGOUT1(Y)

#define FGOUT3(X,Y,Z) FGOUT2(X,Y) FGOUT1(Z)

#define FGOUT4(X,Y,Z,A) FGOUT2(X,Y) FGOUT2(Z,A)

}

#endif
