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

// Geometry.h : Declaration of the CGeometry

#ifndef __GEOMETRY_H_
#define __GEOMETRY_H_

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CGeometry
class ATL_NO_VTABLE CGeometry : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CGeometry, &CLSID_Geometry>,
	public IDispatchImpl<IGeometry, &IID_IGeometry, &LIBID_MAGICKCOMLib>
{
public:
	CGeometry()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_GEOMETRY)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CGeometry)
	COM_INTERFACE_ENTRY(IGeometry)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

// IGeometry
public:
	STDMETHOD(Geometry)(/*[in]*/ BSTR PSSizeNick, /*[out, retval]*/ BSTR *pVal);
	STDMETHOD(IsValid)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(get_Less)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_Less)(/*[in]*/ VARIANT_BOOL newVal);
	STDMETHOD(get_Greater)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_Greater)(/*[in]*/ VARIANT_BOOL newVal);
	STDMETHOD(get_Aspect)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_Aspect)(/*[in]*/ VARIANT_BOOL newVal);
	STDMETHOD(get_Percent)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_Percent)(/*[in]*/ VARIANT_BOOL newVal);
	STDMETHOD(get_YNegative)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_YNegative)(/*[in]*/ VARIANT_BOOL newVal);
	STDMETHOD(get_XNegative)(/*[out, retval]*/ VARIANT_BOOL *pVal);
	STDMETHOD(put_XNegative)(/*[in]*/ VARIANT_BOOL newVal);
	STDMETHOD(get_YOff)(/*[out, retval]*/ unsigned int *pVal);
	STDMETHOD(put_YOff)(/*[in]*/ unsigned int newVal);
	STDMETHOD(get_XOff)(/*[out, retval]*/ unsigned int *pVal);
	STDMETHOD(put_XOff)(/*[in]*/ unsigned int newVal);
	STDMETHOD(get_Height)(/*[out, retval]*/ unsigned int *pVal);
	STDMETHOD(put_Height)(/*[in]*/ unsigned int newVal);
	STDMETHOD(get_Width)(/*[out, retval]*/ unsigned int *pVal);
	STDMETHOD(put_Width)(/*[in]*/ unsigned int newVal);
};

#endif //__GEOMETRY_H_
