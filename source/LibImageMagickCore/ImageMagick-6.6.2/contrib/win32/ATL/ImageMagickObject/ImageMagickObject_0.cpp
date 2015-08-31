// ImageMagickObject.cpp : Implementation of DLL Exports.


// Note: Proxy/Stub Information
//      To build a separate proxy/stub DLL, 
//      run nmake -f ImageMagickObjectps.mk in the project directory.

#include "stdafx.h"
#include "resource.h"
#include <initguid.h>
#include "ImageMagickObject.h"

#include "ImageMagickObject_i.c"
#include "MagickImage.h"

CMagickComModule::CMagickComModule() : m_hInstance(NULL)
{
  m_szAppPath[0]='\0';
}

CMagickComModule::~CMagickComModule()
{
}

CMagickComModule _Module;

//CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_MagickImage, CMagickImage)
END_OBJECT_MAP()

/////////////////////////////////////////////////////////////////////////////
// DLL Entry Point

#include <direct.h>

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID /*lpReserved*/)
{
  //_DbgBreak();
  if (dwReason == DLL_PROCESS_ATTACH)
  {
#ifdef _DEBUG
    int tmpDbgFlag;
#endif
    _Module.m_hInstance = hInstance;
    if (!GetModuleFileName (hInstance, _Module.m_szAppPath, MAX_PATH))
      return FALSE;

    _Module.Init(ObjectMap, hInstance, &LIBID_IMAGEMAGICKOBJECTLib);
    DisableThreadLibraryCalls(hInstance);

#ifdef _DEBUG
    tmpDbgFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
    tmpDbgFlag |= _CRTDBG_CHECK_ALWAYS_DF;
    //tmpDbgFlag |= _CRTDBG_DELAY_FREE_MEM_DF;
    tmpDbgFlag |= _CRTDBG_LEAK_CHECK_DF;
    _CrtSetDbgFlag(tmpDbgFlag);
#endif

    ::InitializeMagick(_Module.m_szAppPath);
  }
  else if (dwReason == DLL_PROCESS_DETACH)
  {
    _Module.Term();
    ::DestroyMagick();
  }
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


