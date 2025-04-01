//
// Copyright (c) 2025 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgFile.hpp"
#include "FgTime.hpp"

using namespace std;

namespace Fg {

String              FgException::englishMessage() const
{
    String              ret;
    for (Context const & ctxt : contexts) {
        ret += ctxt.english;
        if (!ctxt.dataUtf8.empty())
            ret += " : " + ctxt.dataUtf8;
        ret += "\n";
    }
    return ret;
}

String              FgException::nativeMessage() const
{
    String              ret;
    for (Context const & ctxt : contexts) {
        if (ctxt.native.empty())        // TODO: translate the english language text into native local language
            ret += ctxt.english;
        else
            ret += ctxt.native;
        if (!ctxt.dataUtf8.empty())
            ret += " : " + ctxt.dataUtf8;
        ret += "\n";
    }
    return ret;
}

string              toFilePosString(char const * path_cstr,int line)
{
    // don't use Fg::Path here since we cannot throw within this function:
    string          path = path_cstr;
    size_t          pos = 0;
    for (size_t ii=0; ii<path.size(); ++ii) {
        char            ch = path[ii];
        if ((ch == '\\') || (ch == '/'))        // win or nix
            pos = ii+1;
    }
    string          fname;
    if (pos < path.size())
        fname = path.substr(pos);               // to end of string
    return fname + ":" + toStr(line);
}

void                fgAssert(char const * fname,int line,string const &  msg)
{
    fgThrow("Internal program error",msg+"\n"+toFilePosString(fname,line));
}

void                fgWarn(const std::string & msg,const std::string & dataUtf8)
{
    fgout << "\nWARNING: " << msg << ": " << dataUtf8;
}

void                fgWarn(char const * fname,int line,string const & msg)
{
    fgout << "\nWARNING: " <<  msg << " (" << toFilePosString(fname,line) << ")";
}

static Ofstream     s_ofs;
// Only this single global instance should ever be instantiated.
// Note that 'fgout' can't be used in global variable constructors since it's not guaranteed to be constructed yet itself:
FgOut               fgout;
FgOut               nout {false};

ostream &           fgnl(ostream & ss)
{
    ss << '\n';
    uint                il = fgout.indentLevel();
    for (uint ii=0; ii<il; ii++)
        ss << "|   ";
    return ss;
}

ostream &           fgpush(ostream & ss)
{
    fgout.push();
    return ss;
}

ostream &           fgpop(ostream & ss)
{
    fgout.pop();
    return ss;
}

ostream &           fgreset(ostream & ss)
{
    fgout.reset();
    return ss << '\n';
}

FgOut::FgOut(bool enable)
{
    if (enable) {
        m_streams.push_back(OStr{defOut()});
        m_streams.back().pOStr->precision(9);
    }
}

FgOut::~FgOut()
{
    if (s_ofs.is_open())
        s_ofs.close();
}

bool                FgOut::setDefOut(bool b)
{
    for (auto it=m_streams.begin(); it!=m_streams.end(); ++it) {
        if (it->pOStr == defOut()) {
            if (!b)
                m_streams.erase(it);
            return true;
        }
    }
    if (b)
        m_streams.push_back(OStr(defOut()));
    return false;
}

bool                FgOut::defOutEnabled()
{
    for (auto it=m_streams.begin(); it!=m_streams.end(); ++it)
        if (it->pOStr == defOut())
            return true;
    return false;
}

void                FgOut::logFile(const std::string & fnameUtf8,bool appendFile,bool prependDate)
{
    if (s_ofs.is_open())
        s_ofs.close();
    s_ofs.open(fnameUtf8,appendFile,true);
    s_ofs.precision(9);
    if (prependDate)
        s_ofs << '\n' << getDateTimeString() << '\n';
    auto    it = find(m_streams.begin(),m_streams.end(),&s_ofs);
    if (it == m_streams.end())
        m_streams.push_back(OStr{&s_ofs});
}

void                FgOut::logFileClose()
{
    if (s_ofs.is_open())
        s_ofs.close();
    auto    it = find(m_streams.begin(),m_streams.end(),&s_ofs);
    if (it != m_streams.end())
        m_streams.erase(it);
}

void                FgOut::setIndentLevel(uint l)
{
    for (OStr & o : m_streams)
        o.indent = l;
}

FgOut &             FgOut::flush()
{
    for (auto & s : m_streams)
        (*s.pOStr) << std::flush;
    return *this;
}

FgOut &             FgOut::operator<<(std::ostream& (*manip)(std::ostream&))
{
    // Handle the case of fgpush and fgpop explicitly since otherwise they
    // may be passed on to both streams and double called resulting in twice
    // the indenting:
    if (manip == fgpush) {
        for (OStr & o : m_streams)
            ++o.indent;
    }
    else if (manip == fgpop) {
        for (OStr & o : m_streams)
            if (o.indent > 0)
                --o.indent;
    }
    else if (manip == fgnl) {
        for (OStr & o : m_streams) {
            (*o.pOStr) << '\n';
            for (uint ii=0; ii<o.indent; ii++)
                (*o.pOStr) << "|   ";
        }
    }
    else {
        for (auto & s : m_streams)
            (*s.pOStr) << manip;
    }
    return *this;
}

void                FgOut::addStream(ostream * os,size_t indentLevel)
{
    for (OStr ostr : m_streams)
        if (ostr.pOStr == os)
            return;
    m_streams.emplace_back(os,indentLevel);
}

size_t              FgOut::delStream(ostream * os)
{
    for (auto it=m_streams.begin(); it!=m_streams.end(); ++it) {
        if (it->pOStr == os) {
            size_t          indentLevel = it->indent;
            m_streams.erase(it);
            return indentLevel;
        }
    }
    return 0;
}

std::ostream *      FgOut::defOut()
{
#ifdef __ANDROID__
    // std::cout not supported by Android (supposed to pipe to /dev/null but in fact crashes with invalid pointer)
    return &m_stringStream;
#else
    return &std::cout;
#endif
}

}
