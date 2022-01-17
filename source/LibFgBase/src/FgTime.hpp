//
// Coypright (c) 2022 Singular Inversions Inc. (facegen.com)
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

// Absolute date / time:

uint64          getTimeMs();                        // Time in milliseconds since start of Jan 1 1970 GMT
String          getDateTimeString();                // GMT date and time string in format: yyyy.mm.dd hh:mm:ss
String          getDateTimeString(time_t rawTime);
String          getDateString(time_t rawTime);      // GMT date string in format: yy.mm.dd
// Handy way of naming log files using current date and time (does not append a suffix):
String          getDateTimeFilename();
String          cYearString();

// Relative time:

void            sleepSeconds(uint seconds);         // Cross-platform version always has units of seconds
// Show the time in appropriate units (microseconds, milliseconds, seconds, minutes, hours):
String          toPrettyTime(double durationSeconds);

typedef std::chrono::time_point<std::chrono::steady_clock>  TimerPoint;

struct  Timer
{
    TimerPoint              startTime;

    Timer() {start(); }

    void                start();
    // Returns the time since 'start()' (or object construction) in seconds.
    double              elapsedSeconds() const;
    // Watch out for truncation on short functions; milliseconds is a long time:
    uint64              elapsedMilliseconds() const;
    // Outputs 'label' to 'fgout' newline along with pretty print of duration and restarts the timer:
    void                report(String const & label);
};

std::ostream &
operator<<(std::ostream &,const Timer &);

// Output 'msg' on construction, indent for duration of object scope, then print time elapsed on destruction:
struct PushTimer
{
    TimerPoint              startTime;
    PushTimer(String const & msg);
    ~PushTimer();
};

// Returns true at most once per second:
bool
secondPassedSinceLast();

}

#endif
