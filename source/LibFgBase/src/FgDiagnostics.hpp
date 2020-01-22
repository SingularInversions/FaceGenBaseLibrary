//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// USE:
//
// FGASSERT is retained release builds, FG_ASSERT_FAST is not.
// Use FGASSERT for unexpected errors (use 'fgThrow' otherwise)
//

#ifndef FGDIAGNOSTICS_HPP
#define FGDIAGNOSTICS_HPP

#include "FgException.hpp"

namespace Fg {

// With visual studio the __FILE__ macro always includes the full path in release compiles,
// there is no way to specify otherwise. When this includes unicode, the literal becomes
// a wchar_t* instead of char*. Macros can be used to cast to wchar_t* in both cases, then
// dealt with but I haven't bothered; currently the source will not compile properly in a
// non-ascii path.

// Remove path from filename (see above).
// This simple version does NOT handle base filenames after Windows drive specifiers (eg. C:filename)
// as it only detects the '\' character:
std::string
pathToName(const char * asciiFilePath);

std::string
fgDiagString(const char *fname,int line);

// If you're trying to pass a UTF-8 'msg' here, you should probably be using 'fgThrow' instead:
void
fgAssert(const char * fname,int line,const std::string & msg = "");

// Crude warning system outputs to cout. Use when we don't want to throw in release distros.
// Currently just outputs to fgout but could add telemetry, special dev behaviour:
void fgWarn(const char * fname,int line,const std::string & msg="");
void fgWarn(const std::string & msg,const std::string & dataUtf8="");

}

// We use an 'if' 'else' structure for the macro to avoid the dangling 'else' bug.
// Leave off the semi-colon on the second line to force a compile error:
#define FGASSERT(X)                                                     \
    if(X) (void) 0;                                                     \
    else Fg::fgAssert(__FILE__,__LINE__)

#define FGASSERT1(X,msg)                                            \
    if(X) (void) 0;                                                     \
    else Fg::fgAssert(__FILE__,__LINE__,msg)

#ifdef _DEBUG

#define FGASSERT_FAST(X)                                                \
    if(X) (void) 0;                                                     \
    else Fg::fgAssert(__FILE__,__LINE__)

#define FGASSERT_FAST2(X,Y)                     \
    if(X) (void) 0;                             \
    else Fg::fgAssert(__FILE__,__LINE__,Y)          \

#else

#define FGASSERT_FAST(X)
#define FGASSERT_FAST2(X,Y)

#endif

// Use this instead of FGASSERT(false) to avoid warnings for constant conditionals:
#define FGASSERT_FALSE                                                  \
    Fg::fgAssert(__FILE__,__LINE__)

#define FGASSERT_FALSE1(msg)                                         \
    Fg::fgAssert(__FILE__,__LINE__,msg)

// printf-style debugging:

#define FG_HI std::cout << "\nHI! (" << __FILE__ << ": " << __LINE__ << ")" << std::flush

#define FG_HI1(X) std::cout << "\nHI! " << #X ": " << (X) << std::flush

#define FG_HI2(X,Y) std::cout << "\nHI! " << #X ": " << (X) << " " << #Y ": " << (Y) << std::flush

#define FG_HI3(X,Y,Z) std::cout << "\nHI! " << #X ": " << (X) << " "       \
         << #Y ": " << (Y) << " "                                           \
         << #Z ": " << (Z) << std::flush

#define FG_HI4(X,Y,Z,A) std::cout << "\nHI! " << #X ": " << (X) << " "     \
         << #Y ": " << (Y) << " "                                           \
         << #Z ": " << (Z) << " "                                           \
         << #A ": " << (A) << std::flush

// Use string for debug build, empty string otherwise:
#ifdef _DEBUG
#define FG_DBG_STR(str) str
#else
#define FG_DBG_STR(str) ""
#endif

#define FGWARN Fg::fgWarn(__FILE__,__LINE__)
#define FGWARN1(X) Fg::fgWarn(__FILE__,__LINE__,X)

#endif
