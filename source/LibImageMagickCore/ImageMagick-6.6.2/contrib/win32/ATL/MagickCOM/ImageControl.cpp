// This is just a test class to see how I could implement returning 
// a COM object that was instanciated inside my C++ code to the 
// calling code (eg. VFP, VB, etc.). It barely works, but isn't
// maintaining the internal reference correctly (will blow up if 
// called multiple times), and I don't think it's cleaning up
// after itself. 
//
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

// ImageControl.cpp : Implementation of CImageControl
#include "stdafx.h"
#include "MagickCOM.h"
#include "ImageControl.h"
#include "comutil.h"
#include <atlbase.h>
//CComModule _Module;  // extern if declared elsewhere
#include <atlcom.h>
#include "comdef.h"

/////////////////////////////////////////////////////////////////////////////
// CImageControl


STDMETHODIMP CImageControl::get_Image(IImage **pVal)
{
	// Creates an instance of the Image COM object
	// if it doesn't already exist. Then it returns a 
	// reference to it.
    if(idImage == NULL)
	{ 

		// The Image object isn't defined yet
		// Let's create it, then grab the correct interface

        IImage *pImageControl;

		HRESULT	hrImage = 0;		
		
		
		CComPtr<IDispatch> lpDispatch;

		hrImage = lpDispatch.CoCreateInstance(_bstr_t("MagickCOM.Image"));

		// Check to see if anything went wrong
		// If it did, return NULL
		// NOTE: MessageBox just here for debugging.
		if ( FAILED(hrImage))
		{
			*pVal = NULL;
			MessageBox(NULL, "CoCreateInstance() failed :-(", "MagickCOM Error", MB_OK);
			return S_FALSE;
		}
				
		hrImage = lpDispatch->QueryInterface(IID_IImage, (void **)&pImageControl);
		
		// NOTE: MessageBox just here for debugging.
		if ( FAILED(hrImage))
		{
			*pVal = NULL;
			MessageBox(NULL, "QueryInterface() failed. :-(", "MagickCOM Error", MB_OK);
			return S_FALSE;
		}
		
		idImage = pImageControl;
        
	}
    
	// Finally either return a pointer, or a NULL 
	*pVal = idImage;

	return S_OK;
}




/*
HRESULT hr;
IImageControl *pImageControl;
hr = CoCreateInstance(CLSID_Foo, NULL, CLSCTX_ALL,
               IID_IFoo, (void **)&pImageControl);
*/


//
/*
  HRESULT hr = S_OK;

    CComPtr<IBar> pBar;
    hr = pBar.CoCreateInstance(CLSID_Bar);
    if( FAILED(hr) ) return hr;

    hr = pBar->DoSomething();
    if( FAILED(hr) ) return hr;

    CComPtr<IBarEx> pBarEx;
    hr = pBar->QueryInterface(&pBarEx);
    if( FAILED(hr) ) return hr;

    hr = pBarEx->DoSomethingEx();
    if( FAILED(hr) ) return hr;

    return S_OK;
*/


STDMETHODIMP CImageControl::get_GetAddress(long *pVal)
{
	// TODO: Add your implementation code here
	const long uiMem = reinterpret_cast<const long>(this); 
	 

	*pVal = uiMem;
	return S_OK;
}


STDMETHODIMP CImageControl::SampleMessage()
{
	// TODO: Add your implementation code here
    MessageBox(NULL, "Hello, from MagickCOM.ImageControl", "MagickCOM Message", MB_OK);
	return S_OK;
}
