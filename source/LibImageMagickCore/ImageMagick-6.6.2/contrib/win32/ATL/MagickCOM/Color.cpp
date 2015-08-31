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

// Color.cpp : Implementation of CColor
#include "stdafx.h"
#include "MagickCOM.h"
#include "Color.h"
#include "Magick++.h"
#include "string.h"

using namespace std;


/////////////////////////////////////////////////////////////////////////////
// CColor

//Magick::Color oColor;


STDMETHODIMP CColor::get_ColorMode(BSTR *pVal)
{
	// TODO: Add your implementation code here

	*pVal = ColorMode;

	return S_OK;
}

STDMETHODIMP CColor::SetRGB(double Red, double Green, double Blue)
{
	// TODO: Add your implementation code here
	CColor::Red = Red;
	CColor::Green = Green;
	CColor::Blue = Blue;
	ColorMode = "RGB";

	return S_OK;
}

STDMETHODIMP CColor::SetGray(double Shade)
{
	// TODO: Add your implementation code here
	CColor::Shade = Shade;
	ColorMode = "Gray";

	return S_OK;
}

STDMETHODIMP CColor::SetMono(VARIANT_BOOL WhitePixel)
{
	// TODO: Add your implementation code here
	CColor::WhitePixel = WhitePixel;
	ColorMode = "Mono";

	return S_OK;
}

STDMETHODIMP CColor::SetHSL(double Hue, double Saturation, double Luminosity)
{
	// TODO: Add your implementation code here
	CColor::Hue = Hue;
	CColor::Saturation = Saturation;
	CColor::Luminosity = Luminosity;
	ColorMode = "HSL";

	return S_OK;
}

STDMETHODIMP CColor::SetYUV(double Y, double U, double V)
{
	// TODO: Add your implementation code here
	CColor::Y = Y;
	CColor::U = U;
	CColor::V = V;
	ColorMode = "YUV";

	return S_OK;
}

