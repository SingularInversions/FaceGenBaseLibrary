//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgDiagnostics.hpp"
#include "FgThread.hpp"

#define LEAN_AND_MEAN 1
#include <windows.h>
#include <intrin.h>

#pragma intrinsic(_ReadWriteBarrier)

struct FgWinMutexLock
{
    FgWinMutexLock(HANDLE h):m_handle(h)
    {
        FGASSERT(0 == WaitForSingleObject(m_handle,INFINITE));
    }

    ~FgWinMutexLock()
    {
        ReleaseMutex(m_handle);
    }

private:
    FgWinMutexLock(FgWinMutexLock const &);
    void operator=(FgWinMutexLock const &);
    HANDLE m_handle;
};

// Memory barriers inserted as explained here:
// http://www.ddj.com/development-tools/184405772?pgno=4
void
fgRunOnce(FgOnce & once,
          void(*init_routine)())
{
    FGASSERT(init_routine);

    _ReadWriteBarrier();
        /// This will be true most of the time except for the
        /// first time the function is called.
    if(!once.done)
    {
            /// Windows does not provide a statically initializable
            /// mutex so we need to create a named mutex. The process
            /// of creating and using a named mutex is not as
            /// efficient as the alternatives but this is the easiest
            /// one to get right. Named mutexes are guaranteed to be
            /// done once.
            ///
            /// The name of the mutex must be unique to the process
            /// and the address of the once object.
        std::ostringstream os;
        os << "FgOnce-" << reinterpret_cast<std::ptrdiff_t>(&once) 
           << "-" << GetCurrentProcessId();
        {
            FgWinMutexLock mutex(CreateMutexA(0,0,os.str().c_str()));
            if(!once.done)
            {
                init_routine();
                _ReadWriteBarrier();
                once.done = true;
            }
        }
    }
}
