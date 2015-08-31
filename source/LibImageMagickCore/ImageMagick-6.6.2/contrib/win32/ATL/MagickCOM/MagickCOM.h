/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 5.01.0164 */
/* at Tue Jul 23 09:47:13 2002
 */
/* Compiler settings for C:\Program Files\Microsoft Visual Studio\MyProjects\MagickCOM\MagickCOM.idl:
    Oicf (OptLev=i2), W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
*/
//@@MIDL_FILE_HEADING(  )


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __MagickCOM_h__
#define __MagickCOM_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

#ifndef __IImage_FWD_DEFINED__
#define __IImage_FWD_DEFINED__
typedef interface IImage IImage;
#endif 	/* __IImage_FWD_DEFINED__ */


#ifndef __IImageControl_FWD_DEFINED__
#define __IImageControl_FWD_DEFINED__
typedef interface IImageControl IImageControl;
#endif 	/* __IImageControl_FWD_DEFINED__ */


#ifndef __IGeometry_FWD_DEFINED__
#define __IGeometry_FWD_DEFINED__
typedef interface IGeometry IGeometry;
#endif 	/* __IGeometry_FWD_DEFINED__ */


#ifndef __IColor_FWD_DEFINED__
#define __IColor_FWD_DEFINED__
typedef interface IColor IColor;
#endif 	/* __IColor_FWD_DEFINED__ */


#ifndef __IImageCollection_FWD_DEFINED__
#define __IImageCollection_FWD_DEFINED__
typedef interface IImageCollection IImageCollection;
#endif 	/* __IImageCollection_FWD_DEFINED__ */


#ifndef __Image_FWD_DEFINED__
#define __Image_FWD_DEFINED__

#ifdef __cplusplus
typedef class Image Image;
#else
typedef struct Image Image;
#endif /* __cplusplus */

#endif 	/* __Image_FWD_DEFINED__ */


#ifndef __ImageControl_FWD_DEFINED__
#define __ImageControl_FWD_DEFINED__

#ifdef __cplusplus
typedef class ImageControl ImageControl;
#else
typedef struct ImageControl ImageControl;
#endif /* __cplusplus */

#endif 	/* __ImageControl_FWD_DEFINED__ */


#ifndef __Geometry_FWD_DEFINED__
#define __Geometry_FWD_DEFINED__

#ifdef __cplusplus
typedef class Geometry Geometry;
#else
typedef struct Geometry Geometry;
#endif /* __cplusplus */

#endif 	/* __Geometry_FWD_DEFINED__ */


#ifndef __Color_FWD_DEFINED__
#define __Color_FWD_DEFINED__

#ifdef __cplusplus
typedef class Color Color;
#else
typedef struct Color Color;
#endif /* __cplusplus */

#endif 	/* __Color_FWD_DEFINED__ */


#ifndef __ImageCollection_FWD_DEFINED__
#define __ImageCollection_FWD_DEFINED__

#ifdef __cplusplus
typedef class ImageCollection ImageCollection;
#else
typedef struct ImageCollection ImageCollection;
#endif /* __cplusplus */

#endif 	/* __ImageCollection_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

/* interface __MIDL_itf_MagickCOM_0000 */
/* [local] */ 




extern RPC_IF_HANDLE __MIDL_itf_MagickCOM_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_MagickCOM_0000_v0_0_s_ifspec;

#ifndef __IImage_INTERFACE_DEFINED__
#define __IImage_INTERFACE_DEFINED__

/* interface IImage */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IImage;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5C6D6667-0614-4E85-801D-2ED93CEF2C19")
    IImage : public IDispatch
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnStartPage( 
            /* [in] */ IUnknown __RPC_FAR *piUnk) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnEndPage( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Resize( 
            BSTR cFilename,
            BSTR cOutput,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ImageWidth( 
            /* [retval][out] */ unsigned int __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ImageWidth( 
            /* [in] */ unsigned int newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ImageHeight( 
            /* [retval][out] */ unsigned int __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ImageHeight( 
            /* [in] */ unsigned int newVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Read( 
            /* [in] */ BSTR cFilename,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ErrorMsg( 
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ErrorMsg( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE InitMagick( 
            BSTR cPath) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Columns( 
            /* [retval][out] */ unsigned int __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Rows( 
            /* [retval][out] */ unsigned int __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Scale( 
            unsigned int x,
            unsigned int y,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Sample( 
            unsigned int x,
            unsigned int y,
            VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Write( 
            BSTR cFilename,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Rotate( 
            double Degrees,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Flip( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Enhance( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Equalize( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Flop( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Normalize( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Despeckle( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Contrast( 
            unsigned int sharpen,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Edge( 
            unsigned int radius,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Emboss( 
            double radius,
            double sigma,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Gamma( 
            double Red,
            double Green,
            double Blue,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GaussianBlur( 
            double width,
            double sigma,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Implode( 
            double factor,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE MedianFilter( 
            double radius,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Minify( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Modulate( 
            double brightness,
            double saturation,
            double hue,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Negate( 
            VARIANT_BOOL grayscale,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE OilPaint( 
            unsigned int radius,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Opacity( 
            unsigned int opacity,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Roll( 
            int columns,
            int rows,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Solarize( 
            double factor,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Spread( 
            unsigned int amount,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Sharpen( 
            double radius,
            double sigma,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Shade( 
            double azimuth,
            double elevation,
            VARIANT_BOOL colorShading,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Shear( 
            double xShearAngle,
            double yShearAngle,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Swirl( 
            double degrees,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Wave( 
            double amplitude,
            double wavelength,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE UnsharpMask( 
            double radius,
            double sigma,
            double amount,
            double threshold,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Trim( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Blur( 
            double radius,
            double sigma,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Charcoal( 
            double radius,
            double sigma,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Erase( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Magnify( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Quantize( 
            VARIANT_BOOL measureError,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Threshold( 
            double threshold,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Crop( 
            BSTR geometry,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Format( 
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_meanErrorPerPixel( 
            /* [retval][out] */ double __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Quality( 
            /* [retval][out] */ unsigned int __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Quality( 
            /* [in] */ unsigned int Quality) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Depth( 
            /* [retval][out] */ unsigned int __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Depth( 
            /* [in] */ unsigned int newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_FileSize( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Adjoin( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Adjoin( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_AntiAlias( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_AntiAlias( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_AnimationDelay( 
            /* [retval][out] */ unsigned int __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_AnimationDelay( 
            /* [in] */ unsigned int newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_AnimationIterations( 
            /* [retval][out] */ unsigned int __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_AnimationIterations( 
            /* [in] */ unsigned int newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_BackgroundTexture( 
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_BackgroundTexture( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_BaseColumns( 
            /* [retval][out] */ unsigned int __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_BaseRows( 
            /* [retval][out] */ unsigned int __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ColorFuzz( 
            /* [retval][out] */ double __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ColorFuzz( 
            /* [in] */ double newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Density( 
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Density( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_GIFDisposeMethod( 
            /* [retval][out] */ unsigned int __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_GIFDisposeMethod( 
            /* [in] */ unsigned int newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Magick( 
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Magick( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Matte( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Matte( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Monochrome( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Monochrome( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_NormalizedMaxError( 
            /* [retval][out] */ double __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_NormalizedMeanError( 
            /* [retval][out] */ double __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_QuantizedColors( 
            /* [retval][out] */ unsigned int __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_QuantizedColors( 
            /* [in] */ unsigned int newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_QuantizeDither( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_QuantizeDither( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_QuantizeTreeDepth( 
            /* [retval][out] */ unsigned int __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_QuantizeTreeDepth( 
            /* [in] */ unsigned int newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Scene( 
            /* [retval][out] */ unsigned int __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Scene( 
            /* [in] */ unsigned int newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_StrokeAntiAlias( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_StrokeAntiAlias( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_StrokeDashOffset( 
            /* [retval][out] */ unsigned int __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_StrokeDashOffset( 
            /* [in] */ unsigned int newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_StrokeMiterLimit( 
            /* [retval][out] */ unsigned int __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_StrokeMiterLimit( 
            /* [in] */ unsigned int newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_StrokeWidth( 
            /* [retval][out] */ double __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_StrokeWidth( 
            /* [in] */ double newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_SubImage( 
            /* [retval][out] */ unsigned int __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_SubImage( 
            /* [in] */ unsigned int newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_SubRange( 
            /* [retval][out] */ unsigned int __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_SubRange( 
            /* [in] */ unsigned int newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_TotalColors( 
            /* [retval][out] */ unsigned long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_XResolution( 
            /* [retval][out] */ double __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_YResolution( 
            /* [retval][out] */ double __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Signature( 
            VARIANT_BOOL Force,
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Label( 
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Label( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Border( 
            BSTR cGeometry,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE AddNoise( 
            int nNoiseType,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Channel( 
            int nChannelType,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Chop( 
            BSTR cGeometry,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE CycleColormap( 
            int nAmount,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Frame( 
            unsigned int width,
            unsigned int height,
            int InnerBevel,
            int OuterBevel,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Ping( 
            BSTR cFilename,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Raise( 
            BSTR cGeometry,
            VARIANT_BOOL RaisedFlag,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ReduceNoise( 
            int Order,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Segment( 
            double ClusterThreshold,
            double SmoothingThreshold,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Shave( 
            BSTR cGeometry,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ShowMessage( 
            long uiMsg) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Images( 
            /* [retval][out] */ IImageCollection __RPC_FAR *__RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetImageAddress( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Composite( 
            IImage __RPC_FAR *pImage,
            BSTR cGeometry,
            int CompositeOp,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Stegano( 
            /* [in] */ IImage __RPC_FAR *pImage,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IImageVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IImage __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IImage __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IImage __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IImage __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IImage __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IImage __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IImage __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OnStartPage )( 
            IImage __RPC_FAR * This,
            /* [in] */ IUnknown __RPC_FAR *piUnk);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OnEndPage )( 
            IImage __RPC_FAR * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Resize )( 
            IImage __RPC_FAR * This,
            BSTR cFilename,
            BSTR cOutput,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ImageWidth )( 
            IImage __RPC_FAR * This,
            /* [retval][out] */ unsigned int __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ImageWidth )( 
            IImage __RPC_FAR * This,
            /* [in] */ unsigned int newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ImageHeight )( 
            IImage __RPC_FAR * This,
            /* [retval][out] */ unsigned int __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ImageHeight )( 
            IImage __RPC_FAR * This,
            /* [in] */ unsigned int newVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Read )( 
            IImage __RPC_FAR * This,
            /* [in] */ BSTR cFilename,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ErrorMsg )( 
            IImage __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ErrorMsg )( 
            IImage __RPC_FAR * This,
            /* [in] */ BSTR newVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *InitMagick )( 
            IImage __RPC_FAR * This,
            BSTR cPath);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Columns )( 
            IImage __RPC_FAR * This,
            /* [retval][out] */ unsigned int __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Rows )( 
            IImage __RPC_FAR * This,
            /* [retval][out] */ unsigned int __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Scale )( 
            IImage __RPC_FAR * This,
            unsigned int x,
            unsigned int y,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Sample )( 
            IImage __RPC_FAR * This,
            unsigned int x,
            unsigned int y,
            VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Write )( 
            IImage __RPC_FAR * This,
            BSTR cFilename,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Rotate )( 
            IImage __RPC_FAR * This,
            double Degrees,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Flip )( 
            IImage __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Enhance )( 
            IImage __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Equalize )( 
            IImage __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Flop )( 
            IImage __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Normalize )( 
            IImage __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Despeckle )( 
            IImage __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Contrast )( 
            IImage __RPC_FAR * This,
            unsigned int sharpen,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Edge )( 
            IImage __RPC_FAR * This,
            unsigned int radius,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Emboss )( 
            IImage __RPC_FAR * This,
            double radius,
            double sigma,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Gamma )( 
            IImage __RPC_FAR * This,
            double Red,
            double Green,
            double Blue,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GaussianBlur )( 
            IImage __RPC_FAR * This,
            double width,
            double sigma,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Implode )( 
            IImage __RPC_FAR * This,
            double factor,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *MedianFilter )( 
            IImage __RPC_FAR * This,
            double radius,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Minify )( 
            IImage __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Modulate )( 
            IImage __RPC_FAR * This,
            double brightness,
            double saturation,
            double hue,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Negate )( 
            IImage __RPC_FAR * This,
            VARIANT_BOOL grayscale,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OilPaint )( 
            IImage __RPC_FAR * This,
            unsigned int radius,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Opacity )( 
            IImage __RPC_FAR * This,
            unsigned int opacity,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Roll )( 
            IImage __RPC_FAR * This,
            int columns,
            int rows,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Solarize )( 
            IImage __RPC_FAR * This,
            double factor,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Spread )( 
            IImage __RPC_FAR * This,
            unsigned int amount,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Sharpen )( 
            IImage __RPC_FAR * This,
            double radius,
            double sigma,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Shade )( 
            IImage __RPC_FAR * This,
            double azimuth,
            double elevation,
            VARIANT_BOOL colorShading,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Shear )( 
            IImage __RPC_FAR * This,
            double xShearAngle,
            double yShearAngle,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Swirl )( 
            IImage __RPC_FAR * This,
            double degrees,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Wave )( 
            IImage __RPC_FAR * This,
            double amplitude,
            double wavelength,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *UnsharpMask )( 
            IImage __RPC_FAR * This,
            double radius,
            double sigma,
            double amount,
            double threshold,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Trim )( 
            IImage __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Blur )( 
            IImage __RPC_FAR * This,
            double radius,
            double sigma,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Charcoal )( 
            IImage __RPC_FAR * This,
            double radius,
            double sigma,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Erase )( 
            IImage __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Magnify )( 
            IImage __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Quantize )( 
            IImage __RPC_FAR * This,
            VARIANT_BOOL measureError,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Threshold )( 
            IImage __RPC_FAR * This,
            double threshold,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Crop )( 
            IImage __RPC_FAR * This,
            BSTR geometry,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Format )( 
            IImage __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_meanErrorPerPixel )( 
            IImage __RPC_FAR * This,
            /* [retval][out] */ double __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Quality )( 
            IImage __RPC_FAR * This,
            /* [retval][out] */ unsigned int __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Quality )( 
            IImage __RPC_FAR * This,
            /* [in] */ unsigned int Quality);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Depth )( 
            IImage __RPC_FAR * This,
            /* [retval][out] */ unsigned int __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Depth )( 
            IImage __RPC_FAR * This,
            /* [in] */ unsigned int newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_FileSize )( 
            IImage __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Adjoin )( 
            IImage __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Adjoin )( 
            IImage __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_AntiAlias )( 
            IImage __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_AntiAlias )( 
            IImage __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_AnimationDelay )( 
            IImage __RPC_FAR * This,
            /* [retval][out] */ unsigned int __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_AnimationDelay )( 
            IImage __RPC_FAR * This,
            /* [in] */ unsigned int newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_AnimationIterations )( 
            IImage __RPC_FAR * This,
            /* [retval][out] */ unsigned int __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_AnimationIterations )( 
            IImage __RPC_FAR * This,
            /* [in] */ unsigned int newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_BackgroundTexture )( 
            IImage __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_BackgroundTexture )( 
            IImage __RPC_FAR * This,
            /* [in] */ BSTR newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_BaseColumns )( 
            IImage __RPC_FAR * This,
            /* [retval][out] */ unsigned int __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_BaseRows )( 
            IImage __RPC_FAR * This,
            /* [retval][out] */ unsigned int __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ColorFuzz )( 
            IImage __RPC_FAR * This,
            /* [retval][out] */ double __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ColorFuzz )( 
            IImage __RPC_FAR * This,
            /* [in] */ double newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Density )( 
            IImage __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Density )( 
            IImage __RPC_FAR * This,
            /* [in] */ BSTR newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_GIFDisposeMethod )( 
            IImage __RPC_FAR * This,
            /* [retval][out] */ unsigned int __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_GIFDisposeMethod )( 
            IImage __RPC_FAR * This,
            /* [in] */ unsigned int newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Magick )( 
            IImage __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Magick )( 
            IImage __RPC_FAR * This,
            /* [in] */ BSTR newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Matte )( 
            IImage __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Matte )( 
            IImage __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Monochrome )( 
            IImage __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Monochrome )( 
            IImage __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_NormalizedMaxError )( 
            IImage __RPC_FAR * This,
            /* [retval][out] */ double __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_NormalizedMeanError )( 
            IImage __RPC_FAR * This,
            /* [retval][out] */ double __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_QuantizedColors )( 
            IImage __RPC_FAR * This,
            /* [retval][out] */ unsigned int __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_QuantizedColors )( 
            IImage __RPC_FAR * This,
            /* [in] */ unsigned int newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_QuantizeDither )( 
            IImage __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_QuantizeDither )( 
            IImage __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_QuantizeTreeDepth )( 
            IImage __RPC_FAR * This,
            /* [retval][out] */ unsigned int __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_QuantizeTreeDepth )( 
            IImage __RPC_FAR * This,
            /* [in] */ unsigned int newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Scene )( 
            IImage __RPC_FAR * This,
            /* [retval][out] */ unsigned int __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Scene )( 
            IImage __RPC_FAR * This,
            /* [in] */ unsigned int newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_StrokeAntiAlias )( 
            IImage __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_StrokeAntiAlias )( 
            IImage __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_StrokeDashOffset )( 
            IImage __RPC_FAR * This,
            /* [retval][out] */ unsigned int __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_StrokeDashOffset )( 
            IImage __RPC_FAR * This,
            /* [in] */ unsigned int newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_StrokeMiterLimit )( 
            IImage __RPC_FAR * This,
            /* [retval][out] */ unsigned int __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_StrokeMiterLimit )( 
            IImage __RPC_FAR * This,
            /* [in] */ unsigned int newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_StrokeWidth )( 
            IImage __RPC_FAR * This,
            /* [retval][out] */ double __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_StrokeWidth )( 
            IImage __RPC_FAR * This,
            /* [in] */ double newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_SubImage )( 
            IImage __RPC_FAR * This,
            /* [retval][out] */ unsigned int __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_SubImage )( 
            IImage __RPC_FAR * This,
            /* [in] */ unsigned int newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_SubRange )( 
            IImage __RPC_FAR * This,
            /* [retval][out] */ unsigned int __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_SubRange )( 
            IImage __RPC_FAR * This,
            /* [in] */ unsigned int newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_TotalColors )( 
            IImage __RPC_FAR * This,
            /* [retval][out] */ unsigned long __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_XResolution )( 
            IImage __RPC_FAR * This,
            /* [retval][out] */ double __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_YResolution )( 
            IImage __RPC_FAR * This,
            /* [retval][out] */ double __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Signature )( 
            IImage __RPC_FAR * This,
            VARIANT_BOOL Force,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Label )( 
            IImage __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Label )( 
            IImage __RPC_FAR * This,
            /* [in] */ BSTR newVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Border )( 
            IImage __RPC_FAR * This,
            BSTR cGeometry,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddNoise )( 
            IImage __RPC_FAR * This,
            int nNoiseType,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Channel )( 
            IImage __RPC_FAR * This,
            int nChannelType,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Chop )( 
            IImage __RPC_FAR * This,
            BSTR cGeometry,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CycleColormap )( 
            IImage __RPC_FAR * This,
            int nAmount,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Frame )( 
            IImage __RPC_FAR * This,
            unsigned int width,
            unsigned int height,
            int InnerBevel,
            int OuterBevel,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Ping )( 
            IImage __RPC_FAR * This,
            BSTR cFilename,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Raise )( 
            IImage __RPC_FAR * This,
            BSTR cGeometry,
            VARIANT_BOOL RaisedFlag,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ReduceNoise )( 
            IImage __RPC_FAR * This,
            int Order,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Segment )( 
            IImage __RPC_FAR * This,
            double ClusterThreshold,
            double SmoothingThreshold,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Shave )( 
            IImage __RPC_FAR * This,
            BSTR cGeometry,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ShowMessage )( 
            IImage __RPC_FAR * This,
            long uiMsg);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Images )( 
            IImage __RPC_FAR * This,
            /* [retval][out] */ IImageCollection __RPC_FAR *__RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetImageAddress )( 
            IImage __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Composite )( 
            IImage __RPC_FAR * This,
            IImage __RPC_FAR *pImage,
            BSTR cGeometry,
            int CompositeOp,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Stegano )( 
            IImage __RPC_FAR * This,
            /* [in] */ IImage __RPC_FAR *pImage,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        END_INTERFACE
    } IImageVtbl;

    interface IImage
    {
        CONST_VTBL struct IImageVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IImage_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IImage_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IImage_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IImage_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IImage_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IImage_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IImage_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IImage_OnStartPage(This,piUnk)	\
    (This)->lpVtbl -> OnStartPage(This,piUnk)

#define IImage_OnEndPage(This)	\
    (This)->lpVtbl -> OnEndPage(This)

#define IImage_Resize(This,cFilename,cOutput,pVal)	\
    (This)->lpVtbl -> Resize(This,cFilename,cOutput,pVal)

#define IImage_get_ImageWidth(This,pVal)	\
    (This)->lpVtbl -> get_ImageWidth(This,pVal)

#define IImage_put_ImageWidth(This,newVal)	\
    (This)->lpVtbl -> put_ImageWidth(This,newVal)

#define IImage_get_ImageHeight(This,pVal)	\
    (This)->lpVtbl -> get_ImageHeight(This,pVal)

#define IImage_put_ImageHeight(This,newVal)	\
    (This)->lpVtbl -> put_ImageHeight(This,newVal)

#define IImage_Read(This,cFilename,pVal)	\
    (This)->lpVtbl -> Read(This,cFilename,pVal)

#define IImage_get_ErrorMsg(This,pVal)	\
    (This)->lpVtbl -> get_ErrorMsg(This,pVal)

#define IImage_put_ErrorMsg(This,newVal)	\
    (This)->lpVtbl -> put_ErrorMsg(This,newVal)

#define IImage_InitMagick(This,cPath)	\
    (This)->lpVtbl -> InitMagick(This,cPath)

#define IImage_get_Columns(This,pVal)	\
    (This)->lpVtbl -> get_Columns(This,pVal)

#define IImage_get_Rows(This,pVal)	\
    (This)->lpVtbl -> get_Rows(This,pVal)

#define IImage_Scale(This,x,y,pVal)	\
    (This)->lpVtbl -> Scale(This,x,y,pVal)

#define IImage_Sample(This,x,y,pVal)	\
    (This)->lpVtbl -> Sample(This,x,y,pVal)

#define IImage_Write(This,cFilename,pVal)	\
    (This)->lpVtbl -> Write(This,cFilename,pVal)

#define IImage_Rotate(This,Degrees,pVal)	\
    (This)->lpVtbl -> Rotate(This,Degrees,pVal)

#define IImage_Flip(This,pVal)	\
    (This)->lpVtbl -> Flip(This,pVal)

#define IImage_Enhance(This,pVal)	\
    (This)->lpVtbl -> Enhance(This,pVal)

#define IImage_Equalize(This,pVal)	\
    (This)->lpVtbl -> Equalize(This,pVal)

#define IImage_Flop(This,pVal)	\
    (This)->lpVtbl -> Flop(This,pVal)

#define IImage_Normalize(This,pVal)	\
    (This)->lpVtbl -> Normalize(This,pVal)

#define IImage_Despeckle(This,pVal)	\
    (This)->lpVtbl -> Despeckle(This,pVal)

#define IImage_Contrast(This,sharpen,pVal)	\
    (This)->lpVtbl -> Contrast(This,sharpen,pVal)

#define IImage_Edge(This,radius,pVal)	\
    (This)->lpVtbl -> Edge(This,radius,pVal)

#define IImage_Emboss(This,radius,sigma,pVal)	\
    (This)->lpVtbl -> Emboss(This,radius,sigma,pVal)

#define IImage_Gamma(This,Red,Green,Blue,pVal)	\
    (This)->lpVtbl -> Gamma(This,Red,Green,Blue,pVal)

#define IImage_GaussianBlur(This,width,sigma,pVal)	\
    (This)->lpVtbl -> GaussianBlur(This,width,sigma,pVal)

#define IImage_Implode(This,factor,pVal)	\
    (This)->lpVtbl -> Implode(This,factor,pVal)

#define IImage_MedianFilter(This,radius,pVal)	\
    (This)->lpVtbl -> MedianFilter(This,radius,pVal)

#define IImage_Minify(This,pVal)	\
    (This)->lpVtbl -> Minify(This,pVal)

#define IImage_Modulate(This,brightness,saturation,hue,pVal)	\
    (This)->lpVtbl -> Modulate(This,brightness,saturation,hue,pVal)

#define IImage_Negate(This,grayscale,pVal)	\
    (This)->lpVtbl -> Negate(This,grayscale,pVal)

#define IImage_OilPaint(This,radius,pVal)	\
    (This)->lpVtbl -> OilPaint(This,radius,pVal)

#define IImage_Opacity(This,opacity,pVal)	\
    (This)->lpVtbl -> Opacity(This,opacity,pVal)

#define IImage_Roll(This,columns,rows,pVal)	\
    (This)->lpVtbl -> Roll(This,columns,rows,pVal)

#define IImage_Solarize(This,factor,pVal)	\
    (This)->lpVtbl -> Solarize(This,factor,pVal)

#define IImage_Spread(This,amount,pVal)	\
    (This)->lpVtbl -> Spread(This,amount,pVal)

#define IImage_Sharpen(This,radius,sigma,pVal)	\
    (This)->lpVtbl -> Sharpen(This,radius,sigma,pVal)

#define IImage_Shade(This,azimuth,elevation,colorShading,pVal)	\
    (This)->lpVtbl -> Shade(This,azimuth,elevation,colorShading,pVal)

#define IImage_Shear(This,xShearAngle,yShearAngle,pVal)	\
    (This)->lpVtbl -> Shear(This,xShearAngle,yShearAngle,pVal)

#define IImage_Swirl(This,degrees,pVal)	\
    (This)->lpVtbl -> Swirl(This,degrees,pVal)

#define IImage_Wave(This,amplitude,wavelength,pVal)	\
    (This)->lpVtbl -> Wave(This,amplitude,wavelength,pVal)

#define IImage_UnsharpMask(This,radius,sigma,amount,threshold,pVal)	\
    (This)->lpVtbl -> UnsharpMask(This,radius,sigma,amount,threshold,pVal)

#define IImage_Trim(This,pVal)	\
    (This)->lpVtbl -> Trim(This,pVal)

#define IImage_Blur(This,radius,sigma,pVal)	\
    (This)->lpVtbl -> Blur(This,radius,sigma,pVal)

#define IImage_Charcoal(This,radius,sigma,pVal)	\
    (This)->lpVtbl -> Charcoal(This,radius,sigma,pVal)

#define IImage_Erase(This,pVal)	\
    (This)->lpVtbl -> Erase(This,pVal)

#define IImage_Magnify(This,pVal)	\
    (This)->lpVtbl -> Magnify(This,pVal)

#define IImage_Quantize(This,measureError,pVal)	\
    (This)->lpVtbl -> Quantize(This,measureError,pVal)

#define IImage_Threshold(This,threshold,pVal)	\
    (This)->lpVtbl -> Threshold(This,threshold,pVal)

#define IImage_Crop(This,geometry,pVal)	\
    (This)->lpVtbl -> Crop(This,geometry,pVal)

#define IImage_get_Format(This,pVal)	\
    (This)->lpVtbl -> get_Format(This,pVal)

#define IImage_get_meanErrorPerPixel(This,pVal)	\
    (This)->lpVtbl -> get_meanErrorPerPixel(This,pVal)

#define IImage_get_Quality(This,pVal)	\
    (This)->lpVtbl -> get_Quality(This,pVal)

#define IImage_put_Quality(This,Quality)	\
    (This)->lpVtbl -> put_Quality(This,Quality)

#define IImage_get_Depth(This,pVal)	\
    (This)->lpVtbl -> get_Depth(This,pVal)

#define IImage_put_Depth(This,newVal)	\
    (This)->lpVtbl -> put_Depth(This,newVal)

#define IImage_get_FileSize(This,pVal)	\
    (This)->lpVtbl -> get_FileSize(This,pVal)

#define IImage_get_Adjoin(This,pVal)	\
    (This)->lpVtbl -> get_Adjoin(This,pVal)

#define IImage_put_Adjoin(This,newVal)	\
    (This)->lpVtbl -> put_Adjoin(This,newVal)

#define IImage_get_AntiAlias(This,pVal)	\
    (This)->lpVtbl -> get_AntiAlias(This,pVal)

#define IImage_put_AntiAlias(This,newVal)	\
    (This)->lpVtbl -> put_AntiAlias(This,newVal)

#define IImage_get_AnimationDelay(This,pVal)	\
    (This)->lpVtbl -> get_AnimationDelay(This,pVal)

#define IImage_put_AnimationDelay(This,newVal)	\
    (This)->lpVtbl -> put_AnimationDelay(This,newVal)

#define IImage_get_AnimationIterations(This,pVal)	\
    (This)->lpVtbl -> get_AnimationIterations(This,pVal)

#define IImage_put_AnimationIterations(This,newVal)	\
    (This)->lpVtbl -> put_AnimationIterations(This,newVal)

#define IImage_get_BackgroundTexture(This,pVal)	\
    (This)->lpVtbl -> get_BackgroundTexture(This,pVal)

#define IImage_put_BackgroundTexture(This,newVal)	\
    (This)->lpVtbl -> put_BackgroundTexture(This,newVal)

#define IImage_get_BaseColumns(This,pVal)	\
    (This)->lpVtbl -> get_BaseColumns(This,pVal)

#define IImage_get_BaseRows(This,pVal)	\
    (This)->lpVtbl -> get_BaseRows(This,pVal)

#define IImage_get_ColorFuzz(This,pVal)	\
    (This)->lpVtbl -> get_ColorFuzz(This,pVal)

#define IImage_put_ColorFuzz(This,newVal)	\
    (This)->lpVtbl -> put_ColorFuzz(This,newVal)

#define IImage_get_Density(This,pVal)	\
    (This)->lpVtbl -> get_Density(This,pVal)

#define IImage_put_Density(This,newVal)	\
    (This)->lpVtbl -> put_Density(This,newVal)

#define IImage_get_GIFDisposeMethod(This,pVal)	\
    (This)->lpVtbl -> get_GIFDisposeMethod(This,pVal)

#define IImage_put_GIFDisposeMethod(This,newVal)	\
    (This)->lpVtbl -> put_GIFDisposeMethod(This,newVal)

#define IImage_get_Magick(This,pVal)	\
    (This)->lpVtbl -> get_Magick(This,pVal)

#define IImage_put_Magick(This,newVal)	\
    (This)->lpVtbl -> put_Magick(This,newVal)

#define IImage_get_Matte(This,pVal)	\
    (This)->lpVtbl -> get_Matte(This,pVal)

#define IImage_put_Matte(This,newVal)	\
    (This)->lpVtbl -> put_Matte(This,newVal)

#define IImage_get_Monochrome(This,pVal)	\
    (This)->lpVtbl -> get_Monochrome(This,pVal)

#define IImage_put_Monochrome(This,newVal)	\
    (This)->lpVtbl -> put_Monochrome(This,newVal)

#define IImage_get_NormalizedMaxError(This,pVal)	\
    (This)->lpVtbl -> get_NormalizedMaxError(This,pVal)

#define IImage_get_NormalizedMeanError(This,pVal)	\
    (This)->lpVtbl -> get_NormalizedMeanError(This,pVal)

#define IImage_get_QuantizedColors(This,pVal)	\
    (This)->lpVtbl -> get_QuantizedColors(This,pVal)

#define IImage_put_QuantizedColors(This,newVal)	\
    (This)->lpVtbl -> put_QuantizedColors(This,newVal)

#define IImage_get_QuantizeDither(This,pVal)	\
    (This)->lpVtbl -> get_QuantizeDither(This,pVal)

#define IImage_put_QuantizeDither(This,newVal)	\
    (This)->lpVtbl -> put_QuantizeDither(This,newVal)

#define IImage_get_QuantizeTreeDepth(This,pVal)	\
    (This)->lpVtbl -> get_QuantizeTreeDepth(This,pVal)

#define IImage_put_QuantizeTreeDepth(This,newVal)	\
    (This)->lpVtbl -> put_QuantizeTreeDepth(This,newVal)

#define IImage_get_Scene(This,pVal)	\
    (This)->lpVtbl -> get_Scene(This,pVal)

#define IImage_put_Scene(This,newVal)	\
    (This)->lpVtbl -> put_Scene(This,newVal)

#define IImage_get_StrokeAntiAlias(This,pVal)	\
    (This)->lpVtbl -> get_StrokeAntiAlias(This,pVal)

#define IImage_put_StrokeAntiAlias(This,newVal)	\
    (This)->lpVtbl -> put_StrokeAntiAlias(This,newVal)

#define IImage_get_StrokeDashOffset(This,pVal)	\
    (This)->lpVtbl -> get_StrokeDashOffset(This,pVal)

#define IImage_put_StrokeDashOffset(This,newVal)	\
    (This)->lpVtbl -> put_StrokeDashOffset(This,newVal)

#define IImage_get_StrokeMiterLimit(This,pVal)	\
    (This)->lpVtbl -> get_StrokeMiterLimit(This,pVal)

#define IImage_put_StrokeMiterLimit(This,newVal)	\
    (This)->lpVtbl -> put_StrokeMiterLimit(This,newVal)

#define IImage_get_StrokeWidth(This,pVal)	\
    (This)->lpVtbl -> get_StrokeWidth(This,pVal)

#define IImage_put_StrokeWidth(This,newVal)	\
    (This)->lpVtbl -> put_StrokeWidth(This,newVal)

#define IImage_get_SubImage(This,pVal)	\
    (This)->lpVtbl -> get_SubImage(This,pVal)

#define IImage_put_SubImage(This,newVal)	\
    (This)->lpVtbl -> put_SubImage(This,newVal)

#define IImage_get_SubRange(This,pVal)	\
    (This)->lpVtbl -> get_SubRange(This,pVal)

#define IImage_put_SubRange(This,newVal)	\
    (This)->lpVtbl -> put_SubRange(This,newVal)

#define IImage_get_TotalColors(This,pVal)	\
    (This)->lpVtbl -> get_TotalColors(This,pVal)

#define IImage_get_XResolution(This,pVal)	\
    (This)->lpVtbl -> get_XResolution(This,pVal)

#define IImage_get_YResolution(This,pVal)	\
    (This)->lpVtbl -> get_YResolution(This,pVal)

#define IImage_get_Signature(This,Force,pVal)	\
    (This)->lpVtbl -> get_Signature(This,Force,pVal)

#define IImage_get_Label(This,pVal)	\
    (This)->lpVtbl -> get_Label(This,pVal)

#define IImage_put_Label(This,newVal)	\
    (This)->lpVtbl -> put_Label(This,newVal)

#define IImage_Border(This,cGeometry,pVal)	\
    (This)->lpVtbl -> Border(This,cGeometry,pVal)

#define IImage_AddNoise(This,nNoiseType,pVal)	\
    (This)->lpVtbl -> AddNoise(This,nNoiseType,pVal)

#define IImage_Channel(This,nChannelType,pVal)	\
    (This)->lpVtbl -> Channel(This,nChannelType,pVal)

#define IImage_Chop(This,cGeometry,pVal)	\
    (This)->lpVtbl -> Chop(This,cGeometry,pVal)

#define IImage_CycleColormap(This,nAmount,pVal)	\
    (This)->lpVtbl -> CycleColormap(This,nAmount,pVal)

#define IImage_Frame(This,width,height,InnerBevel,OuterBevel,pVal)	\
    (This)->lpVtbl -> Frame(This,width,height,InnerBevel,OuterBevel,pVal)

#define IImage_Ping(This,cFilename,pVal)	\
    (This)->lpVtbl -> Ping(This,cFilename,pVal)

#define IImage_Raise(This,cGeometry,RaisedFlag,pVal)	\
    (This)->lpVtbl -> Raise(This,cGeometry,RaisedFlag,pVal)

#define IImage_ReduceNoise(This,Order,pVal)	\
    (This)->lpVtbl -> ReduceNoise(This,Order,pVal)

#define IImage_Segment(This,ClusterThreshold,SmoothingThreshold,pVal)	\
    (This)->lpVtbl -> Segment(This,ClusterThreshold,SmoothingThreshold,pVal)

#define IImage_Shave(This,cGeometry,pVal)	\
    (This)->lpVtbl -> Shave(This,cGeometry,pVal)

#define IImage_ShowMessage(This,uiMsg)	\
    (This)->lpVtbl -> ShowMessage(This,uiMsg)

#define IImage_get_Images(This,pVal)	\
    (This)->lpVtbl -> get_Images(This,pVal)

#define IImage_GetImageAddress(This,pVal)	\
    (This)->lpVtbl -> GetImageAddress(This,pVal)

#define IImage_Composite(This,pImage,cGeometry,CompositeOp,pVal)	\
    (This)->lpVtbl -> Composite(This,pImage,cGeometry,CompositeOp,pVal)

#define IImage_Stegano(This,pImage,pVal)	\
    (This)->lpVtbl -> Stegano(This,pImage,pVal)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IImage_OnStartPage_Proxy( 
    IImage __RPC_FAR * This,
    /* [in] */ IUnknown __RPC_FAR *piUnk);


void __RPC_STUB IImage_OnStartPage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IImage_OnEndPage_Proxy( 
    IImage __RPC_FAR * This);


void __RPC_STUB IImage_OnEndPage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IImage_Resize_Proxy( 
    IImage __RPC_FAR * This,
    BSTR cFilename,
    BSTR cOutput,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IImage_Resize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IImage_get_ImageWidth_Proxy( 
    IImage __RPC_FAR * This,
    /* [retval][out] */ unsigned int __RPC_FAR *pVal);


void __RPC_STUB IImage_get_ImageWidth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IImage_put_ImageWidth_Proxy( 
    IImage __RPC_FAR * This,
    /* [in] */ unsigned int newVal);


void __RPC_STUB IImage_put_ImageWidth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IImage_get_ImageHeight_Proxy( 
    IImage __RPC_FAR * This,
    /* [retval][out] */ unsigned int __RPC_FAR *pVal);


void __RPC_STUB IImage_get_ImageHeight_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IImage_put_ImageHeight_Proxy( 
    IImage __RPC_FAR * This,
    /* [in] */ unsigned int newVal);


void __RPC_STUB IImage_put_ImageHeight_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IImage_Read_Proxy( 
    IImage __RPC_FAR * This,
    /* [in] */ BSTR cFilename,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IImage_Read_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IImage_get_ErrorMsg_Proxy( 
    IImage __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pVal);


void __RPC_STUB IImage_get_ErrorMsg_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IImage_put_ErrorMsg_Proxy( 
    IImage __RPC_FAR * This,
    /* [in] */ BSTR newVal);


void __RPC_STUB IImage_put_ErrorMsg_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IImage_InitMagick_Proxy( 
    IImage __RPC_FAR * This,
    BSTR cPath);


void __RPC_STUB IImage_InitMagick_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IImage_get_Columns_Proxy( 
    IImage __RPC_FAR * This,
    /* [retval][out] */ unsigned int __RPC_FAR *pVal);


void __RPC_STUB IImage_get_Columns_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IImage_get_Rows_Proxy( 
    IImage __RPC_FAR * This,
    /* [retval][out] */ unsigned int __RPC_FAR *pVal);


void __RPC_STUB IImage_get_Rows_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IImage_Scale_Proxy( 
    IImage __RPC_FAR * This,
    unsigned int x,
    unsigned int y,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IImage_Scale_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IImage_Sample_Proxy( 
    IImage __RPC_FAR * This,
    unsigned int x,
    unsigned int y,
    VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IImage_Sample_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IImage_Write_Proxy( 
    IImage __RPC_FAR * This,
    BSTR cFilename,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IImage_Write_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IImage_Rotate_Proxy( 
    IImage __RPC_FAR * This,
    double Degrees,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IImage_Rotate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IImage_Flip_Proxy( 
    IImage __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IImage_Flip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IImage_Enhance_Proxy( 
    IImage __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IImage_Enhance_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IImage_Equalize_Proxy( 
    IImage __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IImage_Equalize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IImage_Flop_Proxy( 
    IImage __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IImage_Flop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IImage_Normalize_Proxy( 
    IImage __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IImage_Normalize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IImage_Despeckle_Proxy( 
    IImage __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IImage_Despeckle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IImage_Contrast_Proxy( 
    IImage __RPC_FAR * This,
    unsigned int sharpen,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IImage_Contrast_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IImage_Edge_Proxy( 
    IImage __RPC_FAR * This,
    unsigned int radius,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IImage_Edge_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IImage_Emboss_Proxy( 
    IImage __RPC_FAR * This,
    double radius,
    double sigma,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IImage_Emboss_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IImage_Gamma_Proxy( 
    IImage __RPC_FAR * This,
    double Red,
    double Green,
    double Blue,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IImage_Gamma_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IImage_GaussianBlur_Proxy( 
    IImage __RPC_FAR * This,
    double width,
    double sigma,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IImage_GaussianBlur_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IImage_Implode_Proxy( 
    IImage __RPC_FAR * This,
    double factor,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IImage_Implode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IImage_MedianFilter_Proxy( 
    IImage __RPC_FAR * This,
    double radius,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IImage_MedianFilter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IImage_Minify_Proxy( 
    IImage __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IImage_Minify_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IImage_Modulate_Proxy( 
    IImage __RPC_FAR * This,
    double brightness,
    double saturation,
    double hue,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IImage_Modulate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IImage_Negate_Proxy( 
    IImage __RPC_FAR * This,
    VARIANT_BOOL grayscale,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IImage_Negate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IImage_OilPaint_Proxy( 
    IImage __RPC_FAR * This,
    unsigned int radius,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IImage_OilPaint_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IImage_Opacity_Proxy( 
    IImage __RPC_FAR * This,
    unsigned int opacity,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IImage_Opacity_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IImage_Roll_Proxy( 
    IImage __RPC_FAR * This,
    int columns,
    int rows,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IImage_Roll_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IImage_Solarize_Proxy( 
    IImage __RPC_FAR * This,
    double factor,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IImage_Solarize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IImage_Spread_Proxy( 
    IImage __RPC_FAR * This,
    unsigned int amount,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IImage_Spread_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IImage_Sharpen_Proxy( 
    IImage __RPC_FAR * This,
    double radius,
    double sigma,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IImage_Sharpen_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IImage_Shade_Proxy( 
    IImage __RPC_FAR * This,
    double azimuth,
    double elevation,
    VARIANT_BOOL colorShading,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IImage_Shade_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IImage_Shear_Proxy( 
    IImage __RPC_FAR * This,
    double xShearAngle,
    double yShearAngle,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IImage_Shear_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IImage_Swirl_Proxy( 
    IImage __RPC_FAR * This,
    double degrees,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IImage_Swirl_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IImage_Wave_Proxy( 
    IImage __RPC_FAR * This,
    double amplitude,
    double wavelength,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IImage_Wave_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IImage_UnsharpMask_Proxy( 
    IImage __RPC_FAR * This,
    double radius,
    double sigma,
    double amount,
    double threshold,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IImage_UnsharpMask_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IImage_Trim_Proxy( 
    IImage __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IImage_Trim_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IImage_Blur_Proxy( 
    IImage __RPC_FAR * This,
    double radius,
    double sigma,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IImage_Blur_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IImage_Charcoal_Proxy( 
    IImage __RPC_FAR * This,
    double radius,
    double sigma,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IImage_Charcoal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IImage_Erase_Proxy( 
    IImage __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IImage_Erase_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IImage_Magnify_Proxy( 
    IImage __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IImage_Magnify_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IImage_Quantize_Proxy( 
    IImage __RPC_FAR * This,
    VARIANT_BOOL measureError,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IImage_Quantize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IImage_Threshold_Proxy( 
    IImage __RPC_FAR * This,
    double threshold,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IImage_Threshold_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IImage_Crop_Proxy( 
    IImage __RPC_FAR * This,
    BSTR geometry,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IImage_Crop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IImage_get_Format_Proxy( 
    IImage __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pVal);


void __RPC_STUB IImage_get_Format_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IImage_get_meanErrorPerPixel_Proxy( 
    IImage __RPC_FAR * This,
    /* [retval][out] */ double __RPC_FAR *pVal);


void __RPC_STUB IImage_get_meanErrorPerPixel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IImage_get_Quality_Proxy( 
    IImage __RPC_FAR * This,
    /* [retval][out] */ unsigned int __RPC_FAR *pVal);


void __RPC_STUB IImage_get_Quality_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IImage_put_Quality_Proxy( 
    IImage __RPC_FAR * This,
    /* [in] */ unsigned int Quality);


void __RPC_STUB IImage_put_Quality_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IImage_get_Depth_Proxy( 
    IImage __RPC_FAR * This,
    /* [retval][out] */ unsigned int __RPC_FAR *pVal);


void __RPC_STUB IImage_get_Depth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IImage_put_Depth_Proxy( 
    IImage __RPC_FAR * This,
    /* [in] */ unsigned int newVal);


void __RPC_STUB IImage_put_Depth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IImage_get_FileSize_Proxy( 
    IImage __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IImage_get_FileSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IImage_get_Adjoin_Proxy( 
    IImage __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IImage_get_Adjoin_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IImage_put_Adjoin_Proxy( 
    IImage __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL newVal);


void __RPC_STUB IImage_put_Adjoin_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IImage_get_AntiAlias_Proxy( 
    IImage __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IImage_get_AntiAlias_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IImage_put_AntiAlias_Proxy( 
    IImage __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL newVal);


void __RPC_STUB IImage_put_AntiAlias_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IImage_get_AnimationDelay_Proxy( 
    IImage __RPC_FAR * This,
    /* [retval][out] */ unsigned int __RPC_FAR *pVal);


void __RPC_STUB IImage_get_AnimationDelay_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IImage_put_AnimationDelay_Proxy( 
    IImage __RPC_FAR * This,
    /* [in] */ unsigned int newVal);


void __RPC_STUB IImage_put_AnimationDelay_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IImage_get_AnimationIterations_Proxy( 
    IImage __RPC_FAR * This,
    /* [retval][out] */ unsigned int __RPC_FAR *pVal);


void __RPC_STUB IImage_get_AnimationIterations_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IImage_put_AnimationIterations_Proxy( 
    IImage __RPC_FAR * This,
    /* [in] */ unsigned int newVal);


void __RPC_STUB IImage_put_AnimationIterations_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IImage_get_BackgroundTexture_Proxy( 
    IImage __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pVal);


void __RPC_STUB IImage_get_BackgroundTexture_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IImage_put_BackgroundTexture_Proxy( 
    IImage __RPC_FAR * This,
    /* [in] */ BSTR newVal);


void __RPC_STUB IImage_put_BackgroundTexture_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IImage_get_BaseColumns_Proxy( 
    IImage __RPC_FAR * This,
    /* [retval][out] */ unsigned int __RPC_FAR *pVal);


void __RPC_STUB IImage_get_BaseColumns_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IImage_get_BaseRows_Proxy( 
    IImage __RPC_FAR * This,
    /* [retval][out] */ unsigned int __RPC_FAR *pVal);


void __RPC_STUB IImage_get_BaseRows_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IImage_get_ColorFuzz_Proxy( 
    IImage __RPC_FAR * This,
    /* [retval][out] */ double __RPC_FAR *pVal);


void __RPC_STUB IImage_get_ColorFuzz_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IImage_put_ColorFuzz_Proxy( 
    IImage __RPC_FAR * This,
    /* [in] */ double newVal);


void __RPC_STUB IImage_put_ColorFuzz_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IImage_get_Density_Proxy( 
    IImage __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pVal);


void __RPC_STUB IImage_get_Density_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IImage_put_Density_Proxy( 
    IImage __RPC_FAR * This,
    /* [in] */ BSTR newVal);


void __RPC_STUB IImage_put_Density_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IImage_get_GIFDisposeMethod_Proxy( 
    IImage __RPC_FAR * This,
    /* [retval][out] */ unsigned int __RPC_FAR *pVal);


void __RPC_STUB IImage_get_GIFDisposeMethod_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IImage_put_GIFDisposeMethod_Proxy( 
    IImage __RPC_FAR * This,
    /* [in] */ unsigned int newVal);


void __RPC_STUB IImage_put_GIFDisposeMethod_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IImage_get_Magick_Proxy( 
    IImage __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pVal);


void __RPC_STUB IImage_get_Magick_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IImage_put_Magick_Proxy( 
    IImage __RPC_FAR * This,
    /* [in] */ BSTR newVal);


void __RPC_STUB IImage_put_Magick_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IImage_get_Matte_Proxy( 
    IImage __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IImage_get_Matte_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IImage_put_Matte_Proxy( 
    IImage __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL newVal);


void __RPC_STUB IImage_put_Matte_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IImage_get_Monochrome_Proxy( 
    IImage __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IImage_get_Monochrome_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IImage_put_Monochrome_Proxy( 
    IImage __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL newVal);


void __RPC_STUB IImage_put_Monochrome_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IImage_get_NormalizedMaxError_Proxy( 
    IImage __RPC_FAR * This,
    /* [retval][out] */ double __RPC_FAR *pVal);


void __RPC_STUB IImage_get_NormalizedMaxError_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IImage_get_NormalizedMeanError_Proxy( 
    IImage __RPC_FAR * This,
    /* [retval][out] */ double __RPC_FAR *pVal);


void __RPC_STUB IImage_get_NormalizedMeanError_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IImage_get_QuantizedColors_Proxy( 
    IImage __RPC_FAR * This,
    /* [retval][out] */ unsigned int __RPC_FAR *pVal);


void __RPC_STUB IImage_get_QuantizedColors_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IImage_put_QuantizedColors_Proxy( 
    IImage __RPC_FAR * This,
    /* [in] */ unsigned int newVal);


void __RPC_STUB IImage_put_QuantizedColors_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IImage_get_QuantizeDither_Proxy( 
    IImage __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IImage_get_QuantizeDither_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IImage_put_QuantizeDither_Proxy( 
    IImage __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL newVal);


void __RPC_STUB IImage_put_QuantizeDither_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IImage_get_QuantizeTreeDepth_Proxy( 
    IImage __RPC_FAR * This,
    /* [retval][out] */ unsigned int __RPC_FAR *pVal);


void __RPC_STUB IImage_get_QuantizeTreeDepth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IImage_put_QuantizeTreeDepth_Proxy( 
    IImage __RPC_FAR * This,
    /* [in] */ unsigned int newVal);


void __RPC_STUB IImage_put_QuantizeTreeDepth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IImage_get_Scene_Proxy( 
    IImage __RPC_FAR * This,
    /* [retval][out] */ unsigned int __RPC_FAR *pVal);


void __RPC_STUB IImage_get_Scene_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IImage_put_Scene_Proxy( 
    IImage __RPC_FAR * This,
    /* [in] */ unsigned int newVal);


void __RPC_STUB IImage_put_Scene_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IImage_get_StrokeAntiAlias_Proxy( 
    IImage __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IImage_get_StrokeAntiAlias_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IImage_put_StrokeAntiAlias_Proxy( 
    IImage __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL newVal);


void __RPC_STUB IImage_put_StrokeAntiAlias_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IImage_get_StrokeDashOffset_Proxy( 
    IImage __RPC_FAR * This,
    /* [retval][out] */ unsigned int __RPC_FAR *pVal);


void __RPC_STUB IImage_get_StrokeDashOffset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IImage_put_StrokeDashOffset_Proxy( 
    IImage __RPC_FAR * This,
    /* [in] */ unsigned int newVal);


void __RPC_STUB IImage_put_StrokeDashOffset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IImage_get_StrokeMiterLimit_Proxy( 
    IImage __RPC_FAR * This,
    /* [retval][out] */ unsigned int __RPC_FAR *pVal);


void __RPC_STUB IImage_get_StrokeMiterLimit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IImage_put_StrokeMiterLimit_Proxy( 
    IImage __RPC_FAR * This,
    /* [in] */ unsigned int newVal);


void __RPC_STUB IImage_put_StrokeMiterLimit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IImage_get_StrokeWidth_Proxy( 
    IImage __RPC_FAR * This,
    /* [retval][out] */ double __RPC_FAR *pVal);


void __RPC_STUB IImage_get_StrokeWidth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IImage_put_StrokeWidth_Proxy( 
    IImage __RPC_FAR * This,
    /* [in] */ double newVal);


void __RPC_STUB IImage_put_StrokeWidth_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IImage_get_SubImage_Proxy( 
    IImage __RPC_FAR * This,
    /* [retval][out] */ unsigned int __RPC_FAR *pVal);


void __RPC_STUB IImage_get_SubImage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IImage_put_SubImage_Proxy( 
    IImage __RPC_FAR * This,
    /* [in] */ unsigned int newVal);


void __RPC_STUB IImage_put_SubImage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IImage_get_SubRange_Proxy( 
    IImage __RPC_FAR * This,
    /* [retval][out] */ unsigned int __RPC_FAR *pVal);


void __RPC_STUB IImage_get_SubRange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IImage_put_SubRange_Proxy( 
    IImage __RPC_FAR * This,
    /* [in] */ unsigned int newVal);


void __RPC_STUB IImage_put_SubRange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IImage_get_TotalColors_Proxy( 
    IImage __RPC_FAR * This,
    /* [retval][out] */ unsigned long __RPC_FAR *pVal);


void __RPC_STUB IImage_get_TotalColors_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IImage_get_XResolution_Proxy( 
    IImage __RPC_FAR * This,
    /* [retval][out] */ double __RPC_FAR *pVal);


void __RPC_STUB IImage_get_XResolution_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IImage_get_YResolution_Proxy( 
    IImage __RPC_FAR * This,
    /* [retval][out] */ double __RPC_FAR *pVal);


void __RPC_STUB IImage_get_YResolution_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IImage_get_Signature_Proxy( 
    IImage __RPC_FAR * This,
    VARIANT_BOOL Force,
    /* [retval][out] */ BSTR __RPC_FAR *pVal);


void __RPC_STUB IImage_get_Signature_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IImage_get_Label_Proxy( 
    IImage __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pVal);


void __RPC_STUB IImage_get_Label_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IImage_put_Label_Proxy( 
    IImage __RPC_FAR * This,
    /* [in] */ BSTR newVal);


void __RPC_STUB IImage_put_Label_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IImage_Border_Proxy( 
    IImage __RPC_FAR * This,
    BSTR cGeometry,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IImage_Border_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IImage_AddNoise_Proxy( 
    IImage __RPC_FAR * This,
    int nNoiseType,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IImage_AddNoise_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IImage_Channel_Proxy( 
    IImage __RPC_FAR * This,
    int nChannelType,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IImage_Channel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IImage_Chop_Proxy( 
    IImage __RPC_FAR * This,
    BSTR cGeometry,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IImage_Chop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IImage_CycleColormap_Proxy( 
    IImage __RPC_FAR * This,
    int nAmount,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IImage_CycleColormap_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IImage_Frame_Proxy( 
    IImage __RPC_FAR * This,
    unsigned int width,
    unsigned int height,
    int InnerBevel,
    int OuterBevel,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IImage_Frame_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IImage_Ping_Proxy( 
    IImage __RPC_FAR * This,
    BSTR cFilename,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IImage_Ping_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IImage_Raise_Proxy( 
    IImage __RPC_FAR * This,
    BSTR cGeometry,
    VARIANT_BOOL RaisedFlag,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IImage_Raise_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IImage_ReduceNoise_Proxy( 
    IImage __RPC_FAR * This,
    int Order,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IImage_ReduceNoise_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IImage_Segment_Proxy( 
    IImage __RPC_FAR * This,
    double ClusterThreshold,
    double SmoothingThreshold,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IImage_Segment_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IImage_Shave_Proxy( 
    IImage __RPC_FAR * This,
    BSTR cGeometry,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IImage_Shave_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IImage_ShowMessage_Proxy( 
    IImage __RPC_FAR * This,
    long uiMsg);


void __RPC_STUB IImage_ShowMessage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IImage_get_Images_Proxy( 
    IImage __RPC_FAR * This,
    /* [retval][out] */ IImageCollection __RPC_FAR *__RPC_FAR *pVal);


void __RPC_STUB IImage_get_Images_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IImage_GetImageAddress_Proxy( 
    IImage __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IImage_GetImageAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IImage_Composite_Proxy( 
    IImage __RPC_FAR * This,
    IImage __RPC_FAR *pImage,
    BSTR cGeometry,
    int CompositeOp,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IImage_Composite_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IImage_Stegano_Proxy( 
    IImage __RPC_FAR * This,
    /* [in] */ IImage __RPC_FAR *pImage,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IImage_Stegano_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IImage_INTERFACE_DEFINED__ */


#ifndef __IImageControl_INTERFACE_DEFINED__
#define __IImageControl_INTERFACE_DEFINED__

/* interface IImageControl */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IImageControl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5B5A7365-75BA-4F88-ABBF-64695DAAA2B0")
    IImageControl : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Image( 
            /* [retval][out] */ IImage __RPC_FAR *__RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_GetAddress( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SampleMessage( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IImageControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IImageControl __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IImageControl __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IImageControl __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IImageControl __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IImageControl __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IImageControl __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IImageControl __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Image )( 
            IImageControl __RPC_FAR * This,
            /* [retval][out] */ IImage __RPC_FAR *__RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_GetAddress )( 
            IImageControl __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SampleMessage )( 
            IImageControl __RPC_FAR * This);
        
        END_INTERFACE
    } IImageControlVtbl;

    interface IImageControl
    {
        CONST_VTBL struct IImageControlVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IImageControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IImageControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IImageControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IImageControl_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IImageControl_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IImageControl_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IImageControl_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IImageControl_get_Image(This,pVal)	\
    (This)->lpVtbl -> get_Image(This,pVal)

#define IImageControl_get_GetAddress(This,pVal)	\
    (This)->lpVtbl -> get_GetAddress(This,pVal)

#define IImageControl_SampleMessage(This)	\
    (This)->lpVtbl -> SampleMessage(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IImageControl_get_Image_Proxy( 
    IImageControl __RPC_FAR * This,
    /* [retval][out] */ IImage __RPC_FAR *__RPC_FAR *pVal);


void __RPC_STUB IImageControl_get_Image_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IImageControl_get_GetAddress_Proxy( 
    IImageControl __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IImageControl_get_GetAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IImageControl_SampleMessage_Proxy( 
    IImageControl __RPC_FAR * This);


void __RPC_STUB IImageControl_SampleMessage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IImageControl_INTERFACE_DEFINED__ */


#ifndef __IGeometry_INTERFACE_DEFINED__
#define __IGeometry_INTERFACE_DEFINED__

/* interface IGeometry */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IGeometry;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2DB6FBA7-2E7E-4F2D-AC2E-29AFB42E419F")
    IGeometry : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Width( 
            /* [retval][out] */ unsigned int __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Width( 
            /* [in] */ unsigned int newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Height( 
            /* [retval][out] */ unsigned int __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Height( 
            /* [in] */ unsigned int newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_XOff( 
            /* [retval][out] */ unsigned int __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_XOff( 
            /* [in] */ unsigned int newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_YOff( 
            /* [retval][out] */ unsigned int __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_YOff( 
            /* [in] */ unsigned int newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_XNegative( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_XNegative( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_YNegative( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_YNegative( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Percent( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Percent( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Aspect( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Aspect( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Greater( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Greater( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Less( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Less( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsValid( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Geometry( 
            /* [in] */ BSTR PSSizeNick,
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IGeometryVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IGeometry __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IGeometry __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IGeometry __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IGeometry __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IGeometry __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IGeometry __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IGeometry __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Width )( 
            IGeometry __RPC_FAR * This,
            /* [retval][out] */ unsigned int __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Width )( 
            IGeometry __RPC_FAR * This,
            /* [in] */ unsigned int newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Height )( 
            IGeometry __RPC_FAR * This,
            /* [retval][out] */ unsigned int __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Height )( 
            IGeometry __RPC_FAR * This,
            /* [in] */ unsigned int newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_XOff )( 
            IGeometry __RPC_FAR * This,
            /* [retval][out] */ unsigned int __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_XOff )( 
            IGeometry __RPC_FAR * This,
            /* [in] */ unsigned int newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_YOff )( 
            IGeometry __RPC_FAR * This,
            /* [retval][out] */ unsigned int __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_YOff )( 
            IGeometry __RPC_FAR * This,
            /* [in] */ unsigned int newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_XNegative )( 
            IGeometry __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_XNegative )( 
            IGeometry __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_YNegative )( 
            IGeometry __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_YNegative )( 
            IGeometry __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Percent )( 
            IGeometry __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Percent )( 
            IGeometry __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Aspect )( 
            IGeometry __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Aspect )( 
            IGeometry __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Greater )( 
            IGeometry __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Greater )( 
            IGeometry __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Less )( 
            IGeometry __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Less )( 
            IGeometry __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsValid )( 
            IGeometry __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Geometry )( 
            IGeometry __RPC_FAR * This,
            /* [in] */ BSTR PSSizeNick,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        END_INTERFACE
    } IGeometryVtbl;

    interface IGeometry
    {
        CONST_VTBL struct IGeometryVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGeometry_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IGeometry_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IGeometry_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IGeometry_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IGeometry_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IGeometry_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IGeometry_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IGeometry_get_Width(This,pVal)	\
    (This)->lpVtbl -> get_Width(This,pVal)

#define IGeometry_put_Width(This,newVal)	\
    (This)->lpVtbl -> put_Width(This,newVal)

#define IGeometry_get_Height(This,pVal)	\
    (This)->lpVtbl -> get_Height(This,pVal)

#define IGeometry_put_Height(This,newVal)	\
    (This)->lpVtbl -> put_Height(This,newVal)

#define IGeometry_get_XOff(This,pVal)	\
    (This)->lpVtbl -> get_XOff(This,pVal)

#define IGeometry_put_XOff(This,newVal)	\
    (This)->lpVtbl -> put_XOff(This,newVal)

#define IGeometry_get_YOff(This,pVal)	\
    (This)->lpVtbl -> get_YOff(This,pVal)

#define IGeometry_put_YOff(This,newVal)	\
    (This)->lpVtbl -> put_YOff(This,newVal)

#define IGeometry_get_XNegative(This,pVal)	\
    (This)->lpVtbl -> get_XNegative(This,pVal)

#define IGeometry_put_XNegative(This,newVal)	\
    (This)->lpVtbl -> put_XNegative(This,newVal)

#define IGeometry_get_YNegative(This,pVal)	\
    (This)->lpVtbl -> get_YNegative(This,pVal)

#define IGeometry_put_YNegative(This,newVal)	\
    (This)->lpVtbl -> put_YNegative(This,newVal)

#define IGeometry_get_Percent(This,pVal)	\
    (This)->lpVtbl -> get_Percent(This,pVal)

#define IGeometry_put_Percent(This,newVal)	\
    (This)->lpVtbl -> put_Percent(This,newVal)

#define IGeometry_get_Aspect(This,pVal)	\
    (This)->lpVtbl -> get_Aspect(This,pVal)

#define IGeometry_put_Aspect(This,newVal)	\
    (This)->lpVtbl -> put_Aspect(This,newVal)

#define IGeometry_get_Greater(This,pVal)	\
    (This)->lpVtbl -> get_Greater(This,pVal)

#define IGeometry_put_Greater(This,newVal)	\
    (This)->lpVtbl -> put_Greater(This,newVal)

#define IGeometry_get_Less(This,pVal)	\
    (This)->lpVtbl -> get_Less(This,pVal)

#define IGeometry_put_Less(This,newVal)	\
    (This)->lpVtbl -> put_Less(This,newVal)

#define IGeometry_IsValid(This,pVal)	\
    (This)->lpVtbl -> IsValid(This,pVal)

#define IGeometry_Geometry(This,PSSizeNick,pVal)	\
    (This)->lpVtbl -> Geometry(This,PSSizeNick,pVal)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IGeometry_get_Width_Proxy( 
    IGeometry __RPC_FAR * This,
    /* [retval][out] */ unsigned int __RPC_FAR *pVal);


void __RPC_STUB IGeometry_get_Width_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IGeometry_put_Width_Proxy( 
    IGeometry __RPC_FAR * This,
    /* [in] */ unsigned int newVal);


void __RPC_STUB IGeometry_put_Width_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IGeometry_get_Height_Proxy( 
    IGeometry __RPC_FAR * This,
    /* [retval][out] */ unsigned int __RPC_FAR *pVal);


void __RPC_STUB IGeometry_get_Height_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IGeometry_put_Height_Proxy( 
    IGeometry __RPC_FAR * This,
    /* [in] */ unsigned int newVal);


void __RPC_STUB IGeometry_put_Height_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IGeometry_get_XOff_Proxy( 
    IGeometry __RPC_FAR * This,
    /* [retval][out] */ unsigned int __RPC_FAR *pVal);


void __RPC_STUB IGeometry_get_XOff_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IGeometry_put_XOff_Proxy( 
    IGeometry __RPC_FAR * This,
    /* [in] */ unsigned int newVal);


void __RPC_STUB IGeometry_put_XOff_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IGeometry_get_YOff_Proxy( 
    IGeometry __RPC_FAR * This,
    /* [retval][out] */ unsigned int __RPC_FAR *pVal);


void __RPC_STUB IGeometry_get_YOff_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IGeometry_put_YOff_Proxy( 
    IGeometry __RPC_FAR * This,
    /* [in] */ unsigned int newVal);


void __RPC_STUB IGeometry_put_YOff_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IGeometry_get_XNegative_Proxy( 
    IGeometry __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IGeometry_get_XNegative_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IGeometry_put_XNegative_Proxy( 
    IGeometry __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL newVal);


void __RPC_STUB IGeometry_put_XNegative_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IGeometry_get_YNegative_Proxy( 
    IGeometry __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IGeometry_get_YNegative_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IGeometry_put_YNegative_Proxy( 
    IGeometry __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL newVal);


void __RPC_STUB IGeometry_put_YNegative_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IGeometry_get_Percent_Proxy( 
    IGeometry __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IGeometry_get_Percent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IGeometry_put_Percent_Proxy( 
    IGeometry __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL newVal);


void __RPC_STUB IGeometry_put_Percent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IGeometry_get_Aspect_Proxy( 
    IGeometry __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IGeometry_get_Aspect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IGeometry_put_Aspect_Proxy( 
    IGeometry __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL newVal);


void __RPC_STUB IGeometry_put_Aspect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IGeometry_get_Greater_Proxy( 
    IGeometry __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IGeometry_get_Greater_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IGeometry_put_Greater_Proxy( 
    IGeometry __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL newVal);


void __RPC_STUB IGeometry_put_Greater_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IGeometry_get_Less_Proxy( 
    IGeometry __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IGeometry_get_Less_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IGeometry_put_Less_Proxy( 
    IGeometry __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL newVal);


void __RPC_STUB IGeometry_put_Less_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IGeometry_IsValid_Proxy( 
    IGeometry __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pVal);


void __RPC_STUB IGeometry_IsValid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IGeometry_Geometry_Proxy( 
    IGeometry __RPC_FAR * This,
    /* [in] */ BSTR PSSizeNick,
    /* [retval][out] */ BSTR __RPC_FAR *pVal);


void __RPC_STUB IGeometry_Geometry_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IGeometry_INTERFACE_DEFINED__ */


#ifndef __IColor_INTERFACE_DEFINED__
#define __IColor_INTERFACE_DEFINED__

/* interface IColor */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IColor;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("983302C5-A008-494A-A16A-D596EC1B38D1")
    IColor : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ColorMode( 
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetRGB( 
            double Red,
            double Green,
            double Blue) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetGray( 
            double Shade) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetMono( 
            VARIANT_BOOL WhitePixel) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetHSL( 
            double Hue,
            double Saturation,
            double Luminosity) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetYUV( 
            double Y,
            double U,
            double V) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IColorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IColor __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IColor __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IColor __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IColor __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IColor __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IColor __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IColor __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ColorMode )( 
            IColor __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetRGB )( 
            IColor __RPC_FAR * This,
            double Red,
            double Green,
            double Blue);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetGray )( 
            IColor __RPC_FAR * This,
            double Shade);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetMono )( 
            IColor __RPC_FAR * This,
            VARIANT_BOOL WhitePixel);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetHSL )( 
            IColor __RPC_FAR * This,
            double Hue,
            double Saturation,
            double Luminosity);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetYUV )( 
            IColor __RPC_FAR * This,
            double Y,
            double U,
            double V);
        
        END_INTERFACE
    } IColorVtbl;

    interface IColor
    {
        CONST_VTBL struct IColorVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IColor_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IColor_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IColor_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IColor_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IColor_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IColor_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IColor_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IColor_get_ColorMode(This,pVal)	\
    (This)->lpVtbl -> get_ColorMode(This,pVal)

#define IColor_SetRGB(This,Red,Green,Blue)	\
    (This)->lpVtbl -> SetRGB(This,Red,Green,Blue)

#define IColor_SetGray(This,Shade)	\
    (This)->lpVtbl -> SetGray(This,Shade)

#define IColor_SetMono(This,WhitePixel)	\
    (This)->lpVtbl -> SetMono(This,WhitePixel)

#define IColor_SetHSL(This,Hue,Saturation,Luminosity)	\
    (This)->lpVtbl -> SetHSL(This,Hue,Saturation,Luminosity)

#define IColor_SetYUV(This,Y,U,V)	\
    (This)->lpVtbl -> SetYUV(This,Y,U,V)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IColor_get_ColorMode_Proxy( 
    IColor __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pVal);


void __RPC_STUB IColor_get_ColorMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IColor_SetRGB_Proxy( 
    IColor __RPC_FAR * This,
    double Red,
    double Green,
    double Blue);


void __RPC_STUB IColor_SetRGB_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IColor_SetGray_Proxy( 
    IColor __RPC_FAR * This,
    double Shade);


void __RPC_STUB IColor_SetGray_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IColor_SetMono_Proxy( 
    IColor __RPC_FAR * This,
    VARIANT_BOOL WhitePixel);


void __RPC_STUB IColor_SetMono_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IColor_SetHSL_Proxy( 
    IColor __RPC_FAR * This,
    double Hue,
    double Saturation,
    double Luminosity);


void __RPC_STUB IColor_SetHSL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IColor_SetYUV_Proxy( 
    IColor __RPC_FAR * This,
    double Y,
    double U,
    double V);


void __RPC_STUB IColor_SetYUV_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IColor_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_MagickCOM_0212 */
/* [local] */ 




extern RPC_IF_HANDLE __MIDL_itf_MagickCOM_0212_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_MagickCOM_0212_v0_0_s_ifspec;

#ifndef __IImageCollection_INTERFACE_DEFINED__
#define __IImageCollection_INTERFACE_DEFINED__

/* interface IImageCollection */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IImageCollection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FB9C5C71-A094-4CCA-9CBE-470524DE3800")
    IImageCollection : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Add( 
            /* [in] */ IImage __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Remove( 
            /* [in] */ long n,
            /* [retval][out] */ IImage __RPC_FAR *__RPC_FAR *ppVal) = 0;
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_Count( 
            /* [retval][out] */ long __RPC_FAR *pnCount) = 0;
        
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_Item( 
            /* [in] */ long n,
            /* [retval][out] */ IImage __RPC_FAR *__RPC_FAR *ppItem) = 0;
        
        virtual /* [restricted][propget][id] */ HRESULT STDMETHODCALLTYPE get__NewEnum( 
            /* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppEnum) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IImageCollectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IImageCollection __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IImageCollection __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IImageCollection __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IImageCollection __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IImageCollection __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IImageCollection __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IImageCollection __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Add )( 
            IImageCollection __RPC_FAR * This,
            /* [in] */ IImage __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Remove )( 
            IImageCollection __RPC_FAR * This,
            /* [in] */ long n,
            /* [retval][out] */ IImage __RPC_FAR *__RPC_FAR *ppVal);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Count )( 
            IImageCollection __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pnCount);
        
        /* [helpstring][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Item )( 
            IImageCollection __RPC_FAR * This,
            /* [in] */ long n,
            /* [retval][out] */ IImage __RPC_FAR *__RPC_FAR *ppItem);
        
        /* [restricted][propget][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get__NewEnum )( 
            IImageCollection __RPC_FAR * This,
            /* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppEnum);
        
        END_INTERFACE
    } IImageCollectionVtbl;

    interface IImageCollection
    {
        CONST_VTBL struct IImageCollectionVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IImageCollection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IImageCollection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IImageCollection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IImageCollection_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IImageCollection_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IImageCollection_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IImageCollection_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IImageCollection_Add(This,pVal)	\
    (This)->lpVtbl -> Add(This,pVal)

#define IImageCollection_Remove(This,n,ppVal)	\
    (This)->lpVtbl -> Remove(This,n,ppVal)

#define IImageCollection_get_Count(This,pnCount)	\
    (This)->lpVtbl -> get_Count(This,pnCount)

#define IImageCollection_get_Item(This,n,ppItem)	\
    (This)->lpVtbl -> get_Item(This,n,ppItem)

#define IImageCollection_get__NewEnum(This,ppEnum)	\
    (This)->lpVtbl -> get__NewEnum(This,ppEnum)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IImageCollection_Add_Proxy( 
    IImageCollection __RPC_FAR * This,
    /* [in] */ IImage __RPC_FAR *pVal);


void __RPC_STUB IImageCollection_Add_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IImageCollection_Remove_Proxy( 
    IImageCollection __RPC_FAR * This,
    /* [in] */ long n,
    /* [retval][out] */ IImage __RPC_FAR *__RPC_FAR *ppVal);


void __RPC_STUB IImageCollection_Remove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget] */ HRESULT STDMETHODCALLTYPE IImageCollection_get_Count_Proxy( 
    IImageCollection __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pnCount);


void __RPC_STUB IImageCollection_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE IImageCollection_get_Item_Proxy( 
    IImageCollection __RPC_FAR * This,
    /* [in] */ long n,
    /* [retval][out] */ IImage __RPC_FAR *__RPC_FAR *ppItem);


void __RPC_STUB IImageCollection_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [restricted][propget][id] */ HRESULT STDMETHODCALLTYPE IImageCollection_get__NewEnum_Proxy( 
    IImageCollection __RPC_FAR * This,
    /* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppEnum);


void __RPC_STUB IImageCollection_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IImageCollection_INTERFACE_DEFINED__ */



#ifndef __MAGICKCOMLib_LIBRARY_DEFINED__
#define __MAGICKCOMLib_LIBRARY_DEFINED__

/* library MAGICKCOMLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_MAGICKCOMLib;

EXTERN_C const CLSID CLSID_Image;

#ifdef __cplusplus

class DECLSPEC_UUID("1FA85FFC-C245-4C1E-B121-00CA5EAB94F4")
Image;
#endif

EXTERN_C const CLSID CLSID_ImageControl;

#ifdef __cplusplus

class DECLSPEC_UUID("6CC4DF86-9661-4B00-A6A9-32C7787A8D07")
ImageControl;
#endif

EXTERN_C const CLSID CLSID_Geometry;

#ifdef __cplusplus

class DECLSPEC_UUID("F21700BE-8E79-40F4-B8C5-3038A9AA8430")
Geometry;
#endif

EXTERN_C const CLSID CLSID_Color;

#ifdef __cplusplus

class DECLSPEC_UUID("F4D71BC1-2606-4EB6-9E67-814D12D97CD7")
Color;
#endif

EXTERN_C const CLSID CLSID_ImageCollection;

#ifdef __cplusplus

class DECLSPEC_UUID("C110309D-B468-47DF-AA2F-415825F87FC8")
ImageCollection;
#endif
#endif /* __MAGICKCOMLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long __RPC_FAR *, unsigned long            , BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long __RPC_FAR *, BSTR __RPC_FAR * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
