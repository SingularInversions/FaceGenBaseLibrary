//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Sohail Somani
//

#ifndef INCLUDED_FGTHREAD_HPP
#define INCLUDED_FGTHREAD_HPP

#include "FgTypes.hpp"

extern bool     fg_debug_thread;        // Set to true for copious debug messages

/// Implementation details
#if defined(__GNUC__)

#include <pthread.h>

typedef pthread_once_t FgOnce;
#define FG_ONCE_INIT PTHREAD_ONCE_INIT;

#elif defined(_WIN32)

struct FgOnce
{
    bool done;
};

#define FG_ONCE_INIT {false}

#else
#  error Threading not configured for this platform
#endif
/// End implementation details

/**
   fgRunOnce provides an implementation of thread-safe once-only
   initialization. Calling fgRunOnce from multiple threads is
   thread-safe.

   Usage:
   \code
   
   static FgOnce once_say_hi = FG_ONCE_INIT;
   
   void say_hi(){std::cout << "Hi!" << std::endl;}

   int main()
   {
     fgRunOnce(once_say_hi,say_hi);
     fgRunOnce(once_say_hi,say_hi);
     fgRunOnce(once_say_hi,say_hi);
     fgRunOnce(once_say_hi,say_hi);
     fgRunOnce(once_say_hi,say_hi);
   }

   Output (note only one line is printed):
   Hi!   

   \endcode
 */
void fgRunOnce(FgOnce & once,
               void(*init_routine)());

#endif
