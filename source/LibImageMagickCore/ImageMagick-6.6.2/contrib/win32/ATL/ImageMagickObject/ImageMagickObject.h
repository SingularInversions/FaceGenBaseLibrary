/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 5.01.0164 */
/* at Mon Sep 02 14:48:18 2002
 */
/* Compiler settings for C:\home\bfriesen\ImageMagick\contrib\win32\ATL\ImageMagickObject\ImageMagickObject.idl:
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

#ifndef __ImageMagickObject_h__
#define __ImageMagickObject_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

#ifndef __IMagickImage_FWD_DEFINED__
#define __IMagickImage_FWD_DEFINED__
typedef interface IMagickImage IMagickImage;
#endif 	/* __IMagickImage_FWD_DEFINED__ */


#ifndef ___IMagickImageEvents_FWD_DEFINED__
#define ___IMagickImageEvents_FWD_DEFINED__
typedef interface _IMagickImageEvents _IMagickImageEvents;
#endif 	/* ___IMagickImageEvents_FWD_DEFINED__ */


#ifndef __MagickImage_FWD_DEFINED__
#define __MagickImage_FWD_DEFINED__

#ifdef __cplusplus
typedef class MagickImage MagickImage;
#else
typedef struct MagickImage MagickImage;
#endif /* __cplusplus */

#endif 	/* __MagickImage_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __IMagickImage_INTERFACE_DEFINED__
#define __IMagickImage_INTERFACE_DEFINED__

/* interface IMagickImage */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IMagickImage;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("7F670536-00AE-4EDF-B06F-13BD22B25624")
    IMagickImage : public IDispatch
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnStartPage( 
            /* [in] */ IUnknown __RPC_FAR *piUnk) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnEndPage( void) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Count( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [vararg][helpstring][id] */ HRESULT STDMETHODCALLTYPE Add( 
            /* [out][in] */ SAFEARRAY __RPC_FAR * __RPC_FAR *pArrayVar,
            /* [retval][out] */ VARIANT __RPC_FAR *pVar2) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Remove( 
            /* [in] */ VARIANT varIndex) = 0;
        
        virtual /* [vararg][helpstring][id] */ HRESULT STDMETHODCALLTYPE Convert( 
            /* [out][in] */ SAFEARRAY __RPC_FAR * __RPC_FAR *pArrayVar,
            /* [retval][out] */ VARIANT __RPC_FAR *pVar2) = 0;
        
        virtual /* [vararg][helpstring][id] */ HRESULT STDMETHODCALLTYPE Composite( 
            /* [out][in] */ SAFEARRAY __RPC_FAR * __RPC_FAR *pArrayVar,
            /* [retval][out] */ VARIANT __RPC_FAR *pVar2) = 0;
        
        virtual /* [vararg][helpstring][id] */ HRESULT STDMETHODCALLTYPE Montage( 
            /* [out][in] */ SAFEARRAY __RPC_FAR * __RPC_FAR *pArrayVar,
            /* [retval][out] */ VARIANT __RPC_FAR *pVar2) = 0;
        
        virtual /* [vararg][helpstring][id] */ HRESULT STDMETHODCALLTYPE Mogrify( 
            /* [out][in] */ SAFEARRAY __RPC_FAR * __RPC_FAR *pArrayVar,
            /* [retval][out] */ VARIANT __RPC_FAR *pVar2) = 0;
        
        virtual /* [vararg][helpstring][id] */ HRESULT STDMETHODCALLTYPE Identify( 
            /* [out][in] */ SAFEARRAY __RPC_FAR * __RPC_FAR *pArrayVar,
            /* [retval][out] */ VARIANT __RPC_FAR *pVar2) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get__NewEnum( 
            /* [retval][out] */ LPUNKNOWN __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Item( 
            /* [in] */ VARIANT varIndex,
            /* [retval][out] */ VARIANT __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Messages( 
            /* [retval][out] */ VARIANT __RPC_FAR *pVal) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IMagickImageVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IMagickImage __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IMagickImage __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IMagickImage __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IMagickImage __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IMagickImage __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IMagickImage __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IMagickImage __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OnStartPage )( 
            IMagickImage __RPC_FAR * This,
            /* [in] */ IUnknown __RPC_FAR *piUnk);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OnEndPage )( 
            IMagickImage __RPC_FAR * This);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Count )( 
            IMagickImage __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [vararg][helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Add )( 
            IMagickImage __RPC_FAR * This,
            /* [out][in] */ SAFEARRAY __RPC_FAR * __RPC_FAR *pArrayVar,
            /* [retval][out] */ VARIANT __RPC_FAR *pVar2);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Remove )( 
            IMagickImage __RPC_FAR * This,
            /* [in] */ VARIANT varIndex);
        
        /* [vararg][helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Convert )( 
            IMagickImage __RPC_FAR * This,
            /* [out][in] */ SAFEARRAY __RPC_FAR * __RPC_FAR *pArrayVar,
            /* [retval][out] */ VARIANT __RPC_FAR *pVar2);
        
        /* [vararg][helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Composite )( 
            IMagickImage __RPC_FAR * This,
            /* [out][in] */ SAFEARRAY __RPC_FAR * __RPC_FAR *pArrayVar,
            /* [retval][out] */ VARIANT __RPC_FAR *pVar2);
        
        /* [vararg][helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Montage )( 
            IMagickImage __RPC_FAR * This,
            /* [out][in] */ SAFEARRAY __RPC_FAR * __RPC_FAR *pArrayVar,
            /* [retval][out] */ VARIANT __RPC_FAR *pVar2);
        
        /* [vararg][helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Mogrify )( 
            IMagickImage __RPC_FAR * This,
            /* [out][in] */ SAFEARRAY __RPC_FAR * __RPC_FAR *pArrayVar,
            /* [retval][out] */ VARIANT __RPC_FAR *pVar2);
        
        /* [vararg][helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Identify )( 
            IMagickImage __RPC_FAR * This,
            /* [out][in] */ SAFEARRAY __RPC_FAR * __RPC_FAR *pArrayVar,
            /* [retval][out] */ VARIANT __RPC_FAR *pVar2);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get__NewEnum )( 
            IMagickImage __RPC_FAR * This,
            /* [retval][out] */ LPUNKNOWN __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Item )( 
            IMagickImage __RPC_FAR * This,
            /* [in] */ VARIANT varIndex,
            /* [retval][out] */ VARIANT __RPC_FAR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Messages )( 
            IMagickImage __RPC_FAR * This,
            /* [retval][out] */ VARIANT __RPC_FAR *pVal);
        
        END_INTERFACE
    } IMagickImageVtbl;

    interface IMagickImage
    {
        CONST_VTBL struct IMagickImageVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMagickImage_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMagickImage_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMagickImage_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMagickImage_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMagickImage_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMagickImage_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMagickImage_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMagickImage_OnStartPage(This,piUnk)	\
    (This)->lpVtbl -> OnStartPage(This,piUnk)

#define IMagickImage_OnEndPage(This)	\
    (This)->lpVtbl -> OnEndPage(This)

#define IMagickImage_get_Count(This,pVal)	\
    (This)->lpVtbl -> get_Count(This,pVal)

#define IMagickImage_Add(This,pArrayVar,pVar2)	\
    (This)->lpVtbl -> Add(This,pArrayVar,pVar2)

#define IMagickImage_Remove(This,varIndex)	\
    (This)->lpVtbl -> Remove(This,varIndex)

#define IMagickImage_Convert(This,pArrayVar,pVar2)	\
    (This)->lpVtbl -> Convert(This,pArrayVar,pVar2)

#define IMagickImage_Composite(This,pArrayVar,pVar2)	\
    (This)->lpVtbl -> Composite(This,pArrayVar,pVar2)

#define IMagickImage_Montage(This,pArrayVar,pVar2)	\
    (This)->lpVtbl -> Montage(This,pArrayVar,pVar2)

#define IMagickImage_Mogrify(This,pArrayVar,pVar2)	\
    (This)->lpVtbl -> Mogrify(This,pArrayVar,pVar2)

#define IMagickImage_Identify(This,pArrayVar,pVar2)	\
    (This)->lpVtbl -> Identify(This,pArrayVar,pVar2)

#define IMagickImage_get__NewEnum(This,pVal)	\
    (This)->lpVtbl -> get__NewEnum(This,pVal)

#define IMagickImage_get_Item(This,varIndex,pVal)	\
    (This)->lpVtbl -> get_Item(This,varIndex,pVal)

#define IMagickImage_get_Messages(This,pVal)	\
    (This)->lpVtbl -> get_Messages(This,pVal)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IMagickImage_OnStartPage_Proxy( 
    IMagickImage __RPC_FAR * This,
    /* [in] */ IUnknown __RPC_FAR *piUnk);


void __RPC_STUB IMagickImage_OnStartPage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMagickImage_OnEndPage_Proxy( 
    IMagickImage __RPC_FAR * This);


void __RPC_STUB IMagickImage_OnEndPage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IMagickImage_get_Count_Proxy( 
    IMagickImage __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IMagickImage_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [vararg][helpstring][id] */ HRESULT STDMETHODCALLTYPE IMagickImage_Add_Proxy( 
    IMagickImage __RPC_FAR * This,
    /* [out][in] */ SAFEARRAY __RPC_FAR * __RPC_FAR *pArrayVar,
    /* [retval][out] */ VARIANT __RPC_FAR *pVar2);


void __RPC_STUB IMagickImage_Add_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IMagickImage_Remove_Proxy( 
    IMagickImage __RPC_FAR * This,
    /* [in] */ VARIANT varIndex);


void __RPC_STUB IMagickImage_Remove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [vararg][helpstring][id] */ HRESULT STDMETHODCALLTYPE IMagickImage_Convert_Proxy( 
    IMagickImage __RPC_FAR * This,
    /* [out][in] */ SAFEARRAY __RPC_FAR * __RPC_FAR *pArrayVar,
    /* [retval][out] */ VARIANT __RPC_FAR *pVar2);


void __RPC_STUB IMagickImage_Convert_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [vararg][helpstring][id] */ HRESULT STDMETHODCALLTYPE IMagickImage_Composite_Proxy( 
    IMagickImage __RPC_FAR * This,
    /* [out][in] */ SAFEARRAY __RPC_FAR * __RPC_FAR *pArrayVar,
    /* [retval][out] */ VARIANT __RPC_FAR *pVar2);


void __RPC_STUB IMagickImage_Composite_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [vararg][helpstring][id] */ HRESULT STDMETHODCALLTYPE IMagickImage_Montage_Proxy( 
    IMagickImage __RPC_FAR * This,
    /* [out][in] */ SAFEARRAY __RPC_FAR * __RPC_FAR *pArrayVar,
    /* [retval][out] */ VARIANT __RPC_FAR *pVar2);


void __RPC_STUB IMagickImage_Montage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [vararg][helpstring][id] */ HRESULT STDMETHODCALLTYPE IMagickImage_Mogrify_Proxy( 
    IMagickImage __RPC_FAR * This,
    /* [out][in] */ SAFEARRAY __RPC_FAR * __RPC_FAR *pArrayVar,
    /* [retval][out] */ VARIANT __RPC_FAR *pVar2);


void __RPC_STUB IMagickImage_Mogrify_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [vararg][helpstring][id] */ HRESULT STDMETHODCALLTYPE IMagickImage_Identify_Proxy( 
    IMagickImage __RPC_FAR * This,
    /* [out][in] */ SAFEARRAY __RPC_FAR * __RPC_FAR *pArrayVar,
    /* [retval][out] */ VARIANT __RPC_FAR *pVar2);


void __RPC_STUB IMagickImage_Identify_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IMagickImage_get__NewEnum_Proxy( 
    IMagickImage __RPC_FAR * This,
    /* [retval][out] */ LPUNKNOWN __RPC_FAR *pVal);


void __RPC_STUB IMagickImage_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IMagickImage_get_Item_Proxy( 
    IMagickImage __RPC_FAR * This,
    /* [in] */ VARIANT varIndex,
    /* [retval][out] */ VARIANT __RPC_FAR *pVal);


void __RPC_STUB IMagickImage_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IMagickImage_get_Messages_Proxy( 
    IMagickImage __RPC_FAR * This,
    /* [retval][out] */ VARIANT __RPC_FAR *pVal);


void __RPC_STUB IMagickImage_get_Messages_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IMagickImage_INTERFACE_DEFINED__ */



#ifndef __IMAGEMAGICKOBJECTLib_LIBRARY_DEFINED__
#define __IMAGEMAGICKOBJECTLib_LIBRARY_DEFINED__

/* library IMAGEMAGICKOBJECTLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_IMAGEMAGICKOBJECTLib;

#ifndef ___IMagickImageEvents_DISPINTERFACE_DEFINED__
#define ___IMagickImageEvents_DISPINTERFACE_DEFINED__

/* dispinterface _IMagickImageEvents */
/* [helpstring][uuid] */ 


EXTERN_C const IID DIID__IMagickImageEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("01834743-E151-45C9-9C43-2FC80114E539")
    _IMagickImageEvents : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct _IMagickImageEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            _IMagickImageEvents __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            _IMagickImageEvents __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            _IMagickImageEvents __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            _IMagickImageEvents __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            _IMagickImageEvents __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            _IMagickImageEvents __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            _IMagickImageEvents __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        END_INTERFACE
    } _IMagickImageEventsVtbl;

    interface _IMagickImageEvents
    {
        CONST_VTBL struct _IMagickImageEventsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define _IMagickImageEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define _IMagickImageEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define _IMagickImageEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define _IMagickImageEvents_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define _IMagickImageEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define _IMagickImageEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define _IMagickImageEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* ___IMagickImageEvents_DISPINTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_MagickImage;

#ifdef __cplusplus

class DECLSPEC_UUID("5630BE5A-3F5F-4BCA-A511-AD6A6386CAC1")
MagickImage;
#endif
#endif /* __IMAGEMAGICKOBJECTLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  LPSAFEARRAY_UserSize(     unsigned long __RPC_FAR *, unsigned long            , LPSAFEARRAY __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  LPSAFEARRAY_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, LPSAFEARRAY __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  LPSAFEARRAY_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, LPSAFEARRAY __RPC_FAR * ); 
void                      __RPC_USER  LPSAFEARRAY_UserFree(     unsigned long __RPC_FAR *, LPSAFEARRAY __RPC_FAR * ); 

unsigned long             __RPC_USER  VARIANT_UserSize(     unsigned long __RPC_FAR *, unsigned long            , VARIANT __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  VARIANT_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, VARIANT __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  VARIANT_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, VARIANT __RPC_FAR * ); 
void                      __RPC_USER  VARIANT_UserFree(     unsigned long __RPC_FAR *, VARIANT __RPC_FAR * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
