// Basic ATL/COM wrapper for the various ImageMagick++ classes.

// Written by: Paul Mrozowski
//             Senior Applications Developer
//     
//             Kirtland Associates, Inc.
//             1220 Morse
//             Suite 200
//             Royal Oak, MI 48067
//             Phone: 248.542.2675
//
//             Email: pcm@kirtlandsys.com
//
//             Copyright(C) 2002 - Kirtland Associates

// MagickCOM.cpp : Implementation of DLL Exports.


// Note: Proxy/Stub Information
//      To build a separate proxy/stub DLL, 
//      run nmake -f MagickCOMps.mk in the project directory.

#include "stdafx.h"
#include "resource.h"
#include <initguid.h>
#include "MagickCOM.h"

#include "MagickCOM_i.c"
#include "Image.h"
#include "ImageControl.h"
#include "Geometry.h"
#include "Color.h"
#include "ImageCollection.h"


CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_Image, CImage)
OBJECT_ENTRY(CLSID_ImageControl, CImageControl)
OBJECT_ENTRY(CLSID_Geometry, CGeometry)
OBJECT_ENTRY(CLSID_Color, CColor)
OBJECT_ENTRY(CLSID_ImageCollection, CImageCollection)
END_OBJECT_MAP()

/////////////////////////////////////////////////////////////////////////////
// DLL Entry Point

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID /*lpReserved*/)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        _Module.Init(ObjectMap, hInstance, &LIBID_MAGICKCOMLib);
        DisableThreadLibraryCalls(hInstance);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
        _Module.Term();
    return TRUE;    // ok
}

/////////////////////////////////////////////////////////////////////////////
// Used to determine whether the DLL can be unloaded by OLE

STDAPI DllCanUnloadNow(void)
{
    return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// Returns a class factory to create an object of the requested type

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _Module.GetClassObject(rclsid, riid, ppv);
}

/////////////////////////////////////////////////////////////////////////////
// DllRegisterServer - Adds entries to the system registry

STDAPI DllRegisterServer(void)
{
    // registers object, typelib and all interfaces in typelib
    return _Module.RegisterServer(TRUE);
}

/////////////////////////////////////////////////////////////////////////////
// DllUnregisterServer - Removes entries from the system registry

STDAPI DllUnregisterServer(void)
{
    return _Module.UnregisterServer(TRUE);
}


