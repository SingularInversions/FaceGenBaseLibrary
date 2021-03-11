//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
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
sleepSeconds(uint seconds);

// Time in milliseconds since start of Jan 1 1970 GMT:
uint64
getTimeMs();

// GMT date and time string in format: yyyy.mm.dd hh:mm:ss
String
getDateTimeString();

String
getDateTimeString(time_t rawTime);

// GMT date string in format: yy.mm.dd
String
getDateString(time_t rawTime);

// Handy way of naming log files using current date and time (does not append a suffix):
String
getDateTimeFilename();

String
cYearString();

// Show the time in appropriate units (milliseconds, seconds, minutes, hours, days):
String
msToPrettyTime(uint64 durationInMilliseconds);

struct  Timer
{
    uint64      m_startTime;

    Timer()
    {start(); }

    void
    start()
    {m_startTime = getTimeMs(); }

    // Returns the time since 'start()' (or object construction) in seconds.
    double
    read() const
    {
        uint64  stopTime = getTimeMs();
        return (double(stopTime - m_startTime) / 1000.0);
    }

    uint64
    readMs() const
    {return getTimeMs()-m_startTime; }

    // Outputs 'label' to 'fgout' newline along with pretty print of duration
    void
    report(String const & label);
};

std::ostream &
operator<<(std::ostream &,const Timer &);

struct PushTimer
{
    uint64          startTime;

    PushTimer(String const & msg)
    {
        fgout << fgnl << "Beginning " << msg << ":" << fgpush << fgnl;
        startTime = getTimeMs();
    }

    ~PushTimer()
    {
        uint64      t = getTimeMs() - startTime;
        fgout << fgpop << fgnl << "Completed in " << msToPrettyTime(t);
    }
};

// Returns true at most once per second:
bool
secondPassedSinceLast();

inline
double
toSeconds(std::chrono::nanoseconds nano)
{return nano/std::chrono::nanoseconds(1) / 1000000000.0;}

}

#endif
