// class ATL_NO_VTABLE CSomeCollection : 
//  public CComObjectRootEx<CComSingleThreadModel>,
//  public CComCollect<ISomeCollection, &IID_ISomeCollection,&LIBID_SomeLib, CSomeItem, 
//                     &CLSID_SomeCollection, ISomeInterface, &IID_ISomeInterface>
//
// If all you need are the standard Add, Remove, Item, get_Count, and get__NewEnum methods,
// then you do not need to provide any other implementation.  Just make sure you declare
// the interface for your collection class in an IDL file.  Here's the example for the
// above declaration:
//
// interface ISomeCollection : IDispatch
// {
//      [propget, id(1)] HRESULT Count([out, retval] long *pVal);
//      [id(2)] HRESULT Add([in] ISPCChart* inItem);
//      [id(3)] HRESULT Remove([in] long inIndex);
//      [propget, id(DISPID_VALUE)] HRESULT Item([in] long inIndex,
//                                               [out, retval] ISPCChart** outChart);
//      [propget, id(DISPID_NEWENUM)] HRESULT _NewEnum([out, retval]LPUNKNOWN *pVal);
//  };


//
//&LIBID_SomeLib

//    public CComCollect<IImageCollection, &IID_IImageCollection,&LIBID_MAGICKCOMLib, CSomeItem, 
//                       &CLSID_ImageCollection, IImageCollectionInterface, &IID_IImageCollectionInterface>

/* Commented out
#include "CComCollect.h"
#include "Image.h"

interface IImageCollection;

class ATL_NO_VTABLE CImageCollection : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCollect<IImageCollection, &IID_IImageCollection,&LIBID_MAGICKCOMLib, CImage, 
                       &CLSID_Image, IImageCollection, &IID_IImageCollection>


*/

//class ATL_NO_VTABLE CImageCollection : 
//    public CComObjectRootEx<CComSingleThreadModel>,
//    public CComCollect<IImageCollection, &IID_IImageCollection,&LIBID_MAGICKCOMLib, CImage, 
//                       &CLSID_ImageCollection, IImageCollectionInterface, &IID_IImageCollectionInterface>
