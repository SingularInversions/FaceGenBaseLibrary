/*
	ImageMagickObject_.h
*/

#pragma once
#ifndef STRICT
#define STRICT
#endif
#ifndef WINVER
#define WINVER 0x0400
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif						
#ifndef _WIN32_WINDOWS
#define _WIN32_WINDOWS 0x0410
#endif
#ifndef _WIN32_IE
#define _WIN32_IE 0x0400
#endif
#define _ATL_ATTRIBUTES
#define _ATL_APARTMENT_THREADED
#define _ATL_NO_AUTOMATIC_NAMESPACE
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
#define _ATL_ALL_WARNINGS
#pragma warning(disable: 4103) // pragma pack
#define _WINSOCKAPI_
#include <atlbase.h>
#include <atlcom.h>
#include <atlwin.h>
#include <atltypes.h>
#include <atlctl.h>
#include <atlhost.h>
#include <atlsafe.h>
#include <atlconv.h>
#include <atlstr.h>
#include <atlcoll.h>
using namespace ATL;

#include <asptlb.h>         // Active Server Pages Definitions
#include <comsvcs.h>

namespace MagickLib
{
#define _VISUALC_
//#define _MAGICKDLL_
//#define _MAGICKLIB_
#include <wand/MagickWand.h>
#include <magick/semaphore.h>
#include <magick/nt-base.h>
#include <magick/log.h>
#include <magick/hashmap.h>
//#undef _MAGICKDLL_
//#undef _MAGICKLIB_
#undef inline
#undef class
}

// do not use _DbgBreak()
// use ATLASSERT( FALSE ) instead
