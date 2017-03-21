//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     March 27, 2006
//

#ifndef FGTIME_HPP
#define FGTIME_HPP

#include "FgStdLibs.hpp"
#include "FgTypes.hpp"
#include "FgOut.hpp"

// Cross-platform version always has units of seconds:
void
fgSleep(uint seconds);

// Time in milliseconds since start of Jan 1 1970 GMT:
uint64
fgTimeMs();

// GMT date and time string in format: yyyy.mm.dd hh:mm:ss
std::string
fgDateTimeString();

std::string
fgDateTimeString(time_t rawTime);

// GMT date string in format: yy.mm.dd
std::string
fgDate(time_t rawTime);

// Handy way of naming log files using current date and time (does not append a suffix):
std::string
fgDateTimePath();

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
};

std::ostream &
operator<<(std::ostream &,const FgTimer &);

struct FgTimeScope
{
    FgTimer     timer;

    FgTimeScope(const std::string & msg)
    {fgout << fgnl << "Beginning " << msg << ":" << fgpush; }

    ~FgTimeScope()
    {fgout << fgpop << fgnl << timer; }
};

// Returns true at most once per second:
bool
fgTick();

#endif
