// MagickImage.cpp : Implementation of CMagickImage
#include "stdafx.h"
#include "ImageMagickObject.h"
#include "MagickImage.h"
#  include "comvector.h"

const LCID lcidDefault = 0;
const DWORD dwErrorBase = 5000;

/////////////////////////////////////////////////////////////////////////////
// CMagickImage

CMagickImage::CMagickImage()
{
  DebugString("ImageMagickObject - new\n");
  SetWarningHandler(warninghandler);
  SetErrorHandler(errorhandler);
  SetFatalErrorHandler(fatalerrorhandler);
  m_bOnStartPageCalled = FALSE;
	m_bOnFirstTime = FALSE;
  AllocateArgs( nDefaultArgumentSize );
}

CMagickImage::~CMagickImage() 
{
	DebugString("ImageMagickObject - delete\n");
  DeleteArgs();
}

STDMETHODIMP CMagickImage::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IMagickImage,
	};
	for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

STDMETHODIMP CMagickImage::OnStartPage (IUnknown* pUnk)  
{
  DebugString("ImageMagickObject - OnStartPage\n");
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
    DebugString("ImageMagickObject - OnStartPage get Request failed\n");
		//spContext.Release();
		//return hr;
	}

	// Get Response Object Pointer
	hr = spContext->get_Response(&m_piResponse);
	if(FAILED(hr))
	{
    DebugString("ImageMagickObject - OnStartPage get Response failed\n");
		//m_piRequest.Release();
		//return hr;
	}
	
	// Get Server Object Pointer
	hr = spContext->get_Server(&m_piServer);
	if(FAILED(hr))
	{
    DebugString("ImageMagickObject - OnStartPage get Server failed\n");
		//m_piRequest.Release();
		//m_piResponse.Release();
		//return hr;
	}
	
	// Get Session Object Pointer
	hr = spContext->get_Session(&m_piSession);
	if(FAILED(hr))
	{
    DebugString("ImageMagickObject - OnStartPage get Session failed\n");
		//m_piRequest.Release();
		//m_piResponse.Release();
		//m_piServer.Release();
		//return hr;
	}

	// Get Application Object Pointer
	hr = spContext->get_Application(&m_piApplication);
	if(FAILED(hr))
	{
    DebugString("ImageMagickObject - OnStartPage get Application failed\n");
		//m_piRequest.Release();
		//m_piResponse.Release();
		//m_piServer.Release();
		//m_piSession.Release();
		//eturn hr;
	}
	m_bOnStartPageCalled = TRUE;

  {
	  CComPtr<IRequestDictionary>pReadDictionary;
	  CComPtr<IReadCookie>pCookieDictionary;
	  	  
		hr=m_piRequest->get_Cookies(&pReadDictionary);
		if(SUCCEEDED(hr))
    {
		  CComVariant vtIn(_T("MAGICK_DEBUG"));
		  CComVariant vtKey(_T("level"));
		  CComVariant vtOut;
		  CComVariant vtCookieValue;

		  hr=pReadDictionary->get_Item(vtIn,&vtOut);
		  if(SUCCEEDED(hr) && (V_VT(&vtOut)==VT_DISPATCH))
      {
        pCookieDictionary = (IReadCookie*)(vtOut.pdispVal);
		    hr=pCookieDictionary->get_Item(vtKey,&vtCookieValue);
		    if(SUCCEEDED(hr) && (V_VT(&vtCookieValue)==VT_BSTR))
        {
          USES_CONVERSION;
          LPTSTR lpstrVal = W2T(vtCookieValue.bstrVal);
          int level = atoi(lpstrVal);
          DebugLevel(level);
	        DebugString("OnStartPage debug level: %d\n",level);
        }
        else
	        DebugString("OnStartPage - parse error\n");
      }
      else
	      DebugString("OnStartPage - no MAGICK_DEBUG\n");
    }
    else
	    DebugString("OnStartPage - no cookies\n");
  }
	return S_OK;
}

STDMETHODIMP CMagickImage::OnEndPage ()  
{
  DebugString("ImageMagickObject - OnEndPage\n");
	m_bOnStartPageCalled = FALSE;
	// Release all interfaces
  if (m_piRequest)
	  m_piRequest.Release();
  if (m_piResponse)
	  m_piResponse.Release();
  if (m_piServer)
	  m_piServer.Release();
  if (m_piSession)
	  m_piSession.Release();
  if (m_piApplication)
	  m_piApplication.Release();
	return S_OK;
}

// ICollectionOnSTLImpl<> supplies these, but we override and pass through
STDMETHODIMP CMagickImage::get_Count(long *pVal)
{
  HRESULT hr = _baseMagickImage::get_Count(pVal);
	return hr;
}

STDMETHODIMP CMagickImage::get__NewEnum(LPUNKNOWN *pVal)
{
  HRESULT hr = _baseMagickImage::get__NewEnum(pVal);
	return hr;
}

STDMETHODIMP CMagickImage::get_Item(VARIANT varIndex, VARIANT *pVal)
{
  USES_CONVERSION;

  HRESULT hr = E_INVALIDARG;
  VARIANTARG *pvarIndex = &varIndex;
  VARTYPE vt = V_VT(pvarIndex);
  long lIndex = 0;
  CComVariant var;
 
  while (vt == (VT_VARIANT|VT_BYREF))
  {
	  pvarIndex = V_VARIANTREF(pvarIndex);
	  vt = V_VT(pvarIndex);
  }
  if (V_ISARRAY(pvarIndex))
    return hr;
  if ((vt & ~VT_BYREF) == VT_BSTR)
    {
      LPTSTR lpszNext;
      LPTSTR lpszVal = W2T(V_BSTR(pvarIndex));
      var = _T("");
      if (lpszVal)
        {
          Image
            *image;

          ImageInfo
            *image_info;

          ExceptionInfo
            exception;

          long
            id;

				  lpszNext = StrChr(lpszVal, _T('.'));
				  if (lpszNext == NULL)
            lpszNext = _T("%w,%h,%m");
          else
            *lpszNext++ = _T('\0');
          // lookup the registry id using token and pass the image in
          GetExceptionInfo(&exception);
          image=GetImageFromMagickRegistry(lpszVal, &id, &exception);
          if (image != (Image *) NULL)
            {
              TCHAR *text;

              image_info=CloneImageInfo((ImageInfo *) NULL);                
              text=TranslateText(image_info,image,lpszNext);
              DestroyImageList(image);
              DestroyImageInfo(image_info);
              var = text;
              LiberateMemory((void **) &text);
              hr = S_OK;
            }
        }
    }
  else
    {
      Image
        *image;

      ImageInfo
        *image_info;

      ExceptionInfo
        exception;

      long
        id;

      RegistryType
        type;

      size_t
        length;

      id = 0;
      var = _T("");
      switch(vt & ~VT_BYREF)
      {
        case VT_UI1:
          id = V_UI1(pvarIndex);
          break;
        case VT_I2:
          id = V_I2(pvarIndex);
          break;
        case VT_I4:
          id = V_I4(pvarIndex);
          break;
      }
      // lookup the registry id using token and pass the image in
      GetExceptionInfo(&exception);
      image=(Image *)GetMagickRegistry(id,&type,&length,&exception);
      if (image != (Image *) NULL)
        {
          TCHAR *text;

          image_info=CloneImageInfo((ImageInfo *) NULL);                
          text=TranslateText(image_info,image,_T("%w,%h,%m"));
          DestroyImageList(image);
          DestroyImageInfo(image_info);
          var = text;
          LiberateMemory((void **) &text);
          hr = S_OK;
        }
    }
  var.Detach(pVal);
	return hr;
}

STDMETHODIMP CMagickImage::Remove(VARIANT varIndex)
{
  USES_CONVERSION;

  HRESULT hr = E_INVALIDARG;
  VARIANTARG *pvarIndex = &varIndex;
  VARTYPE vt = V_VT(pvarIndex);
 
  while (vt == (VT_VARIANT|VT_BYREF))
  {
	  pvarIndex = V_VARIANTREF(pvarIndex);
	  vt = V_VT(pvarIndex);
  }
  if (V_ISARRAY(pvarIndex))
    return hr;
  switch(vt & ~VT_BYREF)
  {
    case VT_BSTR:
    {
      if (!V_ISBYREF(pvarIndex))
      {
        CComVariant var;
        LPTSTR lpszNext;
        LPTSTR lpszVal = W2T(V_BSTR(pvarIndex));
        var = _T("");
        if (lpszVal)
          {
            Image
              *image;

            ExceptionInfo
              exception;

            long
              id;

				    lpszNext = StrChr(lpszVal, _T('.'));
				    if (lpszNext == NULL)
              lpszNext = _T("%w,%h,%m");
            else
              *lpszNext++ = _T('\0');
            // lookup the registry id using token and pass the image in
            GetExceptionInfo(&exception);
            image=GetImageFromMagickRegistry(lpszVal, &id, &exception);
            if (image != (Image *) NULL)
              {
                DestroyImageList(image);
                if (DeleteMagickRegistry(id))
                  hr = S_OK;
              }
          }
      }
      break;
    }
    case VT_UI1:
    {
      long id = V_UI1(pvarIndex);
      if (DeleteMagickRegistry(id))
        hr = S_OK;
    }
    case VT_I2:
    {
      long id = V_I2(pvarIndex);
      if (DeleteMagickRegistry(id))
        hr = S_OK;
    }
    case VT_I4:
    {
      long id = V_I4(pvarIndex);
      if (DeleteMagickRegistry(id))
        hr = S_OK;
    }
  }
	return hr;
}

STDMETHODIMP CMagickImage::Add(SAFEARRAY **pArrayVar, VARIANT *pVar)
{
  return S_OK;
}

#include <exdisp.h>

static long SafeArraySize(SAFEARRAY *psa)
{
  HRESULT hr;
	long lBoundl, lBoundu;

	hr = ::SafeArrayGetLBound(psa, 1, &lBoundl);
  if (FAILED(hr))
    return 0;
	hr = ::SafeArrayGetUBound(psa, 1, &lBoundu);
  if (FAILED(hr))
    return 0;
  return (lBoundu - lBoundl + 1);
}

#ifdef USETHIS_CODE
int COMMagickFifoBuffer(const Image *image,const void *data,const size_t length)
{
  SAFEARRAYBOUND NewArrayBounds[1];  // 1 Dimension
  size_t tlen=length;
  CMagickImage *pMagickImage = (CMagickImage *)image->client_data;
  if ((pMagickImage != NULL) && (pMagickImage->pSafeArray != NULL))
  {
	  DWORD dwSizeOfChunk;
	  unsigned char	*pReturnBuffer = NULL;
	  HRESULT hr = S_OK;
		long lBoundl, lBoundu, lCount;

	  dwSizeOfChunk = (DWORD)length;
    /* is this the signal that says we are all done? */
    if ((dwSizeOfChunk == 0) && (data == (void *) NULL))
    {
      if (pMagickImage->m_bOnFirstTime == FALSE)
        {
          //pMagickImage->m_piResponse->End();
        }
    }
	  if ((dwSizeOfChunk == 0) || (dwSizeOfChunk == 0xFFFFFFFF))
		  return tlen;

		hr = ::SafeArrayGetLBound(pMagickImage->pSafeArray, 1, &lBoundl);
    if (FAILED(hr))
      return tlen;
		hr = ::SafeArrayGetUBound(pMagickImage->pSafeArray, 1, &lBoundu);
    if (FAILED(hr))
      return tlen;

		lCount = lBoundu - lBoundl + 1;
    NewArrayBounds[0].lLbound = 0;   // Start-Index 0
    NewArrayBounds[0].cElements = dwSizeOfChunk+lCount;  // # Elemente
    hr = SafeArrayRedim(pMagickImage->pSafeArray, NewArrayBounds);
    if (FAILED(hr))
      return tlen;

    hr = SafeArrayAccessData(pMagickImage->pSafeArray, (void**)&pReturnBuffer);
	  if( FAILED(hr) )
		  return tlen;
	  memcpy( pReturnBuffer, (unsigned char *)data+lCount, dwSizeOfChunk );
    hr = SafeArrayUnaccessData(pMagickImage->pSafeArray);
	  if( FAILED(hr) )
		  return tlen;
  }
  return(tlen);
}
#endif

HRESULT CMagickImage::DispatchToImage(IDispatch* pdisp,CComObject<CMagickImage>** ppMagickImage)
{
  // Given an IDispatch*, convert it (if possible) to a CComObject<CMagickImage>*
  IMagickImage* pinterface;
  if(FAILED(pdisp->QueryInterface(IID_IMagickImage,
        reinterpret_cast<void**>(&pinterface))))
  {
    return DISP_E_TYPEMISMATCH;
  }
  *ppMagickImage = static_cast<CComObject<CMagickImage>*>(pinterface);
  pinterface->Release();
  return S_OK;
}

HRESULT CMagickImage::UnknownToImage(IUnknown* punk,CComObject<CMagickImage>** ppMagickImage)
{
  // Given an IUnknown*, convert it (if possible) to a CComObject<CMagickImage>*
  IMagickImage* pinterface;
  if(FAILED(punk->QueryInterface(IID_IMagickImage,
        reinterpret_cast<void**>(&pinterface))))
  {
    return DISP_E_TYPEMISMATCH;
  }
  *ppMagickImage = static_cast<CComObject<CMagickImage>*>(pinterface);
  pinterface->Release();
  return S_OK;
}

STDMETHODIMP CMagickImage::get_Messages(VARIANT *pVar)
{
  HRESULT hr = S_OK;

  if (m_coll.size())
  {
    CComVector<VARIANT> v(m_coll.size());
    if( !v )
    {
      //m_coll.clear();
      return E_OUTOFMEMORY;
    }
    else
    {
      // WARNING: This nested code block is required because
      // CComVectorData ctor performs a SafeArrayAccessData
      // and you have to make sure the SafeArrayUnaccessData
      // is called (which it is in the CComVectorData dtor)
      // before you use the CComVector::DetachTo(...).
      CComVectorData<VARIANT> msgs(v);
      if( !msgs )
      {
        //m_coll.clear();
        return E_OUTOFMEMORY;
      }
      else
      {
        for(int index = 0; index < m_coll.size(); ++index)
        {
          CComVariant vt(m_coll[index]);
          HRESULT hr = vt.Detach(&msgs[index]);
        }
      }
    }    
    V_VT(pVar) = VT_ARRAY | VT_VARIANT;
    V_ARRAY(pVar) = v.Detach();
    //m_coll.clear();
  }
  return hr;
}

#ifdef SUPPORT_OBJECTS
HRESULT CMagickImage::FormatRequest(BSTR *pVal)
{
  HRESULT hr = E_INVALIDARG;
  TCHAR sz[128];
  CComBSTR bstrResult;

  if (m_bstrIOType == L"xtrnfile")
  {
    wsprintf(sz, _T("xtrnfile:%S"),m_bstrName);
    bstrResult = sz;
    bstrResult.CopyTo(pVal);
    hr = S_OK;
  }
  else if (m_bstrIOType == L"xtrnimage")
  {
    wsprintf(sz, _T("xtrnimage:0x%lx,0x%lx"),
      (unsigned long)(&pImage_info),(unsigned long)(&pImage));
    bstrResult = sz;
    bstrResult.CopyTo(pVal);
    hr = S_OK;
  }
  else if (m_bstrIOType == L"xtrnblob")
  {
    wsprintf(sz, _T("xtrnblob:0x%lx,0x%lx,%S"),
      (unsigned long)(&pBlob_data),(unsigned long)(&iBlob_length),
        m_bstrName);
    bstrResult = sz;
    bstrResult.CopyTo(pVal);
    hr = S_OK;
  }
  else if (m_bstrIOType == L"xtrnstream")
  {
    wsprintf(sz, _T("xtrnstream:0x%lx,0x%lx,%S"),
      (unsigned long)(&COMMagickFifoBuffer),(unsigned long)this,m_bstrName);
    bstrResult = sz;
    bstrResult.CopyTo(pVal);
    hr = S_OK;
  }
  else if (m_bstrIOType == L"xtrnarray")
  {
    wsprintf(sz, _T("xtrnarray:0x%lx,%S"),
      (unsigned long)(pSafeArray), m_bstrName);
    bstrResult = sz;
    bstrResult.CopyTo(pVal);
    hr = S_OK;
  }
	return hr;
}
#endif

static char *translate_exception(DWORD code)
{
  switch (code)
  {
    case EXCEPTION_ACCESS_VIOLATION:
      return "access violation";
    case EXCEPTION_DATATYPE_MISALIGNMENT:
      return "data misalignment";
    case EXCEPTION_BREAKPOINT:
      return "debug breakpoint";
    case EXCEPTION_SINGLE_STEP:
      return "debug single step";
    case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
      return "array out of bounds";
    case EXCEPTION_FLT_DENORMAL_OPERAND:
      return "float denormal operand";
    case EXCEPTION_FLT_DIVIDE_BY_ZERO:
      return "float devide by zero";
    case EXCEPTION_FLT_INEXACT_RESULT:
      return "float inexact result";
    case EXCEPTION_FLT_INVALID_OPERATION:
      return "float invalid operation";
    case EXCEPTION_FLT_OVERFLOW:
      return "float overflow";
    case EXCEPTION_FLT_STACK_CHECK:
      return "float stack check";
    case EXCEPTION_FLT_UNDERFLOW:
      return "float underflow";
    case EXCEPTION_INT_DIVIDE_BY_ZERO:
      return "integer divide by zero";
    case EXCEPTION_INT_OVERFLOW:
      return "integer overflow";
    case EXCEPTION_PRIV_INSTRUCTION:
      return "privleged instruction";
    case EXCEPTION_IN_PAGE_ERROR:
      return "page error";
    case EXCEPTION_ILLEGAL_INSTRUCTION:
      return "illegal instruction";
    case EXCEPTION_NONCONTINUABLE_EXCEPTION:
      return "noncontinuable instruction";
    case EXCEPTION_STACK_OVERFLOW:
      return "stack overflow";
    case EXCEPTION_INVALID_DISPOSITION:
      return "invalid disosition";
    case EXCEPTION_GUARD_PAGE:
      return "guard page";
    case EXCEPTION_INVALID_HANDLE:
      return "invalid handle";
    default:
      return "operating system exception";
  }
}

STDMETHODIMP CMagickImage::Convert(SAFEARRAY **pArrayVar, VARIANT *pVar)
{
  USES_CONVERSION;

  HRESULT hr;

  ExceptionInfo
    exception;

  char
    *reason,
    *description,
    message_text[MaxTextExtent];

  __try
  {
    EmptyArgs();
    AddArgs(L"-convert");
    GetExceptionInfo(&exception);
    reason = "unknown";
    description = "unknown";
    hr = Perform(ConvertImageCommand,pArrayVar,pVar,&exception);
    if (FAILED(hr))
    {
      if (exception.reason)
        reason = exception.reason;
      if (exception.description)
        description = exception.description;
    }
  }
  __except(1)
  {
    hr = E_UNEXPECTED;
    reason = "exception";
    description = translate_exception(_exception_code());
  }
	if (FAILED(hr))
    {
      hr = MAKE_HRESULT(SEVERITY_ERROR,FACILITY_ITF,dwErrorBase+1001);
      (void) FormatMagickString(message_text,MaxTextExtent,
        "convert: %d: %.1024s: %.1024s",exception.severity,reason,description);
      Error(A2W(message_text),IID_IMagickImage,hr);
    }
  DestroyExceptionInfo(&exception);
  return hr;
}

STDMETHODIMP CMagickImage::Composite(SAFEARRAY **pArrayVar, VARIANT *pVar)
{
  USES_CONVERSION;

  HRESULT hr;

  ExceptionInfo
    exception;

  char
    *reason,
    *description,
    message_text[MaxTextExtent];

  __try
  {
    EmptyArgs();
    AddArgs(L"-convert");
    GetExceptionInfo(&exception);
    reason = "unknown";
    description = "unknown";
    hr = Perform(CompositeImageCommand,pArrayVar,pVar,&exception);
    if (FAILED(hr))
    {
      if (exception.reason)
        reason = exception.reason;
      if (exception.description)
        description = exception.description;
    }
  }
  __except(1)
  {
    hr = E_UNEXPECTED;
    reason = "exception";
    description = translate_exception(_exception_code());
  }
	if (FAILED(hr))
    {
      hr = MAKE_HRESULT(SEVERITY_ERROR,FACILITY_ITF,dwErrorBase+1001);
      (void) FormatMagickString(message_text,MaxTextExtent,
        "composite: %d: %.1024s: %.1024s",exception.severity,reason,
        description);
      Error(A2W(message_text),IID_IMagickImage,hr);
    }
  DestroyExceptionInfo(&exception);
  return hr;
}

STDMETHODIMP CMagickImage::Mogrify(SAFEARRAY **pArrayVar, VARIANT *pVar)
{
  USES_CONVERSION;

  HRESULT hr;

  ExceptionInfo
    exception;

  char
    *reason,
    *description,
    message_text[MaxTextExtent];

  __try
  {
    EmptyArgs();
    AddArgs(L"-convert");
    GetExceptionInfo(&exception);
    reason = "unknown";
    description = "unknown";
    hr = Perform(MogrifyImageCommand,pArrayVar,pVar,&exception);
    if (FAILED(hr))
    {
      if (exception.reason)
        reason = exception.reason;
      if (exception.description)
        description = exception.description;
    }
  }
  __except(1)
  {
    hr = E_UNEXPECTED;
    reason = "exception";
    description = translate_exception(_exception_code());
  }
	if (FAILED(hr))
    {
      hr = MAKE_HRESULT(SEVERITY_ERROR,FACILITY_ITF,dwErrorBase+1001);
      (void) FormatMagickString(message_text,MaxTextExtent,
        "mogrify: %d: %.1024s: %.1024s",exception.severity,reason,description);
      Error(A2W(message_text),IID_IMagickImage,hr);
    }
  DestroyExceptionInfo(&exception);
  return hr;
}

STDMETHODIMP CMagickImage::Montage(SAFEARRAY **pArrayVar, VARIANT *pVar)
{
  USES_CONVERSION;

  HRESULT hr;

  ExceptionInfo
    exception;

  char
    *reason,
    *description,
    message_text[MaxTextExtent];

  __try
  {
    EmptyArgs();
    AddArgs(L"-convert");
    GetExceptionInfo(&exception);
    reason = "unknown";
    description = "unknown";
    hr = Perform(MontageImageCommand,pArrayVar,pVar,&exception);
    if (FAILED(hr))
    {
      if (exception.reason)
        reason = exception.reason;
      if (exception.description)
        description = exception.description;
    }
  }
  __except(1)
  {
    hr = E_UNEXPECTED;
    reason = "exception";
    description = translate_exception(_exception_code());
  }
	if (FAILED(hr))
    {
      hr = MAKE_HRESULT(SEVERITY_ERROR,FACILITY_ITF,dwErrorBase+1001);
      (void) FormatMagickString(message_text,MaxTextExtent,
        "montage: %d: %.1024s: %.1024s",exception.severity,reason,description);
      Error(A2W(message_text),IID_IMagickImage,hr);
    }
  DestroyExceptionInfo(&exception);
  return hr;
}

STDMETHODIMP CMagickImage::Identify(SAFEARRAY **pArrayVar, VARIANT *pVar)
{
  USES_CONVERSION;

  HRESULT hr;

  ExceptionInfo
    exception;

  char
    *reason,
    *description,
    message_text[MaxTextExtent];

  __try
  {
    EmptyArgs();
    AddArgs(L"-convert");
    GetExceptionInfo(&exception);
    reason = "unknown";
    description = "unknown";
    hr = Perform(IdentifyImageCommand,pArrayVar,pVar,&exception);
    if (FAILED(hr))
    {
      if (exception.reason)
        reason = exception.reason;
      if (exception.description)
        description = exception.description;
    }
  }
  __except(1)
  {
    hr = E_UNEXPECTED;
    reason = "exception";
    description = translate_exception(_exception_code());
  }
	if (FAILED(hr))
    {
      hr = MAKE_HRESULT(SEVERITY_ERROR,FACILITY_ITF,dwErrorBase+1001);
      (void) FormatMagickString(message_text,MaxTextExtent,
        "identify: %d: %.1024s: %.1024s",exception.severity,reason,description);
      Error(A2W(message_text),IID_IMagickImage,hr);
    }
  DestroyExceptionInfo(&exception);
  return hr;
}

const char *objName = "ImageMagickObject";

#define ThrowPerformException(exception,code,reason,description) \
{ \
	DebugString("%s - %s %s\n",objName,reason,description); \
  ThrowException(exception,code,reason,description); \
  return E_INVALIDARG; \
}

#define LogInformation(reason,description) \
{ \
	DebugString("%s - %s %s\n",objName,reason,description); \
}

const char *methodName = "Perform";

HRESULT CMagickImage::Perform(unsigned int (*func)(ImageInfo *image_info,
  const int argc,LPTSTR *argv,LPTSTR *text,ExceptionInfo *exception),
  SAFEARRAY **pArrayVar, VARIANT *pVar,ExceptionInfo *exception)
{
  USES_CONVERSION;

  bool bDebug = false;
  HRESULT hr = E_INVALIDARG;
  char *text;

#ifdef _DEBUG
  //_DbgBreak();
#endif

	LogInformation(methodName,"enter");

  //text = (char *) 0xffffffffL;
  //char c = *text;  // causes and access violation

  text = (char *)NULL;
  m_coll.clear();
#ifdef SUPPORT_OBJECTS
  CComObject<CMagickImage>* pMagickImage;
#endif

  if( !pArrayVar ) 
  {
    ThrowPerformException(exception,ErrorException,
      "Perform","Argument list is NULL");
  }

  CComVectorData<VARIANT> rg(*pArrayVar);
  if( !rg ) 
  {
    ThrowPerformException(exception,ErrorException,
      "Perform","Argument list is bad");
  }

  int iLastVal = rg.Length();
  bool bFoundOption = false;
  for( int i = 0; i < iLastVal; ++i )
  {
    VARIANT &varIndex = rg[i];
    VARIANTARG *pvarIndex = &varIndex;
    VARTYPE vt = V_VT(pvarIndex);

    while (vt == (VT_VARIANT|VT_BYREF))
    {
	    pvarIndex = V_VARIANTREF(pvarIndex);
	    vt = V_VT(pvarIndex);
    }
//->
    if (V_ISARRAY(pvarIndex))
    {
      TCHAR sz[128];
      SAFEARRAY *psa;

      if (V_ISBYREF(pvarIndex))
	        psa = *V_ARRAYREF(pvarIndex);
      else
	        psa = V_ARRAY(pvarIndex);
//----->
        {
//------->
          if (psa)
            {
              VARTYPE vatype = (V_VT(pvarIndex) & ~(VT_ARRAY | VT_BYREF));
              int ndim = SafeArrayGetDim(psa);
              if (ndim != 1)
              {
                ThrowPerformException(exception,ErrorException,
                  "Perform","Multi-dimensional arrays not supported");
              }
              if (i < (iLastVal-1))
//------------>
               {
                  bool bFoundIt = false;
                  // This is any array that is not the last one in the arg
                  // list. This means it must be an input so we just pass
                  // it along.
                  switch(vatype)
//--------------->
                  {
                    case VT_UI1:
                    {
                      wsprintf(sz, _T("xtrnarray:0x%lx,"),(unsigned long)(psa));
                      hr = AddArgs(sz);
                      break;
                    }
                    default:
//----------------->
                    {
                      CComVectorData<VARIANT> vals(psa);
                      if (vals)
//--------------------->
                        {
                          VARIANT &varFirst = vals[0];
                          VARIANTARG *pvarFirst = &varFirst;
                          if (V_VT(pvarFirst) ==  VT_BSTR)
//------------------------->
                            {
                              VARIANT &varSecond = vals[1];
                              VARIANTARG *pvarSecond = &varSecond;
                              if (V_ISARRAY(pvarSecond))
//--------------------------->
                              {
                                if (V_ISBYREF(pvarSecond))
                                  {
                                    VARTYPE vatype2 = (V_VT(pvarSecond) & ~(VT_ARRAY | VT_BYREF));
                                    if (vatype2 == VT_UI1)
                                      {
	                                      SAFEARRAY *psax = *V_ARRAYREF(pvarSecond);
                                        int ndim2 = SafeArrayGetDim(psax);
                                        if (ndim2 != 1)
                                        {
                                          ThrowPerformException(exception,ErrorException,
                                            "Perform","Input blob support requires a 1d array (1)");
                                        }
                                        LPTSTR lpszVal2 = W2T(pvarFirst->bstrVal);
                                        wsprintf(sz, _T("xtrnarray:0x%lx,%s"),
                                          (unsigned long)(psax),lpszVal2);
                                        hr = AddArgs(sz);
                                      }
                                  }
                                else
                                  {
                                    VARTYPE vatype2 = (V_VT(pvarSecond) & ~(VT_ARRAY));
                                    if (vatype2 == VT_UI1)
                                      {
	                                      SAFEARRAY *psax = V_ARRAY(pvarSecond);
                                        int ndim2 = SafeArrayGetDim(psax);
                                        if (ndim2 != 1)
                                        {
                                          ThrowPerformException(exception,ErrorException,
                                            "Perform","Input blob support requires a 1d array (2)");
                                        }
                                        LPTSTR lpszVal2 = W2T(pvarFirst->bstrVal);
                                        wsprintf(sz, _T("xtrnarray:0x%lx,%s"),
                                          (unsigned long)(psax),lpszVal2);
                                        hr = AddArgs(sz);
                                      }
                                  }
//--------------------------->
                              } // end of V_ISARRAY
//------------------------->
                            } // end of == VT_BSTR
                          else
                            {
                              wsprintf(sz, _T("xtrnarray:0x%lx,"),(unsigned long)(psa));
                              hr = AddArgs(sz);
                            }
                          vals.UnaccessData();
                          break;
//--------------------->
                        } // end of vals not NULL
//----------------->
                    } // end of default case
//--------------->
                  } // end of the switch
//------------->
                }
              else
                {
                  // This is the last thing in the arg list and thus must
                  // the output array. We check the contents to a string of
                  // characters that says what format to encode the data in.
                  if (vatype == VT_UI1)
                    {
                      // the output is passed as an array of bytes - this
                      // is the way that VB does it.
                      LPCWSTR pReturnBuffer = NULL;
                      long size = SafeArraySize(psa);
                      long sizeneeded;
                      char *ptrANSI;
                      hr = SafeArrayAccessData(psa, (void**)&pReturnBuffer);
	                    if(SUCCEEDED(hr))
                        {
                          sizeneeded = WideCharToMultiByte(
                            CP_ACP, 0, pReturnBuffer, size/2, (LPSTR)NULL, 0, NULL, NULL);
                          if (sizeneeded)
                            {
                              ptrANSI = new char [sizeneeded + 1];
                              WideCharToMultiByte(
                                CP_ACP, 0, pReturnBuffer, size, (LPSTR)ptrANSI, sizeneeded, NULL, NULL);
                              ptrANSI[sizeneeded]='\0';
                              hr = SafeArrayUnaccessData(psa);
                              SafeArrayDestroy(psa);

                              SAFEARRAY* pSafeArray = SafeArrayCreateVector(VT_UI1,0,0);
                              wsprintf(sz, _T("xtrnarray:0x%lx,%s"),
                                (unsigned long)(pSafeArray),ptrANSI);
                              hr = AddArgs(sz);
                              if (ptrANSI)
                                delete ptrANSI;
                              if (V_ISBYREF(pvarIndex))
                                {
                                  V_VT(pvarIndex) = VT_ARRAY | VT_UI1 | VT_BYREF;		
                                  *V_ARRAYREF(pvarIndex) = pSafeArray;
                                }
                              else
                                {
                                  V_VT(pvarIndex) = VT_ARRAY | VT_UI1;		
                                  V_ARRAY(pvarIndex) = pSafeArray;
                                }
                            }
                          else
                            {
                              ThrowPerformException(exception,ErrorException,
                                "Perform","Output array for blob did not specify image format");
                            }
                        }
                      else
                      {
                        ThrowPerformException(exception,ErrorException,
                          "Perform","Output array for blob must be 1d");
                      }
                    }
                  else
                    {
                      // the output is passed as a variant that is a BSTR
                      // - this is the way that VBSCRIPT and ASP does it.
                      CComVectorData<VARIANT> vals(psa);
                      if (vals)
                        {
                          VARIANT &varFirst = vals[0];
                          VARIANTARG *pvarFirst = &varFirst;
                          if (V_VT(pvarFirst) ==  VT_BSTR)
                            {
                              LPTSTR lpszVal = W2T(pvarFirst->bstrVal);
                              vals.UnaccessData();
                              SafeArrayDestroy(psa);

                              SAFEARRAY* pSafeArray = SafeArrayCreateVector(VT_UI1,0,0);
                              wsprintf(sz, _T("xtrnarray:0x%lx,%s"),
                                (unsigned long)(pSafeArray),lpszVal);
                              hr = AddArgs(sz);
                              if (V_ISBYREF(pvarIndex))
                                {
                                  V_VT(pvarIndex) = VT_ARRAY | VT_UI1 | VT_BYREF;		
                                  *V_ARRAYREF(pvarIndex) = pSafeArray;
                                }
                              else
                                {
                                  V_VT(pvarIndex) = VT_ARRAY | VT_UI1;		
                                  V_ARRAY(pvarIndex) = pSafeArray;
                                }
                            }
                        }
                      else
                      {
                        ThrowPerformException(exception,ErrorException,
                          "Perform","Output array for blob is invalid");
                      }
                    }
                }
            }
          else
          {
//------->
            ThrowPerformException(exception,ErrorException,
              "Perform","A passed array is not a vlid array");
          }
        }
//----->
    }
//->  // V_ISARRAY
    else
    {
      switch(vt)
      {
        case VT_VARIANT: // invalid, should never happen
        case VT_EMPTY:
        case VT_NULL:
          bFoundOption = false;
          break;

#ifdef SUPPORT_OBJECTS
        case VT_DISPATCH:
        {
          IDispatch *pDispatch;

          bFoundOption = false;
          if (V_ISBYREF(pvarIndex))
            pDispatch = *V_DISPATCHREF(pvarIndex);
          else
            pDispatch = V_DISPATCH(pvarIndex);

          if (pDispatch)
          {
            DispatchToImage(pDispatch,&pMagickImage);
          }
          break;
        }

        case VT_UNKNOWN:
        {
          IUnknown *pUnknown;

          bFoundOption = false;
          if (V_ISBYREF(pvarIndex))
            pUnknown = *V_UNKNOWNREF(pvarIndex);
          else
            pUnknown = V_UNKNOWN(pvarIndex);

          if (pUnknown)
          {
            UnknownToImage(pUnknown,&pMagickImage);
          }
          break;
        }
#endif

        case VT_BSTR:
        case VT_BSTR | VT_BYREF:
        {
          LPTSTR lpszVal;
				  LPTSTR lpszNext;

          if (V_ISBYREF(pvarIndex))
	          lpszVal = W2T(*V_BSTRREF(pvarIndex));
          else
	          lpszVal = W2T(V_BSTR(pvarIndex));

          bFoundOption = false;
          // is this a command line option argument?
          if ((*lpszVal == _T('+')) || (*lpszVal == _T('-')))
          {
            bFoundOption = true;
				    lpszNext = StrChr(lpszVal, _T('='));
				    if (lpszNext == NULL)
              hr = AddArgs(V_BSTR(pvarIndex));
            else
            {
				      int nLength = lpszNext - lpszVal;
				      if (nLength > 16)
                hr = AddArgs(V_BSTR(pvarIndex));
              else
              {
                *lpszNext = _T('\0');
                hr = AddArgs(lpszVal);
                hr = AddArgs(++lpszNext);
              }
              break;
            }
          }
          else
            hr = AddArgs(lpszVal);
          break;
        }

        case VT_UI1:
        case VT_UI1 | VT_BYREF:
        case VT_I2:
        case VT_I2 | VT_BYREF:
        case VT_I4:
        case VT_I4 | VT_BYREF:
        case VT_R4:
        case VT_R4 | VT_BYREF:
        case VT_R8:
        case VT_R8 | VT_BYREF:
        case VT_DECIMAL:
        case VT_DECIMAL | VT_BYREF:
        {
          VARIANT variant;

          bFoundOption = false;
          VariantInit(&variant);
          hr = VariantChangeTypeEx(&variant, pvarIndex, lcidDefault, 0, VT_BSTR);
          if (SUCCEEDED(hr) && V_VT(&variant) == VT_BSTR)
          {
            hr = AddArgs(V_BSTR(&variant));
          }
          VariantClear(&variant);
          break;
        }

        default:
          ThrowPerformException(exception,ErrorException,
            "Perform","Unsupported argument type");
      }
    }
  }

	LogInformation(methodName,"before execute");

  ImageInfo
    *image_info;

  image_info=CloneImageInfo((ImageInfo *) NULL);
  hr = Execute(func,&text,image_info,exception);
  DestroyImageInfo(image_info);

	LogInformation(methodName,"after execute");

  if (text != (char *) NULL)
    {
      CComVariant var;
      var = text;
      var.Detach(pVar);
      LiberateMemory((void **) &text);
    }

  if (m_coll.size())
  {
    CComVector<VARIANT> v(m_coll.size());
    if( !v )
    {
      //m_coll.clear();
      ThrowPerformException(exception,ErrorException,
        "Perform","Problems sending back array messages (1)");
    }
    else
    {
      // WARNING: This nested code block is required because
      // CComVectorData ctor performs a SafeArrayAccessData
      // and you have to make sure the SafeArrayUnaccessData
      // is called (which it is in the CComVectorData dtor)
      // before you use the CComVector::DetachTo(...).
      CComVectorData<VARIANT> msgs(v);
      if( !msgs )
      {
        //m_coll.clear();
        ThrowPerformException(exception,ErrorException,
          "Perform","Problems sending back array messages (2)");
      }
      else
      {
        for(int index = 0; index < m_coll.size(); ++index)
        {
          CComVariant vt(m_coll[index]);
          HRESULT hr = vt.Detach(&msgs[index]);
        }
      }
    }    
    V_VT(pVar) = VT_ARRAY | VT_VARIANT;
    V_ARRAY(pVar) = v.Detach();
    //m_coll.clear();
  }
	LogInformation(methodName,"return");
  return hr;
}

void CMagickImage::warninghandler(const ExceptionType warning,const char *message,
  const char *qualifier)
{
#ifdef STORE_MESSAGES
  error_callback->error_number=errno;
  errno=0;
  if (!message)
    return;
  (void) FormatMagickString(error_callback->warning_text,MaxTextExtent,
    "Warning %d: %.1024s%s%.1024s%s%s%.64s%s",warning,message,
    qualifier ? " (" : "",qualifier ? qualifier : "",
    qualifier? ")" : "",error_callback->error_number ? " [" : "",
    error_callback->error_number ? strerror(error_callback->error_number) : "",error_callback->error_number ? "]" : "");
  CComVariant var(error_callback->warning_text);
  error_callback->m_coll.push_back(var);
#endif
  char warning_text[MaxTextExtent];

  if (!message)
  {
	  LogInformation("warninghandler","called with no message");
    return;
  }
  (void) FormatMagickString(warning_text,MaxTextExtent,
    "ImageMagickObject - warning %d: %.1024s%s%.1024s%s%s%.64s%s\n",warning,
    message,qualifier ? " (" : "",qualifier ? qualifier : "",qualifier ? ")" :
    "",errno ? " [" : "",errno ? strerror(errno) : "",errno ? "]" : "");
	DebugString(warning_text);
}

void CMagickImage::errorhandler(const ExceptionType warning,const char *message,
  const char *qualifier)
{
  char error_text[MaxTextExtent];

  if (!message)
  {
	  LogInformation("errorhandler","called with no message");
    return;
  }
  (void) FormatMagickString(error_text,MaxTextExtent,
    "ImageMagickObject - error %d: %.1024s%s%.1024s%s%s%.64s%s\n",warning,
    message,qualifier ? " (" : "",qualifier ? qualifier : "", qualifier ? ")" :
    "",errno ? " [" : "",errno ? strerror(errno) : "",errno ? "]" : "");
	DebugString(error_text);
}

void CMagickImage::fatalerrorhandler(const ExceptionType error,const char *message,
  const char *qualifier)
{
  char fatalerror_text[MaxTextExtent];

  if (!message)
  {
	  LogInformation("fatalhandler","called with no message");
    return;
  }
  (void) FormatMagickString(fatalerror_text,MaxTextExtent,
    "ImageMagickObject - fatal error %d: %.1024s%s%.1024s%s%s%.64s%s",error,
    (message ? message : "ERROR"),
    qualifier ? " (" : "",qualifier ? qualifier : "",qualifier ? ")" : "",
    errno ? " [" : "",errno ? strerror(errno) : "",
    errno? "]" : "");
	DebugString(fatalerror_text);
  _DbgBreak();
}

HRESULT CMagickImage::Execute(
  unsigned int (*func)(ImageInfo *image_info,const int argc,char **argv,char **text,ExceptionInfo *exception),
    char **s,
      ImageInfo *image_info,
        ExceptionInfo *exception)
{
  unsigned int retcode = 0;

  retcode = (func)(image_info, GetArgc(), GetArgv(), s, exception);
  if (!retcode)
    return E_UNEXPECTED;
  return S_OK;
}

// Command line argument processing methods
HRESULT CMagickImage::AddArgs(VARIANTARG *rgvarg)
{
  HRESULT hr = S_OK;
  VARTYPE vt = V_VT(rgvarg);

  while (vt == (VT_VARIANT|VT_BYREF))
  {
	  rgvarg = V_VARIANTREF(rgvarg);
	  vt = V_VT(rgvarg);
  }
  if (V_ISARRAY(rgvarg))
  {
	  SAFEARRAY *psa = V_ISBYREF(rgvarg) ? *V_ARRAYREF(rgvarg) : V_ARRAY(rgvarg);
	  void **pav;
	  int index;
	  long *pArrayIndex, *pLowerBound, *pUpperBound;
	  VARIANT variant;

	  int dim = SafeArrayGetDim(psa);

	  VariantInit(&variant);
	  V_VT(&variant) = (vt & ~VT_ARRAY) | VT_BYREF;

	  pArrayIndex = new long [dim];
	  pLowerBound= new long [dim];
	  pUpperBound= new long [dim];
	  pav = new void *[dim];

	  for(index = 0; index < dim; ++index)
    {
      pav[index] = (void *) NULL;
      SafeArrayGetLBound(psa, index+1, &pLowerBound[index]);
      SafeArrayGetLBound(psa, index+1, &pArrayIndex[index]);
      SafeArrayGetUBound(psa, index+1, &pUpperBound[index]);
	  }

	  hr = SafeArrayLock(psa);
	  if (SUCCEEDED(hr))
    {
      while (index >= 0)
      {
		    hr = SafeArrayPtrOfIndex(psa, pArrayIndex, &V_BYREF(&variant));
		    if (FAILED(hr))
		      break;

        hr = AddArgs(&variant);
		    if (FAILED(hr))
		      break;

        for (index = dim-1; index >= 0; --index)
        {
          if (++pArrayIndex[index] <= pUpperBound[index])
            break;

		      pArrayIndex[index] = pLowerBound[index];
        }
      }

      /* preserve previous error code */
      HRESULT hr2 = SafeArrayUnlock(psa);
      if (SUCCEEDED(hr))
        hr = hr2;
	  }

    delete pArrayIndex;
    delete pLowerBound;
    delete pUpperBound;
    delete pav;

    return hr;
  }
  switch(vt & ~VT_BYREF)
  {
    case VT_VARIANT: /* invalid, should never happen */
    case VT_EMPTY:
    case VT_NULL:
      break;

    case VT_BSTR:
    {
      if (V_ISBYREF(rgvarg))
        hr = AddArgs(*V_BSTRREF(rgvarg));
      else
        hr = AddArgs(V_BSTR(rgvarg));
      break;
    }

#ifdef SUPPORT_OBJECTS
    case VT_DISPATCH:
    {
      IDispatch *pDispatch;

      if (V_ISBYREF(rgvarg))
        pDispatch = *V_DISPATCHREF(rgvarg);
      else
        pDispatch = V_DISPATCH(rgvarg);

      if (pDispatch)
      {
        CComBSTR bstrIOType;
        CComBSTR bstrName;
        CComBSTR bstrTemp;

        CComQIPtr<IDispatch> ptrDisp(pDispatch);
        CComQIPtr<IMagickImage> ptrObject;
        ptrObject = ptrDisp;
        ptrObject->get_Name(&bstrIOType);
        ptrObject->get_Name(&bstrName);
        bstrTemp = bstrIOType;
        bstrTemp += _T("@");
        bstrTemp += bstrName;
        hr = AddArgs(bstrTemp);
      }
      break;
    }

    case VT_UNKNOWN:
    {
      IUnknown *pUnknown;

      if (V_ISBYREF(rgvarg))
        pUnknown = *V_UNKNOWNREF(rgvarg);
      else
        pUnknown = V_UNKNOWN(rgvarg);

      if (pUnknown)
      {
        CComBSTR bstrName;

        CComPtr<IUnknown> ptrUnk(pUnknown);
        CComQIPtr<IMagickImage> ptrObject;
        ptrObject = ptrUnk;
        ptrObject->get_Name(&bstrName);
        hr = AddArgs(bstrName);
      }
      break;
    }
#endif

    case VT_DECIMAL:
    {
      VARIANT variant;
      VariantInit(&variant);
      hr = VariantChangeTypeEx(&variant, rgvarg, lcidDefault, 0, VT_R8);
      if (SUCCEEDED(hr) && V_VT(&variant) == VT_R8)
      {
        /* sv_setnv(sv, V_R8(&variant)); */
      }
      VariantClear(&variant);
      break;
    }

    case VT_BOOL:
    case VT_UI1:
    case VT_I2:
    case VT_I4:
    case VT_R4:
    case VT_R8:
    case VT_ERROR:
    case VT_DATE:
    case VT_CY:
    default:
    {
      VARIANT variant;

      VariantInit(&variant);
      hr = VariantChangeTypeEx(&variant, rgvarg, lcidDefault, 0, VT_BSTR);
      if (SUCCEEDED(hr) && V_VT(&variant) == VT_BSTR)
      {
        hr = AddArgs(V_BSTR(&variant));
      }
      VariantClear(&variant);
      break;
    }
  }
  return hr;
}

HRESULT CMagickImage::AddArgs(BSTR widestr)
{
  HRESULT hr = E_OUTOFMEMORY;

  if (m_argvIndex >= m_argc)
    return hr;

  hr = S_OK;
  MAKE_ANSIPTR_FROMWIDE(ptrANSI, widestr);
  m_argv[m_argvIndex++] = ptrANSI;

	DebugString("ImageMagickObject - arg: %s\n",ptrANSI);

  if (m_argvIndex >= m_argc)
    hr = ReAllocateArgs( nDefaultArgumentSize );

  return hr;
}

HRESULT CMagickImage::AddArgs(LPTSTR lpstr)
{
  HRESULT hr = E_OUTOFMEMORY;

  if (m_argvIndex >= m_argc)
    return hr;

  hr = S_OK;
#ifdef _UNICODE
  MAKE_ANSIPTR_FROMWIDE(ptrANSI, lpstr);
#else
  MAKE_COPY_OF_ANSI(ptrANSI, lpstr);
#endif
  m_argv[m_argvIndex++] = ptrANSI;

	DebugString("ImageMagickObject - arg: %s\n",ptrANSI);

  if (m_argvIndex >= m_argc)
    hr = ReAllocateArgs( nDefaultArgumentSize );

  return hr;
}

HRESULT CMagickImage::AllocateArgs(int cArgc)
{
  m_argv = new LPTSTR [cArgc];
  m_argv_t = new LPTSTR [cArgc];

  if ((m_argv == NULL) || (m_argv_t == NULL))
    return E_OUTOFMEMORY;
  m_argc = cArgc;

  m_argvIndex = 0;
  for (int i=0; i<m_argc; i++)
  {
    m_argv[i] = NULL;
    m_argv_t[i] = NULL;
  }
  return S_OK;
}

HRESULT CMagickImage::ReAllocateArgs(int cArgc)
{
  LPTSTR *argv = m_argv;
  LPTSTR *argv_t = m_argv_t;
  int argc = m_argc + cArgc;

  argv = new LPTSTR [argc];
  argv_t = new LPTSTR [argc];

  if ((argv == NULL) || (argv_t == NULL))
    return E_OUTOFMEMORY;

  for (int i=0; i<argc; i++)
  {
    if (i < m_argc)
    {
      argv[i] = m_argv[i];
      argv_t[i] = m_argv_t[i];
    }
    else
    {
      argv[i] = NULL;
      argv_t[i] = NULL;
    }
  }
  if (m_argv)
  {
    delete m_argv;
    m_argv = argv;
  }
  if (m_argv_t)
  {
    delete m_argv_t;
    m_argv_t = argv_t;
  }
  m_argc = argc;
  return S_OK;
}

void CMagickImage::DeleteArgs()
{
  EmptyArgs();
  if (m_argv)
    delete m_argv;
  if (m_argv_t)
    delete m_argv_t;
}

char **CMagickImage::GetArgv()
{
  return m_argv;
}

char **CMagickImage::GetArgvT()
{
  return m_argv_t;
}

int CMagickImage::GetArgc()
{
  return m_argvIndex;
}

void CMagickImage::EmptyArgs()
{
  for (int i=0; i<m_argc; i++)
  {
    if (m_argv[i] != NULL)
      delete (void *) (m_argv[i]);
    m_argv[i] = NULL;
    if (m_argv_t[i] != NULL)
      delete (void *) (m_argv_t[i]);
    m_argv_t[i] = NULL;
  }
  m_argvIndex = 0;
}

LPTSTR CMagickImage::StrChr(LPTSTR lpsz, TCHAR ch)
{
	LPTSTR p = NULL;
	while (*lpsz)
	{
		if (*lpsz == ch)
		{
			p = lpsz;
			break;
		}
		lpsz = CharNext(lpsz);
	}
	return p;
}
