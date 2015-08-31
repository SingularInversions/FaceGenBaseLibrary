// MagickImage.h : Declaration of the CMagickImage

#ifndef __MAGICKIMAGE_H_
#define __MAGICKIMAGE_H_

#include "resource.h"       // main symbols
#include <asptlb.h>         // Active Server Pages Definitions
#include <mtx.h>

// Needed for stdio FILE
#include <stdio.h>

// Needed for time_t
#include <time.h>

// Needed for doing direct memory allocation
//#include <malloc.h>

// Needed for off_t on Unix and Windows
#include <sys/types.h>

// Needed by the convert and combine code
#include <math.h>

// Forward declarations for iostream classes
#include <iosfwd>

// Let's put in the vector class to hold lists of args
//#pragma warning(disable : 4530) // stop warning about exception handling
#include <vector>

#undef PACKAGE
#undef VERSION

#include <magick/ImageMagick.h>
#include <nt-base.h>
#undef inline // Remove possible definition from config.h

#undef class

#undef setjmp
#undef longjmp
#include <setjmpex.h>

typedef std::vector<CComVariant> VarVector;
typedef CComEnumOnSTL<IEnumVARIANT, &IID_IEnumVARIANT, VARIANT, _Copy<VARIANT>, VarVector> VarEnum;
typedef ICollectionOnSTLImpl<IMagickImage, VarVector, VARIANT, _Copy<VARIANT>, VarEnum> IMagickImageCollection;
typedef IDispatchImpl<IMagickImageCollection, &IID_IMagickImage, &LIBID_IMAGEMAGICKOBJECTLib> _baseMagickImage;

const TCHAR chQuote = _T('\'');
const TCHAR chEquals = _T('=');
const TCHAR chRightBracket = _T('}');
const TCHAR chLeftBracket = _T('{');

/////////////////////////////////////////////////////////////////////////////
// CMagickImage
class ATL_NO_VTABLE CMagickImage : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CMagickImage, &CLSID_MagickImage>,
	public IObjectWithSiteImpl<CMagickImage>,
	public ISupportErrorInfo,
	public IConnectionPointContainerImpl<CMagickImage>,
	public IDispatchImpl<IMagickImageCollection, &IID_IMagickImage, &LIBID_IMAGEMAGICKOBJECTLib>
{
public:
	CMagickImage();
	~CMagickImage();

public:

DECLARE_REGISTRY_RESOURCEID(IDR_MAGICKIMAGE)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CMagickImage)
	COM_INTERFACE_ENTRY(IMagickImage)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
	COM_INTERFACE_ENTRY(IConnectionPointContainer)
	COM_INTERFACE_ENTRY(IObjectWithSite)
END_COM_MAP()

BEGIN_CONNECTION_POINT_MAP(CMagickImage)
END_CONNECTION_POINT_MAP()


// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

// IMagickImage
public:
	STDMETHOD(Convert)(/*[in,out]*/ SAFEARRAY **pArrayVar, /*[out, retval]*/ VARIANT *pVar2);
	STDMETHOD(Composite)(/*[in,out]*/ SAFEARRAY **pArrayVar, /*[out, retval]*/ VARIANT *pVar2);
	STDMETHOD(Mogrify)(/*[in,out]*/ SAFEARRAY **pArrayVar, /*[out, retval]*/ VARIANT *pVar2);
	STDMETHOD(Montage)(/*[in,out]*/ SAFEARRAY **pArrayVar, /*[out, retval]*/ VARIANT *pVar2);
	STDMETHOD(Identify)(/*[in,out]*/ SAFEARRAY **pArrayVar, /*[out, retval]*/ VARIANT *pVar2);
	STDMETHOD(Remove)(/*[in]*/ VARIANT varIndex);
	STDMETHOD(Add)(/*[in,out]*/ SAFEARRAY **pArrayVar, /*[out, retval]*/ VARIANT *pVar2);
  // ICollectionOnSTLImpl<> supplies these, but we override and pass through
	STDMETHOD(get_Item)(/*[in]*/ VARIANT varIndex, /*[out, retval]*/ VARIANT *pVar);
	STDMETHOD(get__NewEnum)(/*[out, retval]*/ LPUNKNOWN *pVal);
	STDMETHOD(get_Count)(/*[out, retval]*/ long *pVal);
	STDMETHOD(get_Messages)(/*[out, retval]*/ VARIANT *pVar);
	STDMETHOD(OnStartPage)(IUnknown* IUnk);
	STDMETHOD(OnEndPage)();
  // Utility routines not called by COM interfaces
  HRESULT AllocateArgs(int iArgs);
  HRESULT ReAllocateArgs(int iArgs);
  void DeleteArgs(void);
  char **GetArgv(void);
  char **GetArgvT(void);
  int GetArgc(void);
  void EmptyArgs(void);
  HRESULT AddArgs(VARIANTARG *arg);
  HRESULT AddArgs(BSTR arg);
  HRESULT AddArgs(LPTSTR arg);
  HRESULT ProcessArgs(int iFunction,SAFEARRAY **pArrayVar);
  HRESULT DispatchToImage(IDispatch* pdisp,CComObject<CMagickImage>** ppMagickImage);
  HRESULT UnknownToImage(IUnknown* pdisp,CComObject<CMagickImage>** ppMagickImage);
  HRESULT Execute(unsigned int (*func)(ImageInfo *image_info,
    const int argc,char **argv,char **text,ExceptionInfo *exception),
      char **text,ImageInfo *info,ExceptionInfo *exception);
	HRESULT Perform(unsigned int (*func)(ImageInfo *image_info,
    const int argc,char **argv,char **text,ExceptionInfo *exception),
      SAFEARRAY **pArrayVar,VARIANT *pVar2,ExceptionInfo *exception);
  LPTSTR StrChr(LPTSTR lpsz, TCHAR ch);
public:
	CComPtr<IRequest> m_piRequest;					//Request Object
	CComPtr<IResponse> m_piResponse;				//Response Object
	CComPtr<ISessionObject> m_piSession;			//Session Object
	CComPtr<IServer> m_piServer;					//Server Object
	CComPtr<IApplicationObject> m_piApplication;	//Application Object
	BOOL m_bOnStartPageCalled;						//OnStartPage successful?
	BOOL m_bOnFirstTime;						//Flag to indicate first call?

private:
  char **m_argv;
  char **m_argv_t;
  int m_argc;
  int m_argvIndex;
	LPTSTR m_pchCur;

  char warning_text[MaxTextExtent];
  char error_text[MaxTextExtent];
  int error_number;

  static void warninghandler(const ExceptionType warning,const char *message,
    const char *qualifier);

  static void errorhandler(const ExceptionType error,const char *message,
    const char *qualifier);

  static void fatalerrorhandler(const ExceptionType error,const char *message,
    const char *qualifier);
};

const int nDefaultArgumentSize = 128;

//
// This routine convert a wide character string to ANSI and stores the new
// string in a buffer allocated by the new operator
//
#define MAKE_WIDEPTR_FROMANSI(ptrname, ansistr) \
    long __l##ptrname = (lstrlen(ansistr) + 1) * sizeof(WCHAR); \
    void * __TempBuffer##ptrname = (void *) new char [ __l##ptrname ]; \
    MultiByteToWideChar(CP_ACP, 0, ansistr, -1, (LPWSTR)__TempBuffer##ptrname.GetBuffer(), __l##ptrname); \
    LPWSTR ptrname = (LPWSTR)__TempBuffer##ptrname

//
// This routine converts an ANSI string to a wide character string in a new
// buffer area allocated using the new operator
// We allocate lstrlenW(widestr) * 2 because its possible for a UNICODE char
// to map to 2 ansi characters this is a quick guarantee that enough space
// will be allocated.
//
#define MAKE_ANSIPTR_FROMWIDE(ptrname, widestr) \
    long __l##ptrname = (lstrlenW(widestr) + 1) * 2 * sizeof(char); \
    void * __TempBuffer##ptrname = (void *) new char [ __l##ptrname]; \
    WideCharToMultiByte(CP_ACP, 0, widestr, -1, (LPSTR)__TempBuffer##ptrname, __l##ptrname, NULL, NULL); \
    LPSTR ptrname = (LPSTR)__TempBuffer##ptrname

#define MAKE_ANSIPTR_FROMWIDE_2(ptrname, widestr, wlen) \
    long __l##ptrname = (wlen + 1) * 2 * sizeof(char); \
    void * __TempBuffer##ptrname = (void *) new char [ __l##ptrname]; \
    WideCharToMultiByte(CP_ACP, 0, widestr, -1, (LPSTR)__TempBuffer##ptrname, __l##ptrname, NULL, NULL); \
    LPSTR ptrname = (LPSTR)__TempBuffer##ptrname
//
// This routine is used to make a copy of an existing ANSI string
//
#define MAKE_COPY_OF_ANSI(ptrname, ansistr) \
    long __l##ptrname = (lstrlen(ansistr) + 1); \
    void * __TempBuffer##ptrname = (void *) new char [ __l##ptrname ]; \
    lstrcpyn((LPSTR)__TempBuffer##ptrname, (LPCSTR)ansistr, __l##ptrname); \
    LPSTR ptrname = (LPSTR)__TempBuffer##ptrname

#endif //__MAGICKIMAGE_H_
