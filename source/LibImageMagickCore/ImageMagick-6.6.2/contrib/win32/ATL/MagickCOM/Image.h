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

// Image.h : Declaration of the CImage

#ifndef __Image_H_
#define __Image_H_

#include "resource.h"       // main symbols
#include <asptlb.h>         // Active Server Pages Definitions
#include "string.h"
#include "MagickCOM.h"
#include "Magick++.h"
//#include "CImageCollection.h"


/////////////////////////////////////////////////////////////////////////////
// CImage
class ATL_NO_VTABLE CImage : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CImage, &CLSID_Image>,
	public ISupportErrorInfo,
	public IDispatchImpl<IImage, &IID_IImage, &LIBID_MAGICKCOMLib>	
{
public:
	CImage()
	{ 
		m_bOnStartPageCalled = FALSE;
		ImageWidth = 70;
		ImageHeight = 70;
		oImage = new Magick::Image;

		//MessageBox(NULL, "Constructor fired.", "MagickCOM Status", MB_OK);
	}
	~CImage()
	{
		delete oImage;
		oImage = 0;
	}

public:

DECLARE_REGISTRY_RESOURCEID(IDR_IMAGE)

DECLARE_PROTECT_FINAL_CONSTRUCT()
DECLARE_NOT_AGGREGATABLE(CImage)

BEGIN_COM_MAP(CImage)
	COM_INTERFACE_ENTRY(IImage)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

// IImage
public:
	STDMETHOD(Stegano)(/*[in]*/IImage *pImage, /*[out, retval]*/VARIANT_BOOL *pVal);
	STDMETHOD(Composite)(/*[in]*/ IImage *pImage, BSTR cGeometry, int CompositeOp, /*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(GetImageAddress)(/*[out, retval]*/ long *pVal);
	STDMETHOD(get_Images)(/*[out, retval]*/ IImageCollection* *pVal);
	STDMETHOD(ShowMessage)(long uiMsg);
	STDMETHOD(Shave)(/*[in]*/ BSTR cGeometry, /*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(Segment)(/*[in]*/ double ClusterThreshold, double SmoothingThreshold,/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(ReduceNoise)(/*[in]*/ int Order, /*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(Raise)(/*[in]*/ BSTR cGeometry, /*[out, retval]*/ VARIANT_BOOL RaisedFlag, VARIANT_BOOL *pVal);
	STDMETHOD(Ping)(/*[in]*/ BSTR cFilename, /*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(Frame)(/*[in]*/ unsigned int width, unsigned int height, int InnerBevel, int OuterBevel, /*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(CycleColormap)(/*[in]*/ int nAmount, /*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(Chop)(/*[in]*/ BSTR cGeometry, /*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(Channel)(/*[in]*/ int nChannelType,/*[out, retval]*/  VARIANT_BOOL *pVal);
	STDMETHOD(AddNoise)(/*[in]*/ int nNoiseType, /*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(Border)(/*[in]*/ BSTR cGeometry, /*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(get_Label)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_Label)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_Signature)(VARIANT_BOOL Force, /*[out, retval]*/ BSTR *pVal);
	STDMETHOD(get_YResolution)(/*[out, retval]*/ double *pVal);
	STDMETHOD(get_XResolution)(/*[out, retval]*/ double *pVal);
	STDMETHOD(get_TotalColors)(/*[out, retval]*/ unsigned long *pVal);
	STDMETHOD(get_SubRange)(/*[out, retval]*/ unsigned int *pVal);
	STDMETHOD(put_SubRange)(/*[in]*/ unsigned int newVal);
	STDMETHOD(get_SubImage)(/*[out, retval]*/ unsigned int *pVal);
	STDMETHOD(put_SubImage)(/*[in]*/ unsigned int newVal);
	STDMETHOD(get_StrokeWidth)(/*[out, retval]*/ double *pVal);
	STDMETHOD(put_StrokeWidth)(/*[in]*/ double newVal);
	STDMETHOD(get_StrokeMiterLimit)(/*[out, retval]*/ unsigned int *pVal);
	STDMETHOD(put_StrokeMiterLimit)(/*[in]*/ unsigned int newVal);
	STDMETHOD(get_StrokeDashOffset)(/*[out, retval]*/ unsigned int *pVal);
	STDMETHOD(put_StrokeDashOffset)(/*[in]*/ unsigned int newVal);
	STDMETHOD(get_StrokeAntiAlias)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_StrokeAntiAlias)(/*[in]*/ VARIANT_BOOL newVal);
	STDMETHOD(get_Scene)(/*[out, retval]*/ unsigned int *pVal);
	STDMETHOD(put_Scene)(/*[in]*/ unsigned int newVal);
	STDMETHOD(get_QuantizeTreeDepth)(/*[out, retval]*/ unsigned int *pVal);
	STDMETHOD(put_QuantizeTreeDepth)(/*[in]*/ unsigned int newVal);
	STDMETHOD(get_QuantizeDither)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_QuantizeDither)(/*[in]*/ VARIANT_BOOL newVal);
	STDMETHOD(get_QuantizedColors)(/*[out, retval]*/ unsigned int *pVal);
	STDMETHOD(put_QuantizedColors)(/*[in]*/ unsigned int newVal);
	STDMETHOD(get_NormalizedMeanError)(/*[out, retval]*/ double *pVal);
	STDMETHOD(get_NormalizedMaxError)(/*[out, retval]*/ double *pVal);
	STDMETHOD(get_Monochrome)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_Monochrome)(/*[in]*/ VARIANT_BOOL newVal);
	STDMETHOD(get_Matte)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_Matte)(/*[in]*/ VARIANT_BOOL newVal);
	STDMETHOD(get_Magick)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_Magick)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_GIFDisposeMethod)(/*[out, retval]*/ unsigned int *pVal);
	STDMETHOD(put_GIFDisposeMethod)(/*[in]*/ unsigned int newVal);
	STDMETHOD(get_Density)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_Density)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_ColorFuzz)(/*[out, retval]*/ double *pVal);
	STDMETHOD(put_ColorFuzz)(/*[in]*/ double newVal);
	STDMETHOD(get_BaseRows)(/*[out, retval]*/ unsigned int *pVal);
	STDMETHOD(get_BaseColumns)(/*[out, retval]*/ unsigned int *pVal);
	STDMETHOD(get_BackgroundTexture)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_BackgroundTexture)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_AnimationIterations)(/*[out, retval]*/ unsigned int *pVal);
	STDMETHOD(put_AnimationIterations)(/*[in]*/ unsigned int newVal);
	STDMETHOD(get_AnimationDelay)(/*[out, retval]*/ unsigned int *pVal);
	STDMETHOD(put_AnimationDelay)(/*[in]*/ unsigned int newVal);
	STDMETHOD(get_AntiAlias)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_AntiAlias)(/*[in]*/ VARIANT_BOOL newVal);
	STDMETHOD(get_Adjoin)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_Adjoin)(/*[in]*/ VARIANT_BOOL newVal);
	STDMETHOD(get_FileSize)(/*[out, retval]*/ long *pVal);
	STDMETHOD(get_Depth)(/*[out, retval]*/ unsigned int *pVal);
	STDMETHOD(put_Depth)(/*[in]*/ unsigned int newVal);
	STDMETHOD(get_Quality)(/*[out, retval]*/ unsigned int *pVal);
	STDMETHOD(put_Quality)(/*[in]*/ unsigned int Quality);
	STDMETHOD(get_meanErrorPerPixel)(/*[out, retval]*/ double *pVal);
	STDMETHOD(get_Format)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(Crop)(BSTR geometry, /*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(Threshold)(double threshold, /*[out, retval]*/  VARIANT_BOOL *pVal);
	STDMETHOD(Quantize)(VARIANT_BOOL measureError, /*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(Magnify)(/*[out, retval]*/VARIANT_BOOL *pVal);
	STDMETHOD(Erase)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(Charcoal)(double radius, double sigma, /*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(Blur)(double radius, double sigma, /*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(Trim)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(UnsharpMask)(double radius, double sigma, double amount, double threshold, /*[out, retval]*/  VARIANT_BOOL *pVal);
	STDMETHOD(Wave)(double amplitude, double wavelength, /*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(Swirl)(double degrees, /*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(Shear)(double xShearAngle, double yShearAngle, /*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(Shade)(double azimuth, double elevation, VARIANT_BOOL colorShading, /*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(Sharpen)(double radius, double sigma, /*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(Spread)(unsigned int amount, /*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(Solarize)(double factor, /*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(Roll)(int columns, int rows, /*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(Opacity)(unsigned int opacity, /*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(OilPaint)(unsigned int radius, /*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(Negate)(VARIANT_BOOL grayscale, /*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(Modulate)(double brightness, double saturation, double hue, /*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(Minify)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(MedianFilter)(double radius, /*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(Implode)(double factor, /*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(GaussianBlur)(double width, double sigma, /*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(Gamma)(double Red, double Green, double Blue, /*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(Emboss)(double radius, double sigma, /*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(Edge)(unsigned int radius, /*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(Contrast)(unsigned int sharpen, /*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(Despeckle)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(Normalize)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(Flop)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(Equalize)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(Enhance)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(Flip)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(Rotate)(double Degrees, /*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(Write)(BSTR cFilename, /*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(Sample)(unsigned int x, unsigned int y, VARIANT_BOOL *pVal);
	STDMETHOD(Scale)(unsigned int x, unsigned int y, /*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(get_Rows)(/*[out, retval]*/ unsigned int *pVal);	
	STDMETHOD(get_Columns)(/*[out, retval]*/ unsigned int *pVal);
	STDMETHOD(InitMagick)(BSTR cPath);
	STDMETHOD(get_ErrorMsg)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_ErrorMsg)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_ImageHeight)(/*[out, retval]*/ unsigned int *pVal);
	STDMETHOD(put_ImageHeight)(/*[in]*/ unsigned int newVal);
	STDMETHOD(get_ImageWidth)(/*[out, retval]*/ unsigned int *pVal);
	STDMETHOD(put_ImageWidth)(/*[in]*/ unsigned int newVal);
	STDMETHOD(Resize)(BSTR cFilename, BSTR cOutput, /*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(Read)(BSTR cFilename, /*[out, retval]*/ VARIANT_BOOL *pVal);	
	STDMETHOD(TestAddress)(/*[in]*/const Magick::Image &pImage);
    CComBSTR ErrorMsg;
    unsigned int ImageWidth;
	unsigned int ImageHeight;

	//Active Server Pages Methods
	STDMETHOD(OnStartPage)(IUnknown* IUnk);
	STDMETHOD(OnEndPage)();
	

private:
	CComPtr<IRequest> m_piRequest;					//Request Object
	CComPtr<IResponse> m_piResponse;				//Response Object
	CComPtr<ISessionObject> m_piSession;			//Session Object
	CComPtr<IServer> m_piServer;					//Server Object
	CComPtr<IApplicationObject> m_piApplication;	//Application Object
	BOOL m_bOnStartPageCalled;						//OnStartPage successful?	
	//STDMETHOD(SetErrorMessage)(const char *);
protected:
	void SetErrorMessage(const char *strErrorG);
	unsigned int iSampleVar;
	Magick::Image* oImage;
};

#endif //__Image_H_
