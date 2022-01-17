//
// Coypright (c) 2022 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//


#include "stdafx.h"
#include "FgTime.hpp"
#include "FgString.hpp"
#include "FgOut.hpp"
#include "FgStdString.hpp"

using namespace std;

namespace Fg {

std::string
getDateTimeString(time_t rawTime)
{
    struct tm   *fmtTime = gmtime(&rawTime);
    ostringstream   oss;
    oss << fmtTime->tm_year+1900 << "."
        << setw(2) << setfill('0') 
        << fmtTime->tm_mon+1 << "."
        << setw(2) << setfill('0') 
        << fmtTime->tm_mday << " "
        << setw(2) << setfill('0') 
        << fmtTime->tm_hour << ":"
        << setw(2) << setfill('0') 
        << fmtTime->tm_min << ":"
        << setw(2) << setfill('0') 
        << fmtTime->tm_sec;
    return oss.str();
}

std::string
getDateTimeString()
{
    time_t          rawTime;
    time(&rawTime);
    return getDateTimeString(rawTime);
}

std::string
getDateString(time_t rawTime)
{
    struct tm   *fmtTime = gmtime(&rawTime);
    ostringstream   oss;
    oss << setw(2) << setfill('0') 
        << fmtTime->tm_year-100 << "."
        << setw(2) << setfill('0') 
        << fmtTime->tm_mon+1 << "."
        << setw(2) << setfill('0') 
        << fmtTime->tm_mday << " ";
    return oss.str();
}

std::string
getDateTimeFilename()
{
    time_t          rawtime;
    time(&rawtime);
    const int       buffSize = 256;
    char            buffer[buffSize] = {0};
    tm              *timeinfo = localtime(&rawtime);
    strftime(buffer,buffSize-1,"%y%m%d_%H%M%S",timeinfo);
    return string(buffer);
}

String
cYearString()
{
    time_t          rawTime;
    time(&rawTime);
    struct tm *     fmtTime = gmtime(&rawTime);
    ostringstream   oss;
    oss << fmtTime->tm_year+1900;
    return oss.str();
}

String
toPrettyTime(double durationSeconds)
{
    double                      d = abs(durationSeconds);   // In case of negative duration
    Svec<pair<double,String> > const units {
        {3600.0,"hours"},
        {60.0,"minutes"},
        {1.0,"seconds"},
        {0.001,"milliseconds"},
        {0.000001,"microseconds"},
    };
    size_t                      choice = 0;
    while ((d < units[choice].first) && (choice+1 < units.size()))
        ++choice;
    pair<double,String> const & unit = units[choice];
    return toStrPrec(durationSeconds/unit.first,4) + " " + unit.second;
}

std::ostream &
operator<<(std::ostream & os,const Timer & t)
{
    double      et = t.elapsedSeconds();
    return os << "Elapsed time: " << toStrPrec(et,4) << " s";
}

void
Timer::start()
{
    startTime = std::chrono::steady_clock::now();
}

double
Timer::elapsedSeconds() const
{
    TimerPoint          stopTime = std::chrono::steady_clock::now();
    // The subtraction below returns a duration which with MSVC is duration<long long,std::nano>
    // The cast to duration<double> converts to seconds
    // Then count() returns a plain double
    return std::chrono::duration<double>{stopTime - startTime}.count();
}

uint64
Timer::elapsedMilliseconds() const
{
    TimerPoint              stopTime = std::chrono::steady_clock::now();
    auto                    delta = stopTime - startTime;   // duration<long long,std::nano> w/ MSVC
    // Converting between integer time gauges requires this explicit cast,
    // Then count() returns a plan long long
    long long               deltaMs = std::chrono::duration_cast<std::chrono::milliseconds>(delta).count();
    return uint64(deltaMs);     // Can't be negative
}

void
Timer::report(String const & label)
{
    fgout << fgnl << label << ": " << toPrettyTime(elapsedSeconds());
    start();
}


PushTimer::PushTimer(String const & msg)
{
    fgout << fgnl << "Beginning " << msg << ": " << fgpush;
    startTime = std::chrono::steady_clock::now();
}

PushTimer::~PushTimer()
{
    TimerPoint              stopTime = std::chrono::steady_clock::now();
    double                  deltaSeconds =  std::chrono::duration<double>{stopTime - startTime}.count();
    fgout << fgpop << fgnl << "Completed in " << toPrettyTime(deltaSeconds);
}

bool
secondPassedSinceLast()
{
    static Timer  timer;
    if (timer.elapsedSeconds() > 1.0) {
        timer.start();
        return true;
    }
    return false;
}

}
