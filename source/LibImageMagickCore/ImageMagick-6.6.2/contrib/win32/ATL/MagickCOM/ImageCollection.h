// ImageCollection.h : Declaration of the CImageCollection

#ifndef __IMAGECOLLECTION_H_
#define __IMAGECOLLECTION_H_

#include "resource.h"       // main symbols

#include "IImagelistImpl.h"

typedef ICollectionOnSTLImpl<IDispatchImpl<IImageCollection, &IID_IImageCollection>,
                             list< CAdapt< CComPtr<IImage> > >,
                             IImage*,
                             _CopyItfFromAdaptItf<IImage>,
                             CComEnumIImageVariantOnSTLlist>
        IImageCollectionImpl;
/////////////////////////////////////////////////////////////////////////////
// CImageCollection
class ATL_NO_VTABLE CImageCollection : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CImageCollection, &CLSID_ImageCollection>,
	public IImageCollectionImpl
{
public:
	CImageCollection()
	{
	}

DECLARE_NO_REGISTRY()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CImageCollection)
	COM_INTERFACE_ENTRY(IImageCollection)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

// IImageCollection
public:
	STDMETHOD(Add)(IImage* NewVal)
	{
		m_coll.push_back(CComPtr<IImage>(NewVal));
		return S_OK;
	}
	STDMETHOD(Remove)(long Index, IImage** ppVal)
	{
		typedef list< CAdapt< CComPtr<IImage> > >::iterator it;
		long Count = 0;
		for ( it i = m_coll.begin(); i != m_coll.end(); i++,Count++ )
			if ( Count == Index - 1 )
			{
				m_coll.erase(i);
				break;
			}
		return S_OK;
	}
};

#endif //__IMAGECOLLECTION_H_
