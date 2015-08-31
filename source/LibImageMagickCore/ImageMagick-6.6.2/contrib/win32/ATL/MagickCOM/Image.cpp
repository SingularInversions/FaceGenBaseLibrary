// Basic ATL/COM wrapper for the Image object using the ImageMagick++ classes.
// There isn't much in the way of comments in here, since the code is pretty
// cookie-cutter. Lots of "TODO" comments that should be removed or replaced
// with something a little more descriptive.
//
// The Composite() method shows one way to handle multiple image objects.
// As the comments point out, this is a COM hack. It's no reallu a valid way
// to use COM, even though it works. I haven't been able to come up with
// a "legal" way to do this. 
//
// TODO: Methods that require a Color object haven't been implemented yet.
//       The basic idea was for the user to instanciate a MagickCOM.Color
//       object, set it's properties, then pass this object to the Image 
//       method(s). Those methods would then reconstruct a C++ version of
//       the class which finally gets used.
//
//       A lot of the overloaded methods haven't been implemented either. 
//       I picked the most common ones to implement first. The overloaded
//       versions will require a new method name for them to work in COM. 
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

// Image.cpp : Implementation of CImage
#include "stdafx.h"
#include "MagickCOM.h"
#include "Image.h"
#include "Magick++.h"
#include "ShellAPI.h"
#include "string.h"
#include "ImageControl.h"

using namespace std;
using namespace Magick;

//CComBSTR ErrorMsg;
//unsigned int ImageWidth = 70;
//unsigned int ImageHeight = 70;
//unsigned int Columns;
//unsigned int Rows;

//Magick::Image oImage;


/////////////////////////////////////////////////////////////////////////////
// CImage

STDMETHODIMP CImage::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IImage,
	};
	for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}


STDMETHODIMP CImage::OnStartPage (IUnknown* pUnk)  
{
	if(!pUnk)
		return E_POINTER;

	CComPtr<IScriptingContext> spContext;
	HRESULT hr;

	// Get the IScriptingContext Interface
	hr = pUnk->QueryInterface(IID_IScriptingContext, (void **)&spContext);
	if(FAILED(hr))
		return hr;

	// Get Request Object Pointer
	hr = spContext->get_Request(&m_piRequest);
	if(FAILED(hr))
	{
		spContext.Release();
		return hr;
	}

	// Get Response Object Pointer
	hr = spContext->get_Response(&m_piResponse);
	if(FAILED(hr))
	{
		m_piRequest.Release();
		return hr;
	}
	
	// Get Server Object Pointer
	hr = spContext->get_Server(&m_piServer);
	if(FAILED(hr))
	{
		m_piRequest.Release();
		m_piResponse.Release();
		return hr;
	}
	
	// Get Session Object Pointer
	hr = spContext->get_Session(&m_piSession);
	if(FAILED(hr))
	{
		m_piRequest.Release();
		m_piResponse.Release();
		m_piServer.Release();
		return hr;
	}

	// Get Application Object Pointer
	hr = spContext->get_Application(&m_piApplication);
	if(FAILED(hr))
	{
		m_piRequest.Release();
		m_piResponse.Release();
		m_piServer.Release();
		m_piSession.Release();
		return hr;
	}
	m_bOnStartPageCalled = TRUE;
	return S_OK;
}

STDMETHODIMP CImage::OnEndPage ()  
{
	m_bOnStartPageCalled = FALSE;
	// Release all interfaces
	m_piRequest.Release();
	m_piResponse.Release();
	m_piServer.Release();
	m_piSession.Release();
	m_piApplication.Release();

	return S_OK;
}

/*
STDMETHODIMP CImage::SetErrorMessage(string ErrorG);
{
		//string ErrorG = error_.what();		

//      Can't use A2OLE in an Exception....
//		ErrorMsg = A2OLE(ErrorG.c_str()) ;
		
        int nStrSize = MultiByteToWideChar(CP_ACP, MB_COMPOSITE, ErrorG.c_str(), -1, NULL, 0);

        WCHAR *wzStringBuf = new WCHAR[nStrSize + 1];
		
        int nRetVal = MultiByteToWideChar(CP_ACP, MB_COMPOSITE, ErrorG.c_str(), -1, wzStringBuf, nStrSize);
        
        ErrorMsg = wzStringBuf;		
}
*/

STDMETHODIMP CImage::Read(BSTR cFilename, VARIANT_BOOL *pVal)
{
   
//	Image oImage;
    USES_CONVERSION;

	try {	    
    
  	    oImage->read(cFilename ? OLE2A(cFilename) : "");
        *pVal = true;  		
		ErrorMsg.Empty();
	}
	catch( Exception &error_ )
	{		
		
		*pVal = false; //error_.what()		
                
//      Can't use A2OLE in an Exception....

		string ErrorG = error_.what();		

//		ErrorMsg = A2OLE(ErrorG.c_str()) ;
		
		CImage::SetErrorMessage(ErrorG.c_str());
		/*--------------------------
        int nStrSize = MultiByteToWideChar(CP_ACP, MB_COMPOSITE, ErrorG.c_str(), -1, NULL, 0);

        WCHAR *wzStringBuf = new WCHAR[nStrSize + 1];
		
        int nRetVal = MultiByteToWideChar(CP_ACP, MB_COMPOSITE, ErrorG.c_str(), -1, wzStringBuf, nStrSize);
        
        ErrorMsg = wzStringBuf;		
		----------------------------*/

		return S_FALSE;
	}

	return S_OK;
}


STDMETHODIMP CImage::Resize(BSTR cFilename, BSTR cOutput, VARIANT_BOOL *pVal)
{
	// TODO: Add your implementation code here
	
    //Image oImage;
        
	try {
		// Read a file
		USES_CONVERSION;
		oImage->read(cFilename ? OLE2A(cFilename) : "");
		oImage->sample(Magick::Geometry(ImageWidth, ImageHeight));

		//Write a file
		oImage->write(cOutput ? OLE2A(cOutput) : "");
		ErrorMsg.Empty();
		*pVal = true;
	}
	catch( Exception &error_ )
	{
//		cout << error_.what() << endl; 
//		iReturnCode = 1;
	//	CImage::SetErrorMessage(error_.what());
/*		
		USES_CONVERSION;
		string strError;
		string strErrorWhat = error_.what();
		strError = "An error occurred during resize: " + strErrorWhat;
		MessageBox(NULL, strError.c_str(), "MagickCOM Error", MB_OK);
        string ErrorG = error_.what();
		ErrorMsg = A2OLE(ErrorG.c_str()) ;
*/
		string ErrorG = error_.what();				

		CImage::SetErrorMessage(ErrorG.c_str());
		/*------------------------------
        int nStrSize = MultiByteToWideChar(CP_ACP, MB_COMPOSITE, ErrorG.c_str(), -1, NULL, 0);

        WCHAR *wzStringBuf = new WCHAR[nStrSize + 1];
		
        int nRetVal = MultiByteToWideChar(CP_ACP, MB_COMPOSITE, ErrorG.c_str(), -1, wzStringBuf, nStrSize);
        
        ErrorMsg = wzStringBuf;		
        --------------------------------*/
        *pVal = false;

		return S_FALSE;
	}
	return S_OK;
}

STDMETHODIMP CImage::get_ImageWidth(unsigned int *pVal)
{
	// TODO: Add your implementation code here
	// *pVal = oImage.columns() ;     // ImageWidth;
	*pVal = ImageWidth ; 
	return S_OK;
}

STDMETHODIMP CImage::put_ImageWidth(unsigned int newVal)
{
	// TODO: Add your implementation code here
    ImageWidth = newVal;
	return S_OK;
}

STDMETHODIMP CImage::get_ImageHeight(unsigned int *pVal)
{
	// TODO: Add your implementation code here

    //*pVal = oImage.rows(); //ImageHeight;
	*pVal = ImageHeight;
	return S_OK;
}

STDMETHODIMP CImage::put_ImageHeight(unsigned int newVal)
{
	// TODO: Add your implementation code here
    ImageHeight = newVal;
	return S_OK;
}


STDMETHODIMP CImage::get_ErrorMsg(BSTR *pVal)
{
	// TODO: Add your implementation code here
    *pVal = ErrorMsg;
	return S_OK;
}

STDMETHODIMP CImage::put_ErrorMsg(BSTR newVal)
{
	// TODO: Add your implementation code here
    ErrorMsg = newVal;
	return S_OK;
}

STDMETHODIMP CImage::InitMagick(BSTR cPath)
{
	// TODO: Add your implementation code here
    USES_CONVERSION;
    InitializeMagick(cPath ? OLE2A(cPath) : "");	
	return S_OK;
}

STDMETHODIMP CImage::get_Columns(unsigned int *pVal)
{
	// TODO: Add your implementation code here
    *pVal = oImage->columns();
	return S_OK;
}


STDMETHODIMP CImage::get_Rows(unsigned int *pVal)
{
	// TODO: Add your implementation code here
    *pVal = oImage->rows();
	return S_OK;
}


void CImage::SetErrorMessage(const char *strErrorG)
{

//      Can't use A2OLE in an Exception....
//		ErrorMsg = A2OLE(ErrorG.c_str()) ;
		
        int nStrSize = MultiByteToWideChar(CP_ACP, MB_COMPOSITE, strErrorG, -1, NULL, 0);

        WCHAR *wzStringBuf = new WCHAR[nStrSize + 1];
		
        int nRetVal = MultiByteToWideChar(CP_ACP, MB_COMPOSITE, strErrorG, -1, wzStringBuf, nStrSize);
        
        ErrorMsg = wzStringBuf;		
}

STDMETHODIMP CImage::Scale(unsigned int x, unsigned int y, VARIANT_BOOL *pVal)
{
	// TODO: Add your implementation code here
    ErrorMsg.Empty();

    try
	{
		oImage->scale(Magick::Geometry(x, y));
	   *pVal = true;
	}
	catch( Exception &error_ )
	{
        string ErrorG = error_.what();		
	
		CImage::SetErrorMessage(ErrorG.c_str());

	   *pVal = false;

	}

	return S_OK;
}

STDMETHODIMP CImage::Sample(unsigned int x, unsigned int y, VARIANT_BOOL *pVal)
{
	// TODO: Add your implementation code here
    ErrorMsg.Empty();

   try
	{
       oImage->sample(Magick::Geometry(x, y));
	   *pVal = true;
	}
	catch( Exception &error_ )
	{
        string ErrorG = error_.what();		
	
		CImage::SetErrorMessage(ErrorG.c_str());

	   *pVal = false;
	}

	return S_OK;
}

STDMETHODIMP CImage::Write(BSTR cFilename, VARIANT_BOOL *pVal)
{
	// TODO: Add your implementation code here
    ErrorMsg.Empty();

    try {
		// Read a file
		USES_CONVERSION;
		oImage->write(cFilename ? OLE2A(cFilename) : "");
		ErrorMsg.Empty();
		*pVal = true;
	}
	catch( Exception &error_ )
	{
		string ErrorG = error_.what();				

		CImage::SetErrorMessage(ErrorG.c_str());
		*pVal = false;
	}

	return S_OK;
}

STDMETHODIMP CImage::Rotate(double Degrees, VARIANT_BOOL *pVal)
{
	// TODO: Add your implementation code here
    ErrorMsg.Empty();

    try {
		oImage->rotate(Degrees);
		*pVal = true;
	}
	catch( Exception &error_ )
	{
		string ErrorG = error_.what();

		CImage::SetErrorMessage(ErrorG.c_str());
		*pVal = false;
	}

	return S_OK;
}

STDMETHODIMP CImage::Flip(VARIANT_BOOL *pVal)
{
	// TODO: Add your implementation code here
	ErrorMsg.Empty();

    try {
		oImage->flip();
		*pVal = true;
	}
	catch( Exception &error_ )
	{
		string ErrorG = error_.what();

		CImage::SetErrorMessage(ErrorG.c_str());
		*pVal = false;
	}

	return S_OK;
}

STDMETHODIMP CImage::Enhance(VARIANT_BOOL *pVal)
{
	// TODO: Add your implementation code here

	ErrorMsg.Empty();

    try {
		oImage->enhance();
		*pVal = true;
	}
	catch( Exception &error_ )
	{
		string ErrorG = error_.what();

		CImage::SetErrorMessage(ErrorG.c_str());
		*pVal = false;
	}


	return S_OK;
}

STDMETHODIMP CImage::Equalize(VARIANT_BOOL *pVal)
{
	// TODO: Add your implementation code here
	ErrorMsg.Empty();

    try {
		oImage->equalize();
		*pVal = true;
	}
	catch( Exception &error_ )
	{
		string ErrorG = error_.what();

		CImage::SetErrorMessage(ErrorG.c_str());
		*pVal = false;
	}

	return S_OK;
}

STDMETHODIMP CImage::Flop(VARIANT_BOOL *pVal)
{
	// TODO: Add your implementation code here
	ErrorMsg.Empty();

    try {
		oImage->flop();
		*pVal = true;
	}
	catch( Exception &error_ )
	{
		string ErrorG = error_.what();

		CImage::SetErrorMessage(ErrorG.c_str());
		*pVal = false;
	}

	return S_OK;
}

STDMETHODIMP CImage::Normalize(VARIANT_BOOL *pVal)
{
	// TODO: Add your implementation code here
	ErrorMsg.Empty();

    try {
		oImage->normalize();
		*pVal = true;
	}
	catch( Exception &error_ )
	{
		string ErrorG = error_.what();

		CImage::SetErrorMessage(ErrorG.c_str());
		*pVal = false;
	}

	return S_OK;
}

STDMETHODIMP CImage::Despeckle(VARIANT_BOOL *pVal)
{
	// TODO: Add your implementation code here
	ErrorMsg.Empty();

    try {
		oImage->despeckle();
		*pVal = true;
	}
	catch( Exception &error_ )
	{
		string ErrorG = error_.what();

		CImage::SetErrorMessage(ErrorG.c_str());
		*pVal = false;
	}

	return S_OK;
}

STDMETHODIMP CImage::Contrast(unsigned int sharpen, VARIANT_BOOL *pVal)
{
	// TODO: Add your implementation code here
	ErrorMsg.Empty();

    try {
		oImage->contrast(sharpen);
		*pVal = true;
	}
	catch( Exception &error_ )
	{
		string ErrorG = error_.what();

		CImage::SetErrorMessage(ErrorG.c_str());
		*pVal = false;
	}
	return S_OK;
}

STDMETHODIMP CImage::Edge(unsigned int radius, VARIANT_BOOL *pVal)
{
	// TODO: Add your implementation code here
	ErrorMsg.Empty();

    try {
		oImage->edge(radius);
		*pVal = true;
	}
	catch( Exception &error_ )
	{
		string ErrorG = error_.what();

		CImage::SetErrorMessage(ErrorG.c_str());
		*pVal = false;
	}

	return S_OK;
}

STDMETHODIMP CImage::Emboss(double radius, double sigma, VARIANT_BOOL *pVal)
{
	// TODO: Add your implementation code here
	ErrorMsg.Empty();

    try {
		oImage->emboss(radius, sigma);
		*pVal = true;
	}
	catch( Exception &error_ )
	{
		string ErrorG = error_.what();

		CImage::SetErrorMessage(ErrorG.c_str());
		*pVal = false;
	}

	return S_OK;
}

STDMETHODIMP CImage::Gamma(double Red, double Green, double Blue, VARIANT_BOOL *pVal)
{
	// TODO: Add your implementation code here
	ErrorMsg.Empty();

    try {
		oImage->gamma(Red, Green, Blue);
		*pVal = true;
	}
	catch( Exception &error_ )
	{
		string ErrorG = error_.what();

		CImage::SetErrorMessage(ErrorG.c_str());
		*pVal = false;
	}

	return S_OK;
}

STDMETHODIMP CImage::GaussianBlur(double width, double sigma, VARIANT_BOOL *pVal)
{
	// TODO: Add your implementation code here
	ErrorMsg.Empty();

    try {
		oImage->gaussianBlur(width, sigma);
		*pVal = true;
	}
	catch( Exception &error_ )
	{
		string ErrorG = error_.what();

		CImage::SetErrorMessage(ErrorG.c_str());
		*pVal = false;
	}

	return S_OK;
}

STDMETHODIMP CImage::Implode(double factor, VARIANT_BOOL *pVal)
{
	// TODO: Add your implementation code here
	ErrorMsg.Empty();

    try {
		oImage->implode(factor);
		*pVal = true;
	}
	catch( Exception &error_ )
	{
		string ErrorG = error_.what();

		CImage::SetErrorMessage(ErrorG.c_str());
		*pVal = false;
	}

	return S_OK;
}

STDMETHODIMP CImage::MedianFilter(double radius, VARIANT_BOOL *pVal)
{
	// TODO: Add your implementation code here
	ErrorMsg.Empty();

    try {
		oImage->medianFilter(radius);
		*pVal = true;
	}
	catch( Exception &error_ )
	{
		string ErrorG = error_.what();

		CImage::SetErrorMessage(ErrorG.c_str());
		*pVal = false;
	}

	return S_OK;
}

STDMETHODIMP CImage::Minify(VARIANT_BOOL *pVal)
{
	// TODO: Add your implementation code here
	ErrorMsg.Empty();

    try {
		oImage->minify();
		*pVal = true;
	}
	catch( Exception &error_ )
	{
		string ErrorG = error_.what();

		CImage::SetErrorMessage(ErrorG.c_str());
		*pVal = false;
	}

	return S_OK;
}

STDMETHODIMP CImage::Modulate(double brightness, double saturation, double hue, VARIANT_BOOL *pVal)
{
	// TODO: Add your implementation code here
	ErrorMsg.Empty();

    try {
		oImage->modulate(brightness, saturation, hue);
		*pVal = true;
	}
	catch( Exception &error_ )
	{
		string ErrorG = error_.what();

		CImage::SetErrorMessage(ErrorG.c_str());
		*pVal = false;
	}

	return S_OK;
}

STDMETHODIMP CImage::Negate(VARIANT_BOOL grayscale, VARIANT_BOOL *pVal)
{
	// TODO: Add your implementation code here
	ErrorMsg.Empty();

    try {
		oImage->negate(grayscale);
		*pVal = true;
	}
	catch( Exception &error_ )
	{
		string ErrorG = error_.what();

		CImage::SetErrorMessage(ErrorG.c_str());
		*pVal = false;
	}

	return S_OK;
}

STDMETHODIMP CImage::OilPaint(unsigned int radius, VARIANT_BOOL *pVal)
{
	// TODO: Add your implementation code here
	ErrorMsg.Empty();

    try {
		oImage->oilPaint(radius);
		*pVal = true;
	}
	catch( Exception &error_ )
	{
		string ErrorG = error_.what();

		CImage::SetErrorMessage(ErrorG.c_str());
		*pVal = false;
	}

	return S_OK;
}

STDMETHODIMP CImage::Opacity(unsigned int opacity, VARIANT_BOOL *pVal)
{
	// TODO: Add your implementation code here
	ErrorMsg.Empty();

    try {
		oImage->opacity(opacity);
		*pVal = true;
	}
	catch( Exception &error_ )
	{
		string ErrorG = error_.what();

		CImage::SetErrorMessage(ErrorG.c_str());
		*pVal = false;
	}

	return S_OK;
}

STDMETHODIMP CImage::Roll(int columns, int rows, VARIANT_BOOL *pVal)
{
	// TODO: Add your implementation code here
	ErrorMsg.Empty();

    try {
		oImage->roll(columns, rows);
		*pVal = true;
	}
	catch( Exception &error_ )
	{
		string ErrorG = error_.what();

		CImage::SetErrorMessage(ErrorG.c_str());
		*pVal = false;
	}

	return S_OK;
}

STDMETHODIMP CImage::Solarize(double factor, VARIANT_BOOL *pVal)
{
	// TODO: Add your implementation code here
	ErrorMsg.Empty();

    try {
		oImage->solarize(factor);
		*pVal = true;
	}
	catch( Exception &error_ )
	{
		string ErrorG = error_.what();

		CImage::SetErrorMessage(ErrorG.c_str());
		*pVal = false;
	}

	return S_OK;
}

STDMETHODIMP CImage::Spread(unsigned int amount, VARIANT_BOOL *pVal)
{
	// TODO: Add your implementation code here
	ErrorMsg.Empty();

    try {
		oImage->spread(amount);
		*pVal = true;
	}
	catch( Exception &error_ )
	{
		string ErrorG = error_.what();

		CImage::SetErrorMessage(ErrorG.c_str());
		*pVal = false;
	}

	return S_OK;
}

STDMETHODIMP CImage::Sharpen(double radius, double sigma, VARIANT_BOOL *pVal)
{
	// TODO: Add your implementation code here
	ErrorMsg.Empty();

    try {
		oImage->sharpen(radius, sigma);
		*pVal = true;
	}
	catch( Exception &error_ )
	{
		string ErrorG = error_.what();

		CImage::SetErrorMessage(ErrorG.c_str());
		*pVal = false;
	}

	return S_OK;
}

STDMETHODIMP CImage::Shade(double azimuth, double elevation, VARIANT_BOOL colorShading, VARIANT_BOOL *pVal)
{
	// TODO: Add your implementation code here
	ErrorMsg.Empty();

    try {
		oImage->shade(azimuth, elevation, colorShading);
		*pVal = true;
	}
	catch( Exception &error_ )
	{
		string ErrorG = error_.what();

		CImage::SetErrorMessage(ErrorG.c_str());
		*pVal = false;
	}

	return S_OK;
}

STDMETHODIMP CImage::Shear(double xShearAngle, double yShearAngle, VARIANT_BOOL *pVal)
{
	// TODO: Add your implementation code here
	ErrorMsg.Empty();

    try {
		oImage->shear(xShearAngle, yShearAngle);
		*pVal = true;
	}
	catch( Exception &error_ )
	{
		string ErrorG = error_.what();

		CImage::SetErrorMessage(ErrorG.c_str());
		*pVal = false;
	}

	return S_OK;
}

STDMETHODIMP CImage::Swirl(double degrees, VARIANT_BOOL *pVal)
{
	// TODO: Add your implementation code here
	ErrorMsg.Empty();

    try {
		oImage->swirl(degrees);
		*pVal = true;
	}
	catch( Exception &error_ )
	{
		string ErrorG = error_.what();

		CImage::SetErrorMessage(ErrorG.c_str());
		*pVal = false;
	}

	return S_OK;
}

STDMETHODIMP CImage::Wave(double amplitude, double wavelength, VARIANT_BOOL *pVal)
{
	// TODO: Add your implementation code here
	ErrorMsg.Empty();

    try {
		oImage->wave(amplitude, wavelength);
		*pVal = true;
	}
	catch( Exception &error_ )
	{
		string ErrorG = error_.what();

		CImage::SetErrorMessage(ErrorG.c_str());
		*pVal = false;
	}

	return S_OK;
}

STDMETHODIMP CImage::UnsharpMask(double radius, double sigma, double amount, double threshold, VARIANT_BOOL *pVal)
{
	// TODO: Add your implementation code here
	ErrorMsg.Empty();

    try {
		oImage->unsharpmask(radius, sigma, amount, threshold);
		*pVal = true;
	}
	catch( Exception &error_ )
	{
		string ErrorG = error_.what();

		CImage::SetErrorMessage(ErrorG.c_str());
		*pVal = false;
	}

	return S_OK;
}

STDMETHODIMP CImage::Trim(VARIANT_BOOL *pVal)
{
	// TODO: Add your implementation code here
	ErrorMsg.Empty();

    try {
		oImage->trim();
		*pVal = true;
	}
	catch( Exception &error_ )
	{
		string ErrorG = error_.what();

		CImage::SetErrorMessage(ErrorG.c_str());
		*pVal = false;
	}

	return S_OK;
}

STDMETHODIMP CImage::Blur(double radius, double sigma, VARIANT_BOOL *pVal)
{
	// TODO: Add your implementation code here
	ErrorMsg.Empty();

    try {
		oImage->blur(radius, sigma);
		*pVal = true;
	}
	catch( Exception &error_ )
	{
		string ErrorG = error_.what();

		CImage::SetErrorMessage(ErrorG.c_str());
		*pVal = false;
	}

	return S_OK;
}

STDMETHODIMP CImage::Charcoal(double radius, double sigma, VARIANT_BOOL *pVal)
{
	// TODO: Add your implementation code here
	ErrorMsg.Empty();

    try {
		oImage->charcoal(radius, sigma);
		*pVal = true;
	}
	catch( Exception &error_ )
	{
		string ErrorG = error_.what();

		CImage::SetErrorMessage(ErrorG.c_str());
		*pVal = false;
	}

	return S_OK;
}

STDMETHODIMP CImage::Erase(VARIANT_BOOL *pVal)
{
	// TODO: Add your implementation code here
	ErrorMsg.Empty();

    try {
		oImage->erase();
		*pVal = true;
	}
	catch( Exception &error_ )
	{
		string ErrorG = error_.what();

		CImage::SetErrorMessage(ErrorG.c_str());
		*pVal = false;
	}

	return S_OK;
}

STDMETHODIMP CImage::Magnify(VARIANT_BOOL *pVal)
{
	// TODO: Add your implementation code here
	ErrorMsg.Empty();

    try {
		oImage->magnify();
		*pVal = true;
	}
	catch( Exception &error_ )
	{
		string ErrorG = error_.what();

		CImage::SetErrorMessage(ErrorG.c_str());
		*pVal = false;
	}

	return S_OK;
}

STDMETHODIMP CImage::Quantize(VARIANT_BOOL measureError, VARIANT_BOOL *pVal)
{
	// TODO: Add your implementation code here
	ErrorMsg.Empty();

    try {
		oImage->quantize();
		*pVal = true;
	}
	catch( Exception &error_ )
	{
		string ErrorG = error_.what();

		CImage::SetErrorMessage(ErrorG.c_str());
		*pVal = false;
	}

	return S_OK;
}

STDMETHODIMP CImage::Threshold(double threshold, VARIANT_BOOL *pVal)
{
	// TODO: Add your implementation code here
	ErrorMsg.Empty();

    try {
		oImage->threshold(threshold);
		*pVal = true;
	}
	catch( Exception &error_ )
	{
		string ErrorG = error_.what();

		CImage::SetErrorMessage(ErrorG.c_str());
		*pVal = false;
	}

	return S_OK;
}

STDMETHODIMP CImage::Crop(BSTR geometry, VARIANT_BOOL *pVal)
{
	// TODO: Add your implementation code here
    USES_CONVERSION;	

    try {
		oImage->crop(geometry ? OLE2A(geometry) : "");
		*pVal = true;
	}
	catch( Exception &error_ )
	{
		string ErrorG = error_.what();

		CImage::SetErrorMessage(ErrorG.c_str());
		*pVal = false;
	}

	return S_OK;
}


STDMETHODIMP CImage::get_Format(BSTR *pVal)
{
	// TODO: Add your implementation code here

	USES_CONVERSION;
    
	string strFormat = oImage->format();

	CComBSTR cstrFormat = A2OLE(strFormat.c_str());	
	*pVal = cstrFormat; 	

	return S_OK;
}

STDMETHODIMP CImage::get_meanErrorPerPixel(double *pVal)
{
	// TODO: Add your implementation code here
    *pVal = oImage->meanErrorPerPixel();
	return S_OK;
}


STDMETHODIMP CImage::get_Quality(unsigned int *pVal)
{
	// TODO: Add your implementation code here
    *pVal = oImage->quality();
	return S_OK;
}

STDMETHODIMP CImage::put_Quality(unsigned int Quality)
{
	// TODO: Add your implementation code here
    oImage->quality(Quality);
	return S_OK;
}

STDMETHODIMP CImage::get_Depth(unsigned int *pVal)
{
	// TODO: Add your implementation code here
    *pVal = oImage->depth();
	return S_OK;
}

STDMETHODIMP CImage::put_Depth(unsigned int newVal)
{
	// TODO: Add your implementation code here
    oImage->depth(newVal);
	return S_OK;
}

STDMETHODIMP CImage::get_FileSize(long *pVal)
{
	// TODO: Add your implementation code here
    *pVal = oImage->fileSize();
	return S_OK;
}

STDMETHODIMP CImage::get_Adjoin(VARIANT_BOOL *pVal)
{
	// TODO: Add your implementation code here
    *pVal = oImage->adjoin();
	return S_OK;
}

STDMETHODIMP CImage::put_Adjoin(VARIANT_BOOL newVal)
{
	// TODO: Add your implementation code here
    oImage->adjoin(newVal);
	return S_OK;
}

STDMETHODIMP CImage::get_AntiAlias(VARIANT_BOOL *pVal)
{
	// TODO: Add your implementation code here
    *pVal = oImage->antiAlias();
	return S_OK;
}

STDMETHODIMP CImage::put_AntiAlias(VARIANT_BOOL newVal)
{
	// TODO: Add your implementation code here
    oImage->antiAlias(newVal);
	return S_OK;
}

STDMETHODIMP CImage::get_AnimationDelay(unsigned int *pVal)
{
	// TODO: Add your implementation code here
    *pVal = oImage->animationDelay();
	return S_OK;
}

STDMETHODIMP CImage::put_AnimationDelay(unsigned int newVal)
{
	// TODO: Add your implementation code here
    oImage->animationDelay(newVal);
	return S_OK;
}

STDMETHODIMP CImage::get_AnimationIterations(unsigned int *pVal)
{
	// TODO: Add your implementation code here
    *pVal = oImage->animationIterations();
	return S_OK;
}

STDMETHODIMP CImage::put_AnimationIterations(unsigned int newVal)
{
	// TODO: Add your implementation code here
    oImage->animationIterations(newVal);
	return S_OK;
}

STDMETHODIMP CImage::get_BackgroundTexture(BSTR *pVal)
{
	// TODO: Add your implementation code here
	USES_CONVERSION;
    
	string strFormat = oImage->backgroundTexture();

	CComBSTR cstrFormat = A2OLE(strFormat.c_str());	
	*pVal = cstrFormat; 	

	return S_OK;
}

STDMETHODIMP CImage::put_BackgroundTexture(BSTR newVal)
{
	// TODO: Add your implementation code here
	USES_CONVERSION;
	oImage->backgroundTexture(newVal ? OLE2A(newVal) : "");
	return S_OK;
}

STDMETHODIMP CImage::get_BaseColumns(unsigned int *pVal)
{
	// TODO: Add your implementation code here
    *pVal = oImage->baseColumns();
	return S_OK;
}

STDMETHODIMP CImage::get_BaseRows(unsigned int *pVal)
{
	// TODO: Add your implementation code here
	*pVal = oImage->baseRows();
	return S_OK;
}

STDMETHODIMP CImage::get_ColorFuzz(double *pVal)
{
	// TODO: Add your implementation code here
	*pVal = oImage->colorFuzz();
	return S_OK;
}

STDMETHODIMP CImage::put_ColorFuzz(double newVal)
{
	// TODO: Add your implementation code here
    oImage->colorFuzz(newVal);
	return S_OK;
}

STDMETHODIMP CImage::get_Density(BSTR *pVal)
{
	// TODO: Add your implementation code here
	USES_CONVERSION;	
	string strDensity = oImage->density();
    CComBSTR cstrDensity = A2OLE(strDensity.c_str());
	*pVal = cstrDensity;
	return S_OK;
}

STDMETHODIMP CImage::put_Density(BSTR newVal)
{
	// TODO: Add your implementation code here
    USES_CONVERSION;	

    oImage->density(newVal ? OLE2A(newVal) : "");
	return S_OK;
}


STDMETHODIMP CImage::get_GIFDisposeMethod(unsigned int *pVal)
{
	// TODO: Add your implementation code here
    *pVal = oImage->gifDisposeMethod();
	return S_OK;
}

STDMETHODIMP CImage::put_GIFDisposeMethod(unsigned int newVal)
{
	// TODO: Add your implementation code here
	// Probably should check to see if it's 0-4
    oImage->gifDisposeMethod(newVal);
	return S_OK;
}

STDMETHODIMP CImage::get_Magick(BSTR *pVal)
{
	// TODO: Add your implementation code here
	USES_CONVERSION;	
	string strMagick = oImage->magick();
    CComBSTR cstrMagick = A2OLE(strMagick.c_str());
	*pVal = cstrMagick; 
	return S_OK;
}

STDMETHODIMP CImage::put_Magick(BSTR newVal)
{
	// TODO: Add your implementation code here
    USES_CONVERSION;	

    oImage->magick(newVal ? OLE2A(newVal) : "");
	return S_OK;
}

STDMETHODIMP CImage::get_Matte(VARIANT_BOOL *pVal)
{
	// TODO: Add your implementation code here
	*pVal = oImage->matte();
	return S_OK;
}

STDMETHODIMP CImage::put_Matte(VARIANT_BOOL newVal)
{
	// TODO: Add your implementation code here
    oImage->matte(newVal);
	return S_OK;
}

STDMETHODIMP CImage::get_Monochrome(VARIANT_BOOL *pVal)
{
	// TODO: Add your implementation code here
    *pVal = oImage->monochrome();
	return S_OK;
}

STDMETHODIMP CImage::put_Monochrome(VARIANT_BOOL newVal)
{
	// TODO: Add your implementation code here
    oImage->monochrome(newVal);
	return S_OK;
}

STDMETHODIMP CImage::get_NormalizedMaxError(double *pVal)
{
	// TODO: Add your implementation code here
    *pVal = oImage->normalizedMaxError();
	return S_OK;
}

STDMETHODIMP CImage::get_NormalizedMeanError(double *pVal)
{
	// TODO: Add your implementation code here
    *pVal = oImage->normalizedMeanError();
	return S_OK;
}

STDMETHODIMP CImage::get_QuantizedColors(unsigned int *pVal)
{
	// TODO: Add your implementation code here
    *pVal = oImage->quantizeColors();
	return S_OK;
}

STDMETHODIMP CImage::put_QuantizedColors(unsigned int newVal)
{
	// TODO: Add your implementation code here
    oImage->quantizeColors(newVal);
	return S_OK;
}

STDMETHODIMP CImage::get_QuantizeDither(VARIANT_BOOL *pVal)
{
	// TODO: Add your implementation code here
    *pVal = oImage->quantizeDither();
	return S_OK;
}

STDMETHODIMP CImage::put_QuantizeDither(VARIANT_BOOL newVal)
{
	// TODO: Add your implementation code here
    oImage->quantizeDither(newVal);
	return S_OK;
}

STDMETHODIMP CImage::get_QuantizeTreeDepth(unsigned int *pVal)
{
	// TODO: Add your implementation code here
    *pVal = oImage->quantizeTreeDepth();
	return S_OK;
}

STDMETHODIMP CImage::put_QuantizeTreeDepth(unsigned int newVal)
{
	// TODO: Add your implementation code here
    oImage->quantizeTreeDepth(newVal);
	return S_OK;
}

STDMETHODIMP CImage::get_Scene(unsigned int *pVal)
{
	// TODO: Add your implementation code here
	*pVal = oImage->scene();
	return S_OK;
}

STDMETHODIMP CImage::put_Scene(unsigned int newVal)
{
	// TODO: Add your implementation code here
    oImage->scene(newVal);
	return S_OK;
}

STDMETHODIMP CImage::get_StrokeAntiAlias(VARIANT_BOOL *pVal)
{
	// TODO: Add your implementation code here
    *pVal = oImage->strokeAntiAlias();
	return S_OK;
}

STDMETHODIMP CImage::put_StrokeAntiAlias(VARIANT_BOOL newVal)
{
	// TODO: Add your implementation code here
    oImage->strokeAntiAlias(newVal);
	return S_OK;
}

STDMETHODIMP CImage::get_StrokeDashOffset(unsigned int *pVal)
{
	// TODO: Add your implementation code here
    *pVal = oImage->strokeDashOffset();
	return S_OK;
}

STDMETHODIMP CImage::put_StrokeDashOffset(unsigned int newVal)
{
	// TODO: Add your implementation code here
    oImage->strokeDashOffset(newVal);
	return S_OK;
}

STDMETHODIMP CImage::get_StrokeMiterLimit(unsigned int *pVal)
{
	// TODO: Add your implementation code here
    *pVal = oImage->strokeMiterLimit();
	return S_OK;
}

STDMETHODIMP CImage::put_StrokeMiterLimit(unsigned int newVal)
{
	// TODO: Add your implementation code here
    oImage->strokeMiterLimit(newVal);
	return S_OK;
}

STDMETHODIMP CImage::get_StrokeWidth(double *pVal)
{
	// TODO: Add your implementation code here
    *pVal = oImage->strokeWidth();
	return S_OK;
}

STDMETHODIMP CImage::put_StrokeWidth(double newVal)
{
	// TODO: Add your implementation code here
    oImage->strokeWidth(newVal);
 	return S_OK;
}

STDMETHODIMP CImage::get_SubImage(unsigned int *pVal)
{
	// TODO: Add your implementation code here
    *pVal = oImage->subImage();
	return S_OK;
}

STDMETHODIMP CImage::put_SubImage(unsigned int newVal)
{
	// TODO: Add your implementation code here
    oImage->subImage(newVal);
	return S_OK;
}

STDMETHODIMP CImage::get_SubRange(unsigned int *pVal)
{
	// TODO: Add your implementation code here
    *pVal = oImage->subRange();
	return S_OK;
}

STDMETHODIMP CImage::put_SubRange(unsigned int newVal)
{
	// TODO: Add your implementation code here
    oImage->subRange(newVal);
	return S_OK;
}

STDMETHODIMP CImage::get_TotalColors(unsigned long *pVal)
{
	// TODO: Add your implementation code here
    *pVal = oImage->totalColors();
	return S_OK;
}

STDMETHODIMP CImage::get_XResolution(double *pVal)
{
	// TODO: Add your implementation code here
    *pVal = oImage->xResolution();
	return S_OK;
}

STDMETHODIMP CImage::get_YResolution(double *pVal)
{
	// TODO: Add your implementation code here
    *pVal = oImage->yResolution();
	return S_OK;
}

STDMETHODIMP CImage::get_Signature(VARIANT_BOOL Force, BSTR *pVal)
{
	// TODO: Add your implementation code here
	USES_CONVERSION;	
	string strSig = oImage->signature(Force);
    CComBSTR cstrSig = A2OLE(strSig.c_str());
	*pVal = cstrSig; 
    
	return S_OK;
}

STDMETHODIMP CImage::get_Label(BSTR *pVal)
{
	// TODO: Add your implementation code here
	USES_CONVERSION;	
	string strLabel = oImage->label();
    CComBSTR cstrLabel = A2OLE(strLabel.c_str());
	*pVal = cstrLabel; 

	return S_OK;
}

STDMETHODIMP CImage::put_Label(BSTR newVal)
{
	// TODO: Add your implementation code here
   USES_CONVERSION;	

    oImage->label(newVal ? OLE2A(newVal) : "");
	return S_OK;
}

STDMETHODIMP CImage::Border(BSTR cGeometry, VARIANT_BOOL *pVal)
{
	// TODO: Add your implementation code here
   try {
	    USES_CONVERSION;
		oImage->border(cGeometry ? OLE2A(cGeometry) : "");
		*pVal = true;
	}
	catch( Exception &error_ )
	{
		string ErrorG = error_.what();

		CImage::SetErrorMessage(ErrorG.c_str());
		*pVal = false;
	}
	
	return S_OK;
}

STDMETHODIMP CImage::AddNoise(int nNoiseType, VARIANT_BOOL *pVal)
{
	// TODO: Add your implementation code here
	// See \Magick-x.x.x\Magick\Classify.h for enums
    try {		
		oImage->addNoise(static_cast<NoiseType>(nNoiseType));
	}
	catch ( Exception &error_ )
	{
		string ErrorG = error_.what();

		CImage::SetErrorMessage(ErrorG.c_str());
		*pVal = false;
	}

	return S_OK;
}


STDMETHODIMP CImage::Channel(int nChannelType, VARIANT_BOOL *pVal)
{
	// TODO: Add your implementation code here
	// See \Magick-x.x.x\Magick\Classify.h for enums
    try {		
		oImage->channel(static_cast<ChannelType>(nChannelType));
	}
	catch ( Exception &error_ )
	{
		string ErrorG = error_.what();

		CImage::SetErrorMessage(ErrorG.c_str());
		*pVal = false;
	}

	return S_OK;
}

STDMETHODIMP CImage::Chop(BSTR cGeometry, VARIANT_BOOL *pVal)
{
	// TODO: Add your implementation code here
	try {
        USES_CONVERSION;		
		oImage->chop(cGeometry ? OLE2A(cGeometry) : "");
	}
	catch ( Exception &error_ )
	{
		string ErrorG = error_.what();

		CImage::SetErrorMessage(ErrorG.c_str());
		*pVal = false;
	}

	return S_OK;
}

STDMETHODIMP CImage::CycleColormap(int nAmount, VARIANT_BOOL *pVal)
{
	// TODO: Add your implementation code here
	try {
		oImage->cycleColormap(nAmount);
	}
	catch ( Exception &error_ )
	{
		string ErrorG = error_.what();

		CImage::SetErrorMessage(ErrorG.c_str());
		*pVal = false;
	}

	return S_OK;
}

STDMETHODIMP CImage::Frame(unsigned int width, unsigned int height, int InnerBevel, int OuterBevel, VARIANT_BOOL *pVal)
{
	// TODO: Add your implementation code here
	try {
		oImage->frame(width, height, InnerBevel, OuterBevel);
	}
	catch ( Exception &error_ )
	{
		string ErrorG = error_.what();

		CImage::SetErrorMessage(ErrorG.c_str());
		*pVal = false;
	}

	return S_OK;
}

STDMETHODIMP CImage::Ping(BSTR cFilename, VARIANT_BOOL *pVal)
{
	// TODO: Add your implementation code here
	try {
		USES_CONVERSION;
		oImage->ping(cFilename ? OLE2A(cFilename) : "");
	}
	catch ( Exception &error_ )
	{
		string ErrorG = error_.what();

		CImage::SetErrorMessage(ErrorG.c_str());
		*pVal = false;
	}

	return S_OK;
}

STDMETHODIMP CImage::Raise(BSTR cGeometry, VARIANT_BOOL RaisedFlag, VARIANT_BOOL *pVal)
{
	// TODO: Add your implementation code here
	try {
		USES_CONVERSION;
		oImage->raise(cGeometry ? OLE2A(cGeometry) : "");
	}
	catch ( Exception &error_ )
	{
		string ErrorG = error_.what();

		CImage::SetErrorMessage(ErrorG.c_str());
		*pVal = false;
	}

	return S_OK;
}

STDMETHODIMP CImage::ReduceNoise(int Order, VARIANT_BOOL *pVal)
{
	// TODO: Add your implementation code here
	try {
		oImage->reduceNoise(Order);
	}
	catch ( Exception &error_ )
	{
		string ErrorG = error_.what();

		CImage::SetErrorMessage(ErrorG.c_str());
		*pVal = false;
	}

	return S_OK;
}

STDMETHODIMP CImage::Segment(double ClusterThreshold, double SmoothingThreshold, VARIANT_BOOL *pVal)
{
	// TODO: Add your implementation code here
	try {
		oImage->segment(ClusterThreshold, SmoothingThreshold);
	}
	catch ( Exception &error_ )
	{
		string ErrorG = error_.what();

		CImage::SetErrorMessage(ErrorG.c_str());
		*pVal = false;
	}

	return S_OK;
}

STDMETHODIMP CImage::Shave(BSTR cGeometry, VARIANT_BOOL *pVal)
{
	// TODO: Add your implementation code here
	try {
		USES_CONVERSION;
		oImage->shave(cGeometry ? OLE2A(cGeometry) : "");
	}
	catch ( Exception &error_ )
	{
		string ErrorG = error_.what();

		CImage::SetErrorMessage(ErrorG.c_str());
		*pVal = false;
	}

	return S_OK;
}



STDMETHODIMP CImage::ShowMessage(long uiMsg)
{
	// Sample code to test the idea of passing
	// the address of the underlying C++ class
	// when using COM. 

	if(uiMsg > 0)
	{
       CImageControl* pImageControl = 0 ;
	   pImageControl = reinterpret_cast<CImageControl*>(uiMsg);
	   pImageControl->SampleMessage();
	}
	return S_OK;
}

//IImageCollection **pVal)
STDMETHODIMP CImage::get_Images(IImageCollection **pVal)
{
	
	IImageCollection *pImageCollection;
	HRESULT hrImage = 0;

	// TODO: Add your implementation code here	

    hrImage = this->QueryInterface(IID_IImageCollection, (void **)&pImageCollection);
    
	if (FAILED(hrImage))
	{   
		*pVal = NULL;
	}
	else
	{
	    *pVal = pImageCollection;
	}

	return S_OK;
}

STDMETHODIMP CImage::GetImageAddress(long *pVal)
{
	// NOTE: The technique used here of getting the image address is not
	//       really a valid way to use a COM object. It's definitely in 
	//       realm of a "hack". These objects must be in the same address
	//       space, and this definitely won't work under DCOM. 

	//const long uiMem = reinterpret_cast<const long>(&oImage); 	 
	const long uiMem = reinterpret_cast<const long>(oImage); 	 

	*pVal = uiMem;
	return S_OK;
}

STDMETHODIMP CImage::TestAddress(const Magick::Image &pImage)
{
	char buffer[20];
	_itoa((long) &pImage, buffer, 10);
	MessageBox(NULL, buffer, "MagickCOM TestAddress", MB_OK);
	return 0;
}

STDMETHODIMP CImage::Composite(IImage *pImage, BSTR cGeometry, int CompositeOp, VARIANT_BOOL *pVal)
{

	long uiMem = 0;
    // Using the object reference to another image passed in, call it's
	// GetImageAddress method. We can then cast that back into an
	// image pointer.	
	
	pImage->GetImageAddress(&uiMem);
	
    
	Magick::Image* pAltImage = 0;
    
	pAltImage = reinterpret_cast<Magick::Image*>(uiMem);	
    
	try {
		USES_CONVERSION;
		//int xOff = 0;
		//int yOff = 0;
		//oImage->composite(*pAltImage, xOff, yOff, (CompositeOperator) CompositeOp);
		oImage->composite(*pAltImage, cGeometry ? OLE2A(cGeometry) : "", (CompositeOperator) CompositeOp);				
		*pVal = true;
	}
	catch ( Exception &error_ )
	{
		string ErrorG = error_.what();

		CImage::SetErrorMessage(ErrorG.c_str());
		*pVal = false;
	}
	    

	return S_OK;
}


STDMETHODIMP CImage::Stegano(IImage *pImage, VARIANT_BOOL *pVal)
{
	// TODO: Add your implementation code here
    
	long uiMem = 0;
	pImage->GetImageAddress(&uiMem);

	Magick::Image* pAltImage = 0;
	pAltImage = reinterpret_cast<Magick::Image*>(uiMem);

	try {
		oImage->stegano(*pAltImage);		
		*pVal = true;
	}
	catch ( Exception &error_ )
	{
		string ErrorG = error_.what();
		CImage::SetErrorMessage(ErrorG.c_str());

		*pVal = false;
	}

	return S_OK;
}
