/*
	ImageMagickObject.cpp
*/

#include "ImageMagickObject_.h"

static const DWORD dwErrorBase = 5000;
static const int nDefaultArgumentSize = 128;
static const LCID lcidDefault = 0;

static const int DEF_DEBUG_MODE = _CRTDBG_MODE_DEBUG;

// for logging
static const char objName[] = "ImageMagickObject";
static const char methodName[] = "Perform";

// xtrnarray
static const TCHAR xtrnarray_fmt[] = _T("xtrnarray:0x%lx,%ws");


[ module( dll, name = "ImageMagickObject", helpstring = "ImageMagickObject 1.0 Type Library" ) ]
class CModuelOverrideClass
{
	private:

	TCHAR m_szAppPath[ MAX_PATH ];

	public:

		BOOL WINAPI DllMain(
			DWORD dwReason,
			LPVOID lpReserved
		)
		{
			//ATLASSERT( FALSE );
			if( dwReason == DLL_PROCESS_ATTACH )
			{
				MagickLib::ExceptionInfo
				exception;

#ifdef _DEBUG
				int tmpDbgFlag;
#endif
				HINSTANCE hModuleInstance = _AtlBaseModule.GetModuleInstance();
				HINSTANCE hMResourceInstance = _AtlBaseModule.GetResourceInstance();
				if( !GetModuleFileName( hModuleInstance, m_szAppPath, MAX_PATH ) )
				{
					return FALSE;
				}

#ifdef _DEBUG
				tmpDbgFlag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );
				tmpDbgFlag |= _CRTDBG_CHECK_ALWAYS_DF;

				//tmpDbgFlag |= _CRTDBG_DELAY_FREE_MEM_DF;
				tmpDbgFlag |= _CRTDBG_LEAK_CHECK_DF;
				tmpDbgFlag |= _CRTDBG_ALLOC_MEM_DF;
				_CrtSetDbgFlag( tmpDbgFlag );
				_CrtSetReportMode( _CRT_ASSERT, DEF_DEBUG_MODE );
				_CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_DEBUG );
				_CrtSetReportMode( _CRT_ERROR, DEF_DEBUG_MODE );
#endif

				/* The first thing to do is make sure that basic critical sections are
				   initialized properly. This should prevent ANY other race conditions
				 */

				// MagickLib::InitializeSemaphore();

				/* Next we use a back door to init the path to US so that the logging
				   system can find its configuration file log.gk and load it
				 */
				 CT2AEX<MAX_PATH> app_path( m_szAppPath );
				(void)MagickLib::SetClientPath( app_path );

				//(void) MagickLib::SetLogEventMask("All");
				(void)MagickLib::LogMagickEvent( MagickLib::ResourceEvent, GetMagickModule(),
					"DLL Attach -  path: %s", app_path );

				// MagickLib::InitializeTracingCriticalSection();
				// MagickLib::DebugString("DLL Attach -  path: %s\n",m_szAppPath);
				MagickLib::InitializeMagick( app_path );
				MagickLib::RegisterStaticModules();
				MagickLib::GetExceptionInfo( &exception );
				(void)MagickLib::GetMagicInfo( (unsigned char*)NULL, 0, &exception );
				(void)MagickLib::GetDelegateInfo( "*", "*", &exception );
				MagickLib::DestroyExceptionInfo( &exception );
			}
			else if( dwReason == DLL_PROCESS_DETACH )
			{
				CT2AEX<MAX_PATH> app_path( m_szAppPath );
				(void)MagickLib::LogMagickEvent( MagickLib::ResourceEvent, GetMagickModule(),
					"DLL Detach -  path: %s", app_path );
				MagickLib::DestroyMagick();
#ifdef _DEBUG
				if( _CrtDumpMemoryLeaks() )
				{
					/* MagickLib::DebugString("ImageMagickObject - DLL Detach - leaks detected\n"); */
				}

#endif
			}

			return __super::DllMain( dwReason, lpReserved );
		}
};
[ emitidl ];

/////////////////////////////////////////////////////////////////////////////
// IMagickImage
[
	object,
	uuid( "7F670536-00AE-4EDF-B06F-13BD22B25624" ),
	dual,
	helpstring( "IMagickImage Interface" ),
	pointer_default( unique )
]
__interface IMagickImage : IDispatch
{
	//Standard Server Side Component Methods
	HRESULT OnStartPage([ in ] IUnknown * piUnk );
	HRESULT OnEndPage();
	[ propget, id( 1 ) ] HRESULT Count([ out, retval ] long* pVal );
	[ vararg, id( 2 ) ] HRESULT Add([ in, out, satype( VARIANT ) ] SAFEARRAY * *pArrayVar, [ out, retval ] VARIANT * pVar );
	[ id( 3 ) ] HRESULT Remove([ in ] VARIANT varIndex );
	[ vararg, id( 4 ) ] HRESULT Compare([ in, out, satype( VARIANT ) ] SAFEARRAY * *pArrayVar, [ out, retval ] VARIANT * pVar );
	[ vararg, id( 5 ) ] HRESULT Composite([ in, out, satype( VARIANT ) ] SAFEARRAY * *pArrayVar, [ out, retval ] VARIANT * pVar );
	[ vararg, id( 6 ) ] HRESULT Convert([ in, out, satype( VARIANT ) ] SAFEARRAY * *pArrayVar, [ out, retval ] VARIANT * pVar );
	[ vararg, id( 7 ) ] HRESULT Identify([ in, out, satype( VARIANT ) ] SAFEARRAY * *pArrayVar, [ out, retval ] VARIANT * pVar );
	[ vararg, id( 8 ) ] HRESULT Mogrify([ in, out, satype( VARIANT ) ] SAFEARRAY * *pArrayVar, [ out, retval ] VARIANT * pVar );
	[ vararg, id( 9 ) ] HRESULT Montage([ in, out, satype( VARIANT ) ] SAFEARRAY * *pArrayVar, [ out, retval ] VARIANT * pVar );
	[ vararg, id( 10 ) ] HRESULT Stream([ in, out, satype( VARIANT ) ] SAFEARRAY * *pArrayVar, [ out, retval ] VARIANT * pVar );
	[ vararg, id( 11 ) ] HRESULT TestHarness([ in, out, satype( VARIANT ) ] SAFEARRAY * *pArrayVar, [ out, retval ] VARIANT * pVar );
	[ propget, id( DISPID_NEWENUM ) ] HRESULT _NewEnum([ out, retval ] LPUNKNOWN * pVal );
	[ propget, id( DISPID_VALUE ) ] HRESULT Item([ in ] VARIANT varIndex, [ out, retval ] VARIANT * pVal );
	[ propget, id( 14 ) ] HRESULT Messages([ out, retval ] VARIANT * pVal );
};

class MagickImageError
{
	public:

		MagickLib::ExceptionInfo exception;
		bool fullException;
		DWORD fullExceptionCode;

	public:

		MagickImageError()
		{
			fullException = FALSE;
			fullExceptionCode = 0;

			MagickLib::GetExceptionInfo(& exception );
		}

		static LPCSTR translate_exception( DWORD );
		LPCSTR translate_exception() const
		{
			return translate_exception( fullExceptionCode );
		}
};

/////////////////////////////////////////////////////////////////////////////
// MagickImage
[
	coclass,
	threading( both ),
	support_error_info( "IMagickImage" ),
	vi_progid( "ImageMagickObject.MagickImage" ),
	progid( "ImageMagickObject.MagickImage.1" ),
	version( 1.0 ),
	uuid( "5630BE5A-3F5F-4BCA-A511-AD6A6386CAC1" ),
	helpstring( "MagickImage Class" )
]
class ATL_NO_VTABLE MagickImage:
	public IObjectControl,
	public IObjectConstruct,
	public ISupportErrorInfoImpl<&__uuidof(IMagickImage)>,
	public IObjectWithSiteImpl<MagickImage>,
	public IMagickImage
{
	public:

		MagickImage()
		{
#ifdef _DEBUG
			//ATLASSERT( FALSE );
#endif
			MagickLib::SetWarningHandler( warninghandler );
			MagickLib::SetErrorHandler( errorhandler );
			MagickLib::SetFatalErrorHandler( fatalerrorhandler );
			(void)MagickLib::LogMagickEvent( MagickLib::ResourceEvent, GetMagickModule(),
				"MagickImage constructor" );
			m_bOnStartPageCalled = FALSE;
		}

	private:

		HRESULT TestHarness(
			SAFEARRAY** pArrayVar,
			VARIANT* pVar
		);

		HRESULT Compare(
			SAFEARRAY** pArrayVar,
			VARIANT* pVar
		);

		HRESULT Composite(
			SAFEARRAY** pArrayVar,
			VARIANT* pVar
		);

		HRESULT Convert(
			SAFEARRAY** pArrayVar,
			VARIANT* pVar
		);

		HRESULT Identify(
			SAFEARRAY** pArrayVar,
			VARIANT* pVar
		);

		HRESULT Mogrify(
			SAFEARRAY** pArrayVar,
			VARIANT* pVar
		);

		HRESULT Montage(
			SAFEARRAY** pArrayVar,
			VARIANT* pVar
		);

		HRESULT Stream(
			SAFEARRAY** pArrayVar,
			VARIANT* pVar
		);

		HRESULT Execute( 
			MagickLib::MagickBooleanType (*func)( MagickLib::ImageInfo* image_info, const int argc, char** argv, char** text, MagickLib::ExceptionInfo* exception ),
			char** text,
			MagickLib::ImageInfo * info,
			MagickLib::ExceptionInfo * exception
		);

		HRESULT Perform( 
			MagickLib::MagickBooleanType (*func)( MagickLib::ImageInfo* image_info, const int argc, char** argv, char** text, MagickLib::ExceptionInfo* exception ),
			SAFEARRAY * *pArrayVar,
			VARIANT * pVar2,
			MagickLib::ExceptionInfo * exception
		);

	private:

		LPSTR* m_argv;
		int m_argc;
		int m_argvIndex;

		HRESULT AllocateArgs(
			int iArgs
		);

		HRESULT ReAllocateArgs(
			int iArgs
		);

		void DeleteArgs(
			void
		);

		char** GetArgv(
			void
		);

		int GetArgc(
			void
		);

		void EmptyArgs(
			void
		);

		HRESULT AddArgs( LPCWSTR );
		HRESULT AddArgs( LPCSTR );

	private:

		static void warninghandler(
			const MagickLib::ExceptionType warning,
			const char* message,
			const char* qualifier
		);

		static void errorhandler(
			const MagickLib::ExceptionType error,
			const char* message,
			const char* qualifier
		);

		static void fatalerrorhandler(
			const MagickLib::ExceptionType error,
			const char* message,
			const char* qualifier
		);

		static void CheckAndReportError(
			MagickImageError& error,
			HRESULT& hr,
			const char* program
		);

		static void GetXtrnArrayStr( CString&, const SAFEARRAY*, LPCWSTR );
		static void GetXtrnArrayStr( CString&, const SAFEARRAY* );

	public:

		DECLARE_PROTECT_FINAL_CONSTRUCT()

		HRESULT FinalConstruct()
		{
			(void)MagickLib::LogMagickEvent( MagickLib::ResourceEvent, GetMagickModule(),
				"FinalConstruct" );
			AllocateArgs( nDefaultArgumentSize );

			//MagickLib::InitializeMagick(NULL);
			return S_OK;
		}

		void FinalRelease()
		{
			(void)MagickLib::LogMagickEvent( MagickLib::ResourceEvent, GetMagickModule(),
				"FinalRelease" );
			DeleteArgs();
		}

		// Support for ASP page notifications methods
	public:

		//Active Server Pages Methods
		STDMETHOD( OnStartPage ) ( IUnknown * IUnk );
		STDMETHOD( OnEndPage ) ();
		STDMETHOD( get_Item ) ( /*[in]*/ VARIANT varIndex, /*[out, retval]*/ VARIANT * pVar );
		STDMETHOD( get__NewEnum ) ( /*[out, retval]*/ LPUNKNOWN * pVal );
		STDMETHOD( get_Count ) ( /*[out, retval]*/ long* pVal );
		STDMETHOD( get_Messages ) ( /*[out, retval]*/ VARIANT * pVar );
		STDMETHOD( Remove ) ( /*[in]*/ VARIANT varIndex );
		STDMETHOD( Add ) ( /*[in,out]*/ SAFEARRAY * *pArrayVar, /*[out, retval]*/ VARIANT * pVar );

	private:

		CComPtr<IRequest>m_piRequest;               //Request Object
		CComPtr<IResponse>m_piResponse;             //Response Object
		CComPtr<ISessionObject>m_piSession;         //Session Object
		CComPtr<IServer>m_piServer;                 //Server Object
		CComPtr<IApplicationObject>m_piApplication; //Application Object
		BOOL m_bOnStartPageCalled;                  //OnStartPage successful?
		CComPtr<IObjectContext> m_spObjectContext;

	public:

		// Support for COM+ activation and deactivation
		STDMETHOD( Activate ) ();
		STDMETHOD_( BOOL, CanBePooled ) ();
		STDMETHOD_( void, Deactivate ) ();
		STDMETHOD( Construct ) ( IDispatch * pCtorObj );
};

LPCSTR MagickImageError::translate_exception( DWORD code )
{
	switch( code )
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

#define ThrowPerformException( exception, code, reason, description )\
	{\
		(void)MagickLib::LogMagickEvent( MagickLib::ResourceEvent, GetMagickModule(),\
			"%s - %s %s", objName, reason, description );\
		MagickLib::ThrowException( exception, code, reason, description );\
		return E_INVALIDARG;\
	}

#define LogInformation( reason, description )\
	{\
		(void)MagickLib::LogMagickEvent( MagickLib::ResourceEvent, GetMagickModule(),\
			"%s - %s %s", objName, reason, description );\
	}

static long SafeArraySize(
	SAFEARRAY* psa
)
{
	HRESULT hr;
	long lBoundl, lBoundu;

	hr = ::SafeArrayGetLBound( psa, 1, &lBoundl );
	if( FAILED( hr ) )
	{
		return 0;
	}

	hr = ::SafeArrayGetUBound( psa, 1, &lBoundu );
	if( FAILED( hr ) )
	{
		return 0;
	}

	return (lBoundu - lBoundl + 1);
}

void MagickImage::GetXtrnArrayStr( CString& s, const SAFEARRAY* psa, LPCWSTR wszName )
{
	s.Format( xtrnarray_fmt, (unsigned long)psa, wszName );
}

void MagickImage::GetXtrnArrayStr( CString& s, const SAFEARRAY* psa )
{
	GetXtrnArrayStr( s, psa, L"" );
}

STDMETHODIMP MagickImage::get_Count(
	long* pVal
)
{
	ATLTRACENOTIMPL( _T( "MagickImage::get_Count" ) );
}

STDMETHODIMP MagickImage::get__NewEnum(
	LPUNKNOWN* pVal
)
{
	ATLTRACENOTIMPL( _T( "MagickImage::get__NewEnum" ) );
}

STDMETHODIMP MagickImage::get_Item(
	VARIANT varIndex,
	VARIANT* pVal
)
{
	HRESULT hr = E_INVALIDARG;
	VARIANTARG* pvarIndex = &varIndex;
	VARTYPE vt = V_VT( pvarIndex );
	long lIndex = 0;
	CComVariant var;

	while( vt == (VT_VARIANT | VT_BYREF) )
	{
		pvarIndex = V_VARIANTREF( pvarIndex );
		vt = V_VT( pvarIndex );
	}

	if( V_ISARRAY( pvarIndex ) )
	{
		return hr;
	}

	if( (vt & ~VT_BYREF) == VT_BSTR )
	{
		CW2A szVal( V_BSTR( pvarIndex ) );
		var = _T( "" );
		if( szVal )
		{
			MagickLib::Image* image;
			MagickLib::ImageInfo* image_info;
			MagickLib::ExceptionInfo exception;
			ssize_t id;
			LPSTR lpszNext;

			lpszNext = StrChrA( szVal, '.' );
			if( lpszNext == NULL )
			{
				lpszNext = "%w,%h,%m";
			}
			else
			{
				*lpszNext++ = '\0';
			}

			// lookup the registry id using token and pass the image in
			MagickLib::GetExceptionInfo( &exception );
			image = MagickLib::GetImageFromMagickRegistry( szVal, &id, &exception );
			if( image != (MagickLib::Image*)NULL )
			{
				LPSTR text;

				image_info = MagickLib::CloneImageInfo( (MagickLib::ImageInfo*)NULL );
				text = MagickLib::TranslateText( image_info, image, lpszNext );
				MagickLib::DestroyImageList( image );
				MagickLib::DestroyImageInfo( image_info );
				var = text;
				MagickLib::RelinquishMagickMemory( text );
				hr = S_OK;
			}
		}
	}

	var.Detach( pVal );
	return hr;
}

STDMETHODIMP MagickImage::get_Messages(
	VARIANT* pVar
)
{
	HRESULT hr = E_NOTIMPL;

#ifdef PERFORM_MESSAGE_CACHING
	if( m_coll.size() )
	{
		CSAVector<VARIANT>v( m_coll.size() );
		if( !v )
		{
			//m_coll.clear();
			return E_OUTOFMEMORY;
		}
		else
		{
			// WARNING: This nested code block is required because
			// CSAVectorData ctor performs a SafeArrayAccessData
			// and you have to make sure the SafeArrayUnaccessData
			// is called (which it is in the CSAVectorData dtor)
			// before you use the CSAVector::DetachTo(...).
			CSAVectorData<VARIANT>msgs( v );
			if( !msgs )
			{
				//m_coll.clear();
				return E_OUTOFMEMORY;
			}
			else
			{
				for( int index = 0; index < m_coll.size(); ++index )
				{
					CComVariant vt( m_coll[ index ] );
					HRESULT hr = vt.Detach( &msgs[ index ] );
				}
			}
		}

		V_VT( pVar ) = VT_ARRAY | VT_VARIANT;
		V_ARRAY( pVar ) = v.Detach();

		//m_coll.clear();
	}

#endif
	return hr;
}

STDMETHODIMP MagickImage::Add(
	SAFEARRAY** pArrayVar,
	VARIANT* pVar
)
{
	ATLTRACENOTIMPL( _T( "MagickImage::Add" ) );
}

STDMETHODIMP MagickImage::Remove(
	VARIANT varIndex
)
{
	HRESULT hr = E_INVALIDARG;
	VARIANTARG* pvarIndex = &varIndex;
	VARTYPE vt = V_VT( pvarIndex );

	while( vt == (VT_VARIANT | VT_BYREF) )
	{
		pvarIndex = V_VARIANTREF( pvarIndex );
		vt = V_VT( pvarIndex );
	}

	if( V_ISARRAY( pvarIndex ) )
	{
		return hr;
	}

	switch( vt & ~VT_BYREF )
	{
		case VT_BSTR:
		{
			if( !V_ISBYREF( pvarIndex ) )
			{
				CW2A szVal( V_BSTR( pvarIndex ) );
				if( szVal )
				{
					if( MagickLib::DeleteImageRegistry( szVal ) )
					{
						hr = S_OK;
					}
				}
			}

			break;
		}
	}
	return hr;
}

HRESULT MagickImage::Perform(
	MagickLib::MagickBooleanType (* func)(MagickLib::ImageInfo* image_info,const int argc, char** argv, char** text, MagickLib::ExceptionInfo* exception),
	SAFEARRAY** pArrayVar,
	VARIANT* pVar,
	MagickLib::ExceptionInfo* exception
)
{
	bool bDebug = false;
	HRESULT hr = E_INVALIDARG;
	char* text;

#ifdef _DEBUG
	//ATLASSERT( FALSE );
#endif

	LogInformation( methodName, "enter" );
	text = (char*)NULL;
	if( !pArrayVar )
	{
		ThrowPerformException( exception, MagickLib::ErrorException,
			"Perform", "Argument list is NULL" );
	}

	CComSafeArray<VARIANT>rg( *pArrayVar );
	if( !rg )
	{
		ThrowPerformException( exception, MagickLib::ErrorException,
			"Perform", "Argument list is bad" );
	}

	if( rg.GetDimensions() != 1 )
	{
		ThrowPerformException( exception, MagickLib::ErrorException,
			"Perform", "Multi dimensional array passed" );
	}

	if( rg.GetType() != VT_VARIANT )
	{
		ThrowPerformException( exception, MagickLib::ErrorException,
			"Perform", "Non VARIANT array type passed" );
	}

	int iLastVal = rg.GetCount();
	bool bFoundOption = false;
	for( int i = 0; i < iLastVal; ++i )
	{
		VARIANT& varIndex = rg[ i ];
		VARIANTARG* pvarIndex = &varIndex;
		VARTYPE vt = V_VT( pvarIndex );

		while( vt == (VT_VARIANT | VT_BYREF) )
		{
			pvarIndex = V_VARIANTREF( pvarIndex );
			vt = V_VT( pvarIndex );
		}

//->
		if( V_ISARRAY( pvarIndex ) )
		{
			CString sArg;
			SAFEARRAY* psa;

			if( V_ISBYREF( pvarIndex ) )
			{
				psa = *V_ARRAYREF( pvarIndex );
			}
			else
			{
				psa = V_ARRAY( pvarIndex );
			}

//----->
			{
//------->
				if( psa )
				{
					VARTYPE vatype = ( V_VT( pvarIndex ) & ~(VT_ARRAY | VT_BYREF) );
					int ndim = SafeArrayGetDim( psa );
					if( ndim != 1 )
					{
						ThrowPerformException( exception, MagickLib::ErrorException,
							"Perform", "Multi-dimensional arrays not supported" );
					}

					if( i < (iLastVal - 1) )

//------------>
					{
						bool bFoundIt = false;

						// This is any array that is not the last one in the arg
						// list. This means it must be an input so we just pass
						// it along.
						switch( vatype )

//--------------->
						{
							case VT_UI1:
							{
								GetXtrnArrayStr( sArg, psa );
								hr = AddArgs( sArg );
								break;
							}

							default:

//----------------->
							{
								CComSafeArray<VARIANT>vals( psa );
								if( vals )

//--------------------->
								{
									VARIANT& varFirst = vals[ 0 ];
									VARIANTARG* pvarFirst = &varFirst;
									if( V_VT( pvarFirst ) == VT_BSTR )

//------------------------->
									{
										VARIANT& varSecond = vals[ 1 ];
										VARIANTARG* pvarSecond = &varSecond;
										if( V_ISARRAY( pvarSecond ) )

//--------------------------->
										{
											if( V_ISBYREF( pvarSecond ) )
											{
												VARTYPE vatype2 = ( V_VT( pvarSecond ) & ~(VT_ARRAY | VT_BYREF) );
												if( vatype2 == VT_UI1 )
												{
													SAFEARRAY* psax = *V_ARRAYREF( pvarSecond );
													int ndim2 = SafeArrayGetDim( psax );
													if( ndim2 != 1 )
													{
														ThrowPerformException( exception, MagickLib::ErrorException,
															"Perform", "Input blob support requires a 1d array (1)" );
													}

													GetXtrnArrayStr( sArg, psax, pvarFirst->bstrVal );
													hr = AddArgs( sArg );
												}
											}
											else
											{
												VARTYPE vatype2 = ( V_VT( pvarSecond ) & ~(VT_ARRAY) );
												if( vatype2 == VT_UI1 )
												{
													SAFEARRAY* psax = V_ARRAY( pvarSecond );
													int ndim2 = SafeArrayGetDim( psax );
													if( ndim2 != 1 )
													{
														ThrowPerformException( exception, MagickLib::ErrorException,
															"Perform", "Input blob support requires a 1d array (2)" );
													}
													else
													{
														LPCWSTR pReturnBuffer = NULL;
														long size = SafeArraySize( psax );
														hr = SafeArrayAccessData( psax, (void**)&pReturnBuffer );
														if( SUCCEEDED( hr ) )
														{
															hr = SafeArrayUnaccessData( psax );
														}
													}

													GetXtrnArrayStr( sArg, psax, pvarFirst->bstrVal );
													hr = AddArgs( sArg );
												}
											}

//--------------------------->
										} // end of V_ISARRAY

//------------------------->
									} // end of == VT_BSTR
									else
									{
										GetXtrnArrayStr( sArg, psa );
										hr = AddArgs( sArg );
									}

									//vals.UnaccessData();
									vals.Detach();
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
						if( vatype == VT_UI1 )
						{
							// the output is passed as an array of bytes - this
							// is the way that VB does it.
							LPCWSTR pReturnBuffer = NULL;
							long size = SafeArraySize( psa );
							hr = SafeArrayAccessData( psa, (void**)&pReturnBuffer );
							if( SUCCEEDED( hr ) )
							{
								CStringW ws( pReturnBuffer, size );
								hr = SafeArrayUnaccessData( psa );
								SafeArrayDestroy( psa );

								SAFEARRAY* pSafeArray = SafeArrayCreateVector( VT_UI1, 0, 0 );
								GetXtrnArrayStr( sArg, pSafeArray, ws );
								hr = AddArgs( sArg );
								if( V_ISBYREF( pvarIndex ) )
								{
									V_VT( pvarIndex ) = VT_ARRAY | VT_UI1 | VT_BYREF;
									*V_ARRAYREF( pvarIndex ) = pSafeArray;
								}
								else
								{
									V_VT( pvarIndex ) = VT_ARRAY | VT_UI1;
									V_ARRAY( pvarIndex ) = pSafeArray;
								}
							}
							else
							{
								ThrowPerformException( exception, MagickLib::ErrorException,
									"Perform", "Output array for blob must be 1d" );
							}
						}
						else
						{
							// the output is passed as a variant that is a BSTR
							// - this is the way that VBSCRIPT and ASP does it.
							CComSafeArray<VARIANT>vals( psa );
							if( vals )
							{
								VARIANT& varFirst = vals[ 0 ];
								VARIANTARG* pvarFirst = &varFirst;
								if( V_VT( pvarFirst ) == VT_BSTR )
								{
									//vals.UnaccessData();
									SafeArrayDestroy( psa );

									SAFEARRAY* pSafeArray = SafeArrayCreateVector( VT_UI1, 0, 0 );
									GetXtrnArrayStr( sArg, pSafeArray, pvarFirst->bstrVal );
									hr = AddArgs( sArg );
									if( V_ISBYREF( pvarIndex ) )
									{
										V_VT( pvarIndex ) = VT_ARRAY | VT_UI1 | VT_BYREF;
										*V_ARRAYREF( pvarIndex ) = pSafeArray;
									}
									else
									{
										V_VT( pvarIndex ) = VT_ARRAY | VT_UI1;
										V_ARRAY( pvarIndex ) = pSafeArray;
									}
								}
							}
							else
							{
								ThrowPerformException( exception, MagickLib::ErrorException,
									"Perform", "Output array for blob is invalid" );
							}
						}
					}
				}
				else
				{
//------->
					ThrowPerformException( exception, MagickLib::ErrorException,
						"Perform", "A passed array is not a valid array" );
				}
			}

//----->
		}
//->  // V_ISARRAY
		else
		{
			switch( vt )
			{
				case VT_VARIANT: // invalid, should never happen
				case VT_EMPTY:
				case VT_NULL:
				bFoundOption = false;
				break;

				case VT_BSTR:
				case VT_BSTR | VT_BYREF:
				{
					LPTSTR lpszVal;
					LPTSTR lpszNext;

					CW2T str( V_ISBYREF( pvarIndex ) ? *V_BSTRREF( pvarIndex ) : V_BSTR( pvarIndex ) );
					lpszVal = (LPTSTR)str;
					bFoundOption = false;

					// is this a command line option argument?
					if( ( *lpszVal == _T( '+' ) ) || ( *lpszVal == _T( '-' ) ) )
					{
						bFoundOption = true;
						lpszNext = StrChr( lpszVal, _T( '=' ) );
						if( lpszNext == NULL )
						{
							hr = AddArgs( V_BSTR( pvarIndex ) );
						}
						else
						{
							int nLength = lpszNext - lpszVal;
							if( nLength > 16 )
							{
								hr = AddArgs( V_BSTR( pvarIndex ) );
							}
							else
							{
								*lpszNext = _T( '\0' );
								hr = AddArgs( lpszVal );
								hr = AddArgs( ++lpszNext );
							}

							break;
						}
					}
					else
					{
						hr = AddArgs( lpszVal );
					}

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
					VariantInit( &variant );
					hr = VariantChangeTypeEx( &variant, pvarIndex, lcidDefault, 0, VT_BSTR );
					if( SUCCEEDED( hr ) && (V_VT( &variant ) == VT_BSTR) )
					{
						hr = AddArgs( V_BSTR( &variant ) );
					}

					VariantClear( &variant );
					break;
				}

				default:
				ThrowPerformException( exception, MagickLib::ErrorException,
					"Perform", "Unsupported argument type" );
			}
		}
	}

	LogInformation( methodName, "before execute" );

	MagickLib::ImageInfo
	* image_info;

	image_info = MagickLib::CloneImageInfo( (MagickLib::ImageInfo*)NULL );
#ifdef _DEBUG

	//ATLASSERT( FALSE );
#endif
	hr = Execute( func, &text, image_info, exception );
	MagickLib::DestroyImageInfo( image_info );

	LogInformation( methodName, "after execute" );

	if( text != (char*)NULL )
	{
		CComVariant var;
		var = text;
		var.Detach( pVar );
		MagickLib::RelinquishMagickMemory( text );
	}

	LogInformation( methodName, "return" );
	return hr;
}

void MagickImage::warninghandler(
	const MagickLib::ExceptionType warning,
	const char* message,
	const char* qualifier
)
{
	char warning_text[ MaxTextExtent ];

	if( !message )
	{
		LogInformation( "warninghandler", "called with no message" );
		return;
	}

	MagickLib::FormatMagickString( warning_text, MaxTextExtent,
		"warning %d: %.1024s%s%.1024s%s%s%.64s%s\n", warning, message,
		qualifier ? " (" : "", qualifier ? qualifier : "",
		qualifier ? ")" : "", errno ? " [" : "",
		errno ? strerror( errno ) : "", errno ? "]" : "" );
	(void)MagickLib::LogMagickEvent( MagickLib::ResourceEvent, GetMagickModule(), warning_text );
}

void MagickImage::errorhandler(
	const MagickLib::ExceptionType warning,
	const char* message,
	const char* qualifier
)
{
	char error_text[ MaxTextExtent ];

	if( !message )
	{
		LogInformation( "errorhandler", "called with no message" );
		return;
	}

	MagickLib::FormatMagickString( error_text, MaxTextExtent,
		"error %d: %.1024s%s%.1024s%s%s%.64s%s\n", warning, message,
		qualifier ? " (" : "", qualifier ? qualifier : "",
		qualifier ? ")" : "", errno ? " [" : "",
		errno ? strerror( errno ) : "", errno ? "]" : "" );
	(void)MagickLib::LogMagickEvent( MagickLib::ResourceEvent, GetMagickModule(), error_text );
}

void MagickImage::fatalerrorhandler(
	const MagickLib::ExceptionType error,
	const char* message,
	const char* qualifier
)
{
	char fatalerror_text[ MaxTextExtent ];

	if( !message )
	{
		LogInformation( "fatalhandler", "called with no message" );
		return;
	}

	MagickLib::FormatMagickString( fatalerror_text, MaxTextExtent,
		"fatal error %d: %.1024s%s%.1024s%s%s%.64s%s", error,
		(message ? message : "ERROR"),
		qualifier ? " (" : "", qualifier ? qualifier : "", qualifier ? ")" : "",
		errno ? " [" : "", errno ? strerror( errno ) : "", errno ? "]" : "" );
	(void)MagickLib::LogMagickEvent( MagickLib::ResourceEvent, GetMagickModule(), fatalerror_text );

//  ATLASSERT( FALSE );
}

#define ENABLE_FULL_EXCEPTIONS

void MagickImage::CheckAndReportError(
	MagickImageError& error,
	HRESULT& hr,
	const char* program
)
{
	char message_text[ MaxTextExtent ];

	if ( FAILED(hr) )
	{
		if( error.fullException )
		{
			MagickLib::FormatMagickString( message_text, MaxTextExtent,
				"%s: 0x%08X: %.1024s", program, error.fullExceptionCode, error.translate_exception() );
		}
		else
		{
			const MagickLib::ExceptionInfo* exceptionlist;
			MagickLib::ResetLinkedListIterator( (MagickLib::LinkedListInfo*)error.exception.exceptions );
			exceptionlist = (const MagickLib::ExceptionInfo*)MagickLib::GetNextValueInLinkedList( (MagickLib::LinkedListInfo*)
				error.exception.exceptions );

			*message_text = 0;

			while( exceptionlist != (const MagickLib::ExceptionInfo*)NULL )
			{
				int len = strlen( message_text );
				if( MaxTextExtent - len < 0 )
				{
					break;
				}

				MagickLib::FormatMagickString( message_text + len, MaxTextExtent - len,
					"%s: %d: %.1024s: %.1024s\r\n",
					program,
					exceptionlist->severity,
					exceptionlist->reason ? exceptionlist->reason : "",
					exceptionlist->description ? exceptionlist->description : "" );

				exceptionlist = (const MagickLib::ExceptionInfo*)MagickLib::GetNextValueInLinkedList( (MagickLib::LinkedListInfo*)
					error.exception.exceptions );
			}
			hr = MAKE_HRESULT( SEVERITY_ERROR, FACILITY_ITF, dwErrorBase + 1001 );

	#ifdef _DEBUG

			//ATLASSERT( FALSE );
	#endif
			CA2WEX<MaxTextExtent>wsMessageText( message_text );
			Error( wsMessageText, __uuidof( IMagickImage ), hr );
		}
	}

	MagickLib::DestroyExceptionInfo( &error.exception );
}

HRESULT MagickImage::Execute(
	MagickLib::MagickBooleanType (* func)(MagickLib::ImageInfo* image_info, const int argc, char** argv, char** text, MagickLib::ExceptionInfo* exception),
	char** s,
	MagickLib::ImageInfo* image_info,
	MagickLib::ExceptionInfo* exception
)
{
	unsigned int retcode = 0;

	retcode = !(func)( image_info, GetArgc(), GetArgv(), s, exception );
	if( !retcode )
	{
		return E_UNEXPECTED;
	}

	return S_OK;
}

HRESULT MagickImage::TestHarness(
	SAFEARRAY** pArrayVar,
	VARIANT* pVar
)
{
	HRESULT hr = S_OK;

	(void)MagickLib::LogMagickEvent( MagickLib::ResourceEvent, GetMagickModule(),
		"TestHarness" );

	MagickLib::ExceptionInfo exception;
	char
	* reason,
	* description,
	  message_text[ MaxTextExtent ];

	reason = "unknown";
	description = "unknown";
	MagickLib::GetExceptionInfo( &exception );

	CComVariant var;

	if( !pArrayVar )
	{
		return E_INVALIDARG;
	}

	CComSafeArray<VARIANT>rg( *pArrayVar );
	if( !rg )
	{
		return E_INVALIDARG;
	}

	if( rg.GetDimensions() != 1 )
	{
		ThrowPerformException( &exception, MagickLib::ErrorException,
			"Perform", "Multi dimensional array passed" );
	}

	if( rg.GetType() != VT_VARIANT )
	{
		ThrowPerformException( &exception, MagickLib::ErrorException,
			"Perform", "Non VARIANT array type passed" );
	}

	EmptyArgs();
	AddArgs( L"-convert" );
	int iLastVal = rg.GetCount();
	for( int i = 0; i < iLastVal; ++i )
	{
		{
			CComVariant vt( rg[ i ] );
			vt.ChangeType( VT_BSTR );
			CW2A str( vt.bstrVal );
			(void)MagickLib::LogMagickEvent( MagickLib::ResourceEvent, GetMagickModule(),
				"arg: %s", (LPCSTR)str );
			hr = AddArgs( vt.bstrVal );
		}
	}

	//__try
	{
		char* text;
		MagickLib::ImageInfo* image_info;
		image_info = MagickLib::CloneImageInfo( (MagickLib::ImageInfo*)NULL );
		text = (char*)NULL;
		hr = Execute( MagickLib::ConvertImageCommand, &text, image_info, &exception );
		MagickLib::DestroyImageInfo( image_info );
		if( text != (char*)NULL )
		{
			var = text;
			var.Detach( pVar );
			MagickLib::RelinquishMagickMemory( text );
		}

		if( FAILED( hr ) )
		{
			if( exception.reason )
			{
				reason = exception.reason;
			}

			if( exception.description )
			{
				description = exception.description;
			}
		}
	}

	//__except(1)
	//{
	//  hr = E_UNEXPECTED;
	//  reason = "exception";
	//  description = translate_exception(_exception_code());
	//}

	if( FAILED( hr ) )
	{
		hr = MAKE_HRESULT( SEVERITY_ERROR, FACILITY_ITF, dwErrorBase + 1001 );
		MagickLib::FormatMagickString( message_text, MaxTextExtent,
			"convert: %d: %.1024s: %.1024s", exception.severity, reason, description );
		CA2WEX<MaxTextExtent> str( message_text );
#ifdef _DEBUG

//    ATLASSERT( FALSE );
#endif
		Error( str, __uuidof( IMagickImage ), hr );
	}

	MagickLib::DestroyExceptionInfo( &exception );
	return hr;
}

STDMETHODIMP MagickImage::Compare(
	SAFEARRAY** pArrayVar,
	VARIANT* pVar
)
{
	HRESULT hr;

	class MagickImageError error;

	//unsigned char
	//  *leaktest;

	//leaktest=(unsigned char *) MagickLib::AcquireMemory(1024);

#ifdef ENABLE_FULL_EXCEPTIONS
	__try
#endif
	{
		EmptyArgs();
		AddArgs( L"-compare" );
		hr = Perform( MagickLib::CompareImageCommand, pArrayVar, pVar, &error.exception );
	}
#ifdef ENABLE_FULL_EXCEPTIONS
	__except( 1 )
	{
		hr = E_UNEXPECTED;
		error.fullException = TRUE;
		error.fullExceptionCode = _exception_code();
	}
#endif
	CheckAndReportError( error, hr, "compare" );
	return hr;
}

STDMETHODIMP MagickImage::Composite(
	SAFEARRAY** pArrayVar,
	VARIANT* pVar
)
{
	HRESULT hr;
	MagickImageError error;

#ifdef ENABLE_FULL_EXCEPTIONS
	__try
#endif
	{
		EmptyArgs();
		AddArgs( L"-composite" );
		hr = Perform( MagickLib::CompositeImageCommand, pArrayVar, pVar, &error.exception );
	}
#ifdef ENABLE_FULL_EXCEPTIONS
	__except( 1 )
	{
		hr = E_UNEXPECTED;
		error.fullException = TRUE;
		error.fullExceptionCode = _exception_code();
	}
#endif
	CheckAndReportError( error, hr, "composite" );
	return hr;
}

STDMETHODIMP MagickImage::Convert(
	SAFEARRAY** pArrayVar,
	VARIANT* pVar
)
{
	HRESULT hr;
	MagickImageError error;

	ATLASSERT( FALSE );

#ifdef ENABLE_FULL_EXCEPTIONS
	__try
#endif
	{
		EmptyArgs();
		AddArgs( L"-convert" );
		hr = Perform( MagickLib::ConvertImageCommand, pArrayVar, pVar, &error.exception );
	}
#ifdef ENABLE_FULL_EXCEPTIONS
	__except( 1 )
	{
		hr = E_UNEXPECTED;
		error.fullException = TRUE;
		error.fullExceptionCode = _exception_code();
	}
#endif
	CheckAndReportError( error, hr, "convert" );
	return hr;
}

HRESULT MagickImage::Identify(
	SAFEARRAY** pArrayVar,
	VARIANT* pVar
)
{
	HRESULT hr;
	MagickImageError error;

#ifdef ENABLE_FULL_EXCEPTIONS
	__try
#endif
	{
		EmptyArgs();
		AddArgs( L"-identify" );
		hr = Perform( MagickLib::IdentifyImageCommand, pArrayVar, pVar, &error.exception );
	}
#ifdef ENABLE_FULL_EXCEPTIONS
	__except( 1 )
	{
		hr = E_UNEXPECTED;
		error.fullException = TRUE;
		error.fullExceptionCode = _exception_code();
	}
#endif
	CheckAndReportError( error, hr, "identity" );
	return hr;
}

HRESULT MagickImage::Mogrify(
	SAFEARRAY** pArrayVar,
	VARIANT* pVar
)
{
	HRESULT hr;
	MagickImageError error;

#ifdef ENABLE_FULL_EXCEPTIONS
	__try
#endif
	{
		EmptyArgs();
		AddArgs( L"-mogrify" );
		hr = Perform( MagickLib::MogrifyImageCommand, pArrayVar, pVar, &error.exception );
	}
#ifdef ENABLE_FULL_EXCEPTIONS
	__except( 1 )
	{
		hr = E_UNEXPECTED;
		error.fullException = TRUE;
		error.fullExceptionCode = _exception_code();
	}
#endif

	CheckAndReportError( error, hr, "morgify" );
	return hr;
}

HRESULT MagickImage::Montage(
	SAFEARRAY** pArrayVar,
	VARIANT* pVar
)
{
	HRESULT hr;
	MagickImageError error;

#ifdef ENABLE_FULL_EXCEPTIONS
	__try
#endif
	{
		EmptyArgs();
		AddArgs( L"-montage" );
		hr = Perform( MagickLib::MontageImageCommand, pArrayVar, pVar, &error.exception );
	}
#ifdef ENABLE_FULL_EXCEPTIONS
	__except( 1 )
	{
		hr = E_UNEXPECTED;
		error.fullException = TRUE;
		error.fullExceptionCode = _exception_code();
	}
#endif
	CheckAndReportError( error, hr, "montage" );
	return hr;
}

STDMETHODIMP MagickImage::Stream(
	SAFEARRAY** pArrayVar,
	VARIANT* pVar
)
{
	HRESULT hr;
	MagickImageError error;

#ifdef ENABLE_FULL_EXCEPTIONS
	__try
#endif
	{
		EmptyArgs();
		AddArgs( L"-stream" );
		hr = Perform( MagickLib::StreamImageCommand, pArrayVar, pVar, &error.exception );
	}
#ifdef ENABLE_FULL_EXCEPTIONS
	__except( 1 )
	{
		hr = E_UNEXPECTED;
		error.fullException = TRUE;
		error.fullExceptionCode = _exception_code();
	}
#endif
	CheckAndReportError( error, hr, "stream" );
	return hr;
}

HRESULT MagickImage::AddArgs(
	LPCWSTR widestr
)
{
	HRESULT hr = E_OUTOFMEMORY;

	if( m_argvIndex >= m_argc )
	{
		return hr;
	}

	hr = S_OK;

	CW2A sArgUTF8( widestr, CP_UTF8 );
	m_argv[ m_argvIndex++ ] = strdup( sArgUTF8 );

	CW2A sArgANSI( widestr );
	(void)MagickLib::LogMagickEvent( MagickLib::ResourceEvent, GetMagickModule(),
		"arg: %s", (LPSTR)sArgANSI );

	if( m_argvIndex >= m_argc )
	{
		hr = ReAllocateArgs( nDefaultArgumentSize );
	}

	return hr;
}

HRESULT MagickImage::AddArgs(
	LPCSTR lpstr
)
{
	HRESULT hr = E_OUTOFMEMORY;

	if( m_argvIndex >= m_argc )
	{
		return hr;
	}

	hr = S_OK;

	CA2W wsArg( lpstr );
	CW2A sArgUTF8( wsArg, CP_UTF8 );
	CW2A sArgANSI( wsArg );

	m_argv[ m_argvIndex++ ] = strdup( sArgUTF8 );

	(void)MagickLib::LogMagickEvent( MagickLib::ResourceEvent, GetMagickModule(),
		"arg: %s", (LPSTR)sArgANSI );

	if( m_argvIndex >= m_argc )
	{
		hr = ReAllocateArgs( nDefaultArgumentSize );
	}

	return hr;
}

HRESULT MagickImage::AllocateArgs(
	int cArgc
)
{
	m_argv = new LPSTR [ cArgc ];

	if( m_argv == NULL )
	{
		return E_OUTOFMEMORY;
	}

	m_argc = cArgc;
	m_argvIndex = 0;
	for( int i = 0; i < m_argc; i++ )
	{
		m_argv[ i ] = NULL;
	}

	return S_OK;
}

HRESULT MagickImage::ReAllocateArgs(
	int cArgc
)
{
	LPSTR* argv = m_argv;
	int argc = m_argc + cArgc;

	argv = new LPSTR [ argc ];

	if( argv == NULL )
	{
		return E_OUTOFMEMORY;
	}

	for( int i = 0; i < argc; i++ )
	{
		if( i < m_argc )
		{
			argv[ i ] = m_argv[ i ];
		}
		else
		{
			argv[ i ] = NULL;
		}
	}

	if( m_argv )
	{
		delete m_argv;
		m_argv = argv;
	}

	m_argc = argc;
	return S_OK;
}

void MagickImage::DeleteArgs()
{
	EmptyArgs();
	if( m_argv )
	{
		delete m_argv;
	}
}

char** MagickImage::GetArgv()
{
	return m_argv;
}

int MagickImage::GetArgc()
{
	return m_argvIndex;
}

void MagickImage::EmptyArgs()
{
	for( int i = 0; i < m_argc; i++ )
	{
		free( (void*)(m_argv[ i ]) );
		m_argv[ i ] = NULL;
	}

	m_argvIndex = 0;
}

STDMETHODIMP MagickImage::OnStartPage(
	IUnknown* pUnk
)
{
	(void)MagickLib::LogMagickEvent( MagickLib::ResourceEvent, GetMagickModule(),
		"OnStartPage" );
	if( !pUnk )
	{
		return E_POINTER;
	}

	CComPtr<IScriptingContext>spContext;
	HRESULT hr;

	// Get the IScriptingContext Interface
	hr = pUnk->QueryInterface( __uuidof( IScriptingContext ), (void**)&spContext );
	if( FAILED( hr ) )
	{
		return hr;
	}

	// Get Request Object Pointer
	hr = spContext->get_Request( &m_piRequest );
	if( FAILED( hr ) )
	{
		(void)MagickLib::LogMagickEvent( MagickLib::ResourceEvent, GetMagickModule(),
			"OnStartPage get Request failed" );

		//spContext.Release();
		//return hr;
	}

	// Get Response Object Pointer
	hr = spContext->get_Response( &m_piResponse );
	if( FAILED( hr ) )
	{
		(void)MagickLib::LogMagickEvent( MagickLib::ResourceEvent, GetMagickModule(),
			"OnStartPage get Response failed" );

		//m_piRequest.Release();
		//return hr;
	}

	// Get Server Object Pointer
	hr = spContext->get_Server( &m_piServer );
	if( FAILED( hr ) )
	{
		(void)MagickLib::LogMagickEvent( MagickLib::ResourceEvent, GetMagickModule(),
			"OnStartPage get Server failed" );

		//m_piRequest.Release();
		//m_piResponse.Release();
		//return hr;
	}

	// Get Session Object Pointer
	hr = spContext->get_Session( &m_piSession );
	if( FAILED( hr ) )
	{
		(void)MagickLib::LogMagickEvent( MagickLib::ResourceEvent, GetMagickModule(),
			"OnStartPage get Session failed" );

		//m_piRequest.Release();
		//m_piResponse.Release();
		//m_piServer.Release();
		//return hr;
	}

	// Get Application Object Pointer
	hr = spContext->get_Application( &m_piApplication );
	if( FAILED( hr ) )
	{
		(void)MagickLib::LogMagickEvent( MagickLib::ResourceEvent, GetMagickModule(),
			"OnStartPage get Application failed" );

		//m_piRequest.Release();
		//m_piResponse.Release();
		//m_piServer.Release();
		//m_piSession.Release();
		//return hr;
	}

	m_bOnStartPageCalled = TRUE;

	{
		CComPtr<IRequestDictionary>pReadDictionary;
		CComPtr<IReadCookie>pCookieDictionary;

		hr = m_piRequest->get_Cookies( &pReadDictionary );
		if( SUCCEEDED( hr ) )
		{
			CComVariant vtIn( _T( "MAGICK_DEBUG" ) );
			CComVariant vtKey( _T( "level" ) );
			CComVariant vtOut;
			CComVariant vtCookieValue;

			hr = pReadDictionary->get_Item( vtIn, &vtOut );
			if( SUCCEEDED( hr ) && (V_VT( &vtOut ) == VT_DISPATCH) )
			{
				pCookieDictionary = (IReadCookie*)(vtOut.pdispVal);
				hr = pCookieDictionary->get_Item( vtKey, &vtCookieValue );
				if( SUCCEEDED( hr ) && (V_VT( &vtCookieValue ) == VT_BSTR) )
				{
					CW2T str( vtCookieValue.bstrVal );
					int level = _ttoi( str );
#if defined (_ENABLE_OLD_LOGGING_SUPPORT_)
					MagickLib::DebugLevel( level );
#endif
					(void)MagickLib::LogMagickEvent( MagickLib::ResourceEvent, GetMagickModule(),
						"OnStartPage debug level: %d", level );
				}
				else
				{
					(void)MagickLib::LogMagickEvent( MagickLib::ResourceEvent, GetMagickModule(),
						"OnStartPage - parse error" );
				}
			}
			else
			{
				(void)MagickLib::LogMagickEvent( MagickLib::ResourceEvent, GetMagickModule(),
					"OnStartPage - no MAGICK_DEBUG" );
			}
		}
		else
		{
			(void)MagickLib::LogMagickEvent( MagickLib::ResourceEvent, GetMagickModule(),
				"OnStartPage - no cookies" );
		}
	}
	return S_OK;
}

STDMETHODIMP MagickImage::OnEndPage()
{
	(void)MagickLib::LogMagickEvent( MagickLib::ResourceEvent, GetMagickModule(),
		"OnEndPage" );
	m_bOnStartPageCalled = FALSE;

	// Release all interfaces
	if( m_piRequest )
	{
		m_piRequest.Release();
	}

	if( m_piResponse )
	{
		m_piResponse.Release();
	}

	if( m_piServer )
	{
		m_piServer.Release();
	}

	if( m_piSession )
	{
		m_piSession.Release();
	}

	if( m_piApplication )
	{
		m_piApplication.Release();
	}

	return S_OK;
}

HRESULT MagickImage::Activate()
{
	(void)MagickLib::LogMagickEvent( MagickLib::ResourceEvent, GetMagickModule(),
		"Activate" );
	HRESULT hr = GetObjectContext( &m_spObjectContext );
	if( SUCCEEDED( hr ) )
	{
		return S_OK;
	}

	return hr;
}

BOOL MagickImage::CanBePooled()
{
	(void)MagickLib::LogMagickEvent( MagickLib::ResourceEvent, GetMagickModule(),
		"CanBePooled" );
	return FALSE;
}

void MagickImage::Deactivate()
{
	(void)MagickLib::LogMagickEvent( MagickLib::ResourceEvent, GetMagickModule(),
		"Deactivate" );
	m_spObjectContext.Release();
}

typedef CAtlArray<CStringA>CStringArray;

static void LocalTokenize(
	const CStringA& s,
	LPCSTR sz,
	CStringArray& asToken
)
{
	int nPos = 0;
	CStringA sToken;

	asToken.RemoveAll();
	sToken = s.Tokenize( sz, nPos );
	while( !sToken.IsEmpty() )
	{
		asToken.Add( sToken );
	}

	if( asToken.GetCount() == 0 )
	{
		asToken.Add( s );
	}
}

HRESULT MagickImage::Construct(
	IDispatch* pCtorObj
)
{
	(void)MagickLib::LogMagickEvent( MagickLib::ResourceEvent, GetMagickModule(),
		"Construct" );
	CComPtr<IObjectConstructString>spObjectConstructString;
	HRESULT hr = pCtorObj->QueryInterface( &spObjectConstructString );
	if( SUCCEEDED( hr ) )
	{
		CComBSTR bstrConstruct;
		hr = spObjectConstructString->get_ConstructString( &bstrConstruct );
		if( SUCCEEDED( hr ) )
		{
			CStringA sOptions( bstrConstruct );
			int nPos = 0;
			CStringA sToken;
			CStringArray asToken;

			(void)MagickLib::LogMagickEvent( MagickLib::ResourceEvent, GetMagickModule(),
				"Construct data: %s", (LPCSTR)sOptions );

			sToken = sOptions.Tokenize( ".", nPos );
			while( !sToken.IsEmpty() )
			{
				LocalTokenize( sToken, "=", asToken );
				if( asToken.GetCount() == 2 )
				{
					const CStringA& sName = asToken[ 0 ];
					const CStringA& sValue = asToken[ 1 ];

					(void)MagickLib::LogMagickEvent( MagickLib::ResourceEvent, GetMagickModule(),
						"Construct name: %s value: %s",
						(LPCSTR)sName, (LPCSTR)sValue );

#if defined (_ENABLE_OLD_LOGGING_SUPPORT_)
					if( sName.Compare( "MAGICK_DEBUG_LEVEL" ) == 0 )
					{
						MagickLib::DebugLevel( atoi( sValue ) );
					}

					if( sName.Compare( "MAGICK_DEBUG_PATH" ) == 0 )
					{
						MagickLib::DebugFilePath( sValue );
					}

					if( sName.Compare( "MAGICK_LOG_EVENTMASK" ) == 0 )
					{
						MagickLib::SetLogEventMask( sValue );
					}

#endif
				} // if

			} // while

		} // if

	}

	return hr;
}
