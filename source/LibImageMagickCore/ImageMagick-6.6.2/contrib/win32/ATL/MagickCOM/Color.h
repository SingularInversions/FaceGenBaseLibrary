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

// Color.h : Declaration of the CColor

#ifndef __COLOR_H_
#define __COLOR_H_

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CColor
class ATL_NO_VTABLE CColor : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CColor, &CLSID_Color>,
	public IDispatchImpl<IColor, &IID_IColor, &LIBID_MAGICKCOMLib>
{
public:
	CColor()
	{
		// Set-up our defaults
		ColorMode = "RGB";
		Red = 0;
		Green = 0;
		Blue = 0;
		WhitePixel = false;

	}

DECLARE_REGISTRY_RESOURCEID(IDR_COLOR)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CColor)
	COM_INTERFACE_ENTRY(IColor)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

// IColor
public:
	STDMETHOD(SetYUV)(double Y, double U, double V);
	STDMETHOD(SetHSL)(double Hue, double Saturation, double Luminosity);
	STDMETHOD(SetMono)(VARIANT_BOOL WhitePixel);
	STDMETHOD(SetGray)(double Shade);
	STDMETHOD(SetRGB)(double Red, double Green, double Blue);
	STDMETHOD(get_ColorMode)(/*[out, retval]*/ BSTR *pVal);
	CComBSTR ColorMode;
	// RGB
	double Red;
	double Green;
	double Blue;
	// Gray
	double Shade;
    // Mono
	BOOL WhitePixel;
	// HSL
	double Hue;
	double Saturation;
	double Luminosity;
	// YUV
	double Y;
	double U;
	double V;

private:

};

#endif //__COLOR_H_
