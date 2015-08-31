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

// ImageControl.h : Declaration of the CImageControl

#ifndef __IMAGECONTROL_H_
#define __IMAGECONTROL_H_

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CImageControl
class ATL_NO_VTABLE CImageControl : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CImageControl, &CLSID_ImageControl>,
	public IDispatchImpl<IImageControl, &IID_IImageControl, &LIBID_MAGICKCOMLib>
{
public:
	CImageControl()
	{
		// Initialize the Image pointer 
		idImage = NULL;
	}

DECLARE_REGISTRY_RESOURCEID(IDR_IMAGECONTROL)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CImageControl)
	COM_INTERFACE_ENTRY(IImageControl)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

// IImageControl
public:
	STDMETHOD(SampleMessage)();
	STDMETHOD(get_GetAddress)(/*[out, retval]*/ long *pVal);
	STDMETHOD(get_Image)(/*[out, retval]*/ IImage* *pVal);

private:
	IImage* idImage;
};

#endif //__IMAGECONTROL_H_
