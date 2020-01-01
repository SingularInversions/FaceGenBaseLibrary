//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#ifndef FGTIME_HPP
#define FGTIME_HPP

#include "FgStdLibs.hpp"
#include "FgStdString.hpp"
#include "FgTypes.hpp"
#include "FgOut.hpp"

namespace Fg {

// Cross-platform version always has units of seconds:
void
fgSleep(uint seconds);

// Time in milliseconds since start of Jan 1 1970 GMT:
uint64
fgTimeMs();

// GMT date and time string in format: yyyy.mm.dd hh:mm:ss
String
fgDateTimeString();

String
fgDateTimeString(time_t rawTime);

// GMT date string in format: yy.mm.dd
String
fgDate(time_t rawTime);

// Handy way of naming log files using current date and time (does not append a suffix):
String
fgDateTimePath();

String
yearString();

struct  FgTimer
{
    uint64      m_startTime;

    FgTimer()
    {start(); }

    void
    start()
    {m_startTime = fgTimeMs(); }

    // Returns the time since 'start()' (or object construction) in seconds.
    double
    read() const
    {
        uint64  stopTime = fgTimeMs();
        return (double(stopTime - m_startTime) / 1000.0);
    }

    uint64
    readMs() const
    {return fgTimeMs()-m_startTime; }

    // Outputs 'label' to 'fgout' newline along with the time taken in 'ms', then resets the timer:
    void
    report(String const & label);
};

std::ostream &
operator<<(std::ostream &,const FgTimer &);

struct FgTimeScope
{
    uint64          startTime;

    FgTimeScope(String const & msg)
    {
        fgout << fgnl << "Beginning " << msg << ":" << fgpush << fgnl;
        startTime = fgTimeMs();
    }

    ~FgTimeScope()
    {
        uint64      t = fgTimeMs() - startTime;
        fgout << fgpop << fgnl << "Done. " << t << " ms ";
    }
};

// Returns true at most once per second:
bool
fgTick();

}

#endif
