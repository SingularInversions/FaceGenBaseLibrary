// stdafx.h : include file for standard system include files,
//      or project specific include files that are used frequently,
//      but are changed infrequently

#if !defined(AFX_STDAFX_H__90A1621B_3F1D_4480_A1E4_948DCDE9EDC1__INCLUDED_)
#define AFX_STDAFX_H__90A1621B_3F1D_4480_A1E4_948DCDE9EDC1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#define _ATL_APARTMENT_THREADED

#include <atlbase.h>
//You may derive a class from CComModule and use it if you want to override
//something, but do not change the name of _Module
class CMagickComModule : public CComModule
{
public:
  CMagickComModule();
  virtual ~CMagickComModule();
  
  HINSTANCE m_hInstance;
  // Global path to this DLL
  TCHAR m_szAppPath[MAX_PATH];
};
extern CMagickComModule _Module;
#include <atlcom.h>
#include <comdef.h>		// for _variant_t and _bstr_t

#define _DbgBreak() __asm { int 3 }

//extern CComModule _Module;
//#include <atlcom.h>

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__90A1621B_3F1D_4480_A1E4_948DCDE9EDC1__INCLUDED)
