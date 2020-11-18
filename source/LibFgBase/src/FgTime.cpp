//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
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
msToPrettyTime(uint64 durationInMilliseconds)
{
    double          d = durationInMilliseconds;
    if (d < 1000.0)
        return toStr(d) + " ms";
    d /= 1000.0;
    if (d < 60.0)
        return toStrPrecision(d,4) + " s";
    d /= 60.0;
    if (d < 60.0)
        return toStrPrecision(d,4) + " min";
    d /= 60.0;
    if (d < 24.0)
        return toStrPrecision(d,4) + " hours";
    d /= 24.0;
    return toStrPrecision(d,4) + " days";
}


std::ostream &
operator<<(std::ostream & os,const Timer & t)
{
    double      et = t.read();
    return os << "Elapsed time: " << toStrPrecision(et,4) << " s";
}

void
Timer::report(string const & label)
{
    uint64      duration = readMs();
    fgout << fgnl << label << ": " << msToPrettyTime(duration);
    start();
}

bool
secondPassedSinceLast()
{
    static Timer  timer;
    if (timer.read() > 1.0) {
        timer.start();
        return true;
    }
    return false;
}

}
