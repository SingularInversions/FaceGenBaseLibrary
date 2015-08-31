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

// Geometry.cpp : Implementation of CGeometry
#include "stdafx.h"
#include "MagickCOM.h"
#include "Geometry.h"
#include "Magick++.h"
#include "string.h"

using namespace std;
using namespace Magick;

/////////////////////////////////////////////////////////////////////////////
// CGeometry

Magick::Geometry oGeometry;

STDMETHODIMP CGeometry::get_Width(unsigned int *pVal)
{
	// TODO: Add your implementation code here
    *pVal = oGeometry.width();
	return S_OK;
}

STDMETHODIMP CGeometry::put_Width(unsigned int newVal)
{
	// TODO: Add your implementation code here
    oGeometry.width(newVal);
	return S_OK;
}

STDMETHODIMP CGeometry::get_Height(unsigned int *pVal)
{
	// TODO: Add your implementation code here
    *pVal = oGeometry.height();
	return S_OK;
}

STDMETHODIMP CGeometry::put_Height(unsigned int newVal)
{
	// TODO: Add your implementation code here
    oGeometry.height(newVal);
	return S_OK;
}

STDMETHODIMP CGeometry::get_XOff(unsigned int *pVal)
{
	// TODO: Add your implementation code here
    *pVal = oGeometry.xOff();
	return S_OK;
}

STDMETHODIMP CGeometry::put_XOff(unsigned int newVal)
{
	// TODO: Add your implementation code here
    oGeometry.xOff(newVal);
	return S_OK;
}

STDMETHODIMP CGeometry::get_YOff(unsigned int *pVal)
{
	// TODO: Add your implementation code here
    *pVal = oGeometry.yOff();
	return S_OK;
}

STDMETHODIMP CGeometry::put_YOff(unsigned int newVal)
{
	// TODO: Add your implementation code here
    oGeometry.yOff(newVal);
	return S_OK;
}

STDMETHODIMP CGeometry::get_XNegative(VARIANT_BOOL *pVal)
{
	// TODO: Add your implementation code here
    *pVal = oGeometry.xNegative();
	return S_OK;
}

STDMETHODIMP CGeometry::put_XNegative(VARIANT_BOOL newVal)
{
	// TODO: Add your implementation code here
    oGeometry.xNegative(newVal);
	return S_OK;
}

STDMETHODIMP CGeometry::get_YNegative(VARIANT_BOOL *pVal)
{
	// TODO: Add your implementation code here
    *pVal = oGeometry.yNegative();
	return S_OK;
}

STDMETHODIMP CGeometry::put_YNegative(VARIANT_BOOL newVal)
{
	// TODO: Add your implementation code here
    oGeometry.yNegative(newVal);
	return S_OK;
}

STDMETHODIMP CGeometry::get_Percent(VARIANT_BOOL *pVal)
{
	// TODO: Add your implementation code here
    *pVal = oGeometry.percent();
	return S_OK;
}

STDMETHODIMP CGeometry::put_Percent(VARIANT_BOOL newVal)
{
	// TODO: Add your implementation code here
    oGeometry.percent(newVal);
	return S_OK;
}

STDMETHODIMP CGeometry::get_Aspect(VARIANT_BOOL *pVal)
{
	// TODO: Add your implementation code here
    *pVal = oGeometry.aspect();
	return S_OK;
}

STDMETHODIMP CGeometry::put_Aspect(VARIANT_BOOL newVal)
{
	// TODO: Add your implementation code here
    oGeometry.aspect(newVal);
	return S_OK;
}

STDMETHODIMP CGeometry::get_Greater(VARIANT_BOOL *pVal)
{
	// TODO: Add your implementation code here
    *pVal = oGeometry.greater();
	return S_OK;
}

STDMETHODIMP CGeometry::put_Greater(VARIANT_BOOL newVal)
{
	// TODO: Add your implementation code here
    oGeometry.greater(newVal);
	return S_OK;
}

STDMETHODIMP CGeometry::get_Less(VARIANT_BOOL *pVal)
{
	// TODO: Add your implementation code here
    *pVal = oGeometry.less();
	return S_OK;
}

STDMETHODIMP CGeometry::put_Less(VARIANT_BOOL newVal)
{
	// TODO: Add your implementation code here
	oGeometry.less(newVal);
	return S_OK;
}

STDMETHODIMP CGeometry::IsValid(VARIANT_BOOL *pVal)
{
	// TODO: Add your implementation code here
    *pVal = oGeometry.isValid();
	return S_OK;
}

STDMETHODIMP CGeometry::Geometry(BSTR PSSizeNick, BSTR *pVal)
{
	// TODO: Add your implementation code here
	
	USES_CONVERSION;

	std::string strSizeNick = PSSizeNick ? OLE2A(PSSizeNick) : "";	
		
	if (strSizeNick.length() > 0)
	{
	    // This code handles the case where the user is passing
		// in a Postscript Page Size Nickname (eg. "Legal", "Letter", etc.)
		// If they pass in an empty string, the class builds the geometry
		// string based on the property settings.
		oGeometry = strSizeNick;
	} 
	
	std::string strGeo = oGeometry;	

    CComBSTR cstrGeo = A2OLE(strGeo.c_str());
	*pVal = cstrGeo; 
	return S_OK;
}

		
