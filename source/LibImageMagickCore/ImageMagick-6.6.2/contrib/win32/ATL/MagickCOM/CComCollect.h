// Snagged this code from http://www.widgetware.com/ATLFAQ_Collections.htm
// The code came from Chris Brooks (clbrooks@micron.com)

#include <vector>


////////////////////////////////////////////////////////////////////////
/////
// CComCollect
// 
// icontainer:      The interface for the container
// container_iid:   The IID for the container interface
// libid:           The GUID for the library
// container:       The implementation class for the container
// icontained:      The interface for the contained interface
// contained_iid:   The IID for the contained interface
//
// Usage:
// Derive your collection class from this and CComObjectRootEx.  Since this template class
// inherits from CComCoClass, IDispatchImpl, and ISupportErrorInfo, you should not inherit
// from those in your concrete class.  Here's an example declaration:
//
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
// You are free to add additional accessor/builder methods (note that the collection 
// implementation is exposed.  Just declare it in your header and the IDL and provide an
// implementation.
//

template <class icontainer, const IID* container_iid, const GUID* libid,
class container, 
          const CLSID* container_clsid, class icontained, const IID*
contained_iid>
class ATL_NO_VTABLE CComCollect :
    public CComCoClass<container, container_clsid>,
public IDispatchImpl<icontainer, container_iid, libid>,
    public ISupportErrorInfo
{
public:
    CComCollect() {}


// ISPCChartSpecs
public:
    STDMETHOD(RemoveAll)();
    STDMETHOD(Add)(/*[in]*/ icontained* inItem);
    STDMETHOD(get__NewEnum)(/*[out, retval]*/ LPUNKNOWN *pVal);
    STDMETHOD(get_Item)(/*[in]*/ long inIndex, /*[out, retval]*/
icontained** outChart);
    STDMETHOD(get_Count)(/*[out, retval]*/ long *pVal);
    STDMETHOD(Remove)(/*[in]*/ long inIndex);

    // ISupportsErrorInfo
    STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

    // Helper functions
    typedef std::vector<CComVariant> collection;
    typedef collection::iterator iterator;
    iterator begin();
    iterator end();
    
protected:
    collection mCollect;
};

template <class icontainer, const IID* container_iid, const GUID* libid,
class container, const CLSID* container_clsid, class icontained, const IID* contained_iid>

STDMETHODIMP CComCollect<icontainer, container_iid, libid, container,
                         container_clsid, icontained, contained_iid>
                        ::InterfaceSupportsErrorInfo(REFIID riid)
{
   static const IID* arr[] = { container_iid, };
   for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
   {
      if (InlineIsEqualGUID(*arr[i],riid))
         return S_OK;
   }
   return S_FALSE;
}

template <class icontainer, const IID* container_iid, const GUID* libid,
          class container, const CLSID* container_clsid, class icontained, const IID* contained_iid>
CComCollect<icontainer, container_iid, libid, container,
            container_clsid, icontained, contained_iid>::iterator 
CComCollect<icontainer, container_iid, libid, container,
            container_clsid, icontained, contained_iid>::begin()
{
    return mCollect.begin();
}

template <class icontainer, const IID* container_iid, const GUID* libid,
class container, const CLSID* container_clsid, class icontained, const IID* contained_iid>
CComCollect<icontainer, container_iid, libid, container,
            container_clsid, icontained, contained_iid>::iterator
CComCollect<icontainer, container_iid, libid, container,
            container_clsid, icontained, contained_iid>::end()
{
    return mCollect.end();
}

// Returns the number of elements in the collection
template <class icontainer, const IID* container_iid, const GUID* libid,
class container, const CLSID* container_clsid, class icontained, const IID* contained_iid>
STDMETHODIMP CComCollect<icontainer, container_iid, libid, container,
                         container_clsid, icontained, contained_iid>::get_Count(long * pVal)
{
    // Make sure we didn't get a null pointer passed in
    if (!pVal)
        return E_POINTER;

    // Get the size from the collection
    *pVal = mCollect.size();

    return S_OK;
}

// Returns a single item corresponding to the index
template <class icontainer, const IID* container_iid, const GUID* libid,
class container, const CLSID* container_clsid, class icontained, const IID* contained_iid>
STDMETHODIMP CComCollect<icontainer, container_iid, libid, container,
                         container_clsid, icontained, contained_iid>::get_Item(long inIndex, icontained** outContained)
{
    // Make sure we didn't get a null pointer passed in
    if (!outContained)
        return E_POINTER;

    // Make sure the index is within range (index is 1-based)
    if (inIndex < 1 || inIndex > mCollect.size())
        return E_INVALIDARG;

    // Get the variant out of the collection
    CComVariant& var = mCollect[inIndex-1];

    // Make sure we've got an element
    HRESULT hr = var.pdispVal->QueryInterface(*contained_iid, (void**)outContained);
    if (FAILED(hr))
        return E_UNEXPECTED;

    return S_OK;
}

template <class icontainer, const IID* container_iid, const GUID* libid,
class container, const CLSID* container_clsid, class icontained, const IID* contained_iid>
STDMETHODIMP CComCollect<icontainer, container_iid, libid, container,
                         container_clsid, icontained, contained_iid>::get__NewEnum(LPUNKNOWN * pVal)
{
    // Make sure we didn't get a null pointer passed in
    if (!pVal)
        return E_POINTER;

    *pVal = 0;

    // Create the enumeration object
    typedef CComObject<CComEnum<IEnumVARIANT, &IID_IEnumVARIANT,
                                VARIANT, _Copy<VARIANT> > > enumVar;
    enumVar* pEnum = new enumVar;
    if (!pEnum)
        return E_OUTOFMEMORY;

    // Initialize with the chart collection iterators
    HRESULT hr = pEnum->Init(mCollect.begin(), mCollect.end(), NULL,AtlFlagCopy);
    if (SUCCEEDED(hr))
        hr = pEnum->QueryInterface(IID_IEnumVARIANT, (void**)pVal);
    if (FAILED(hr))
        delete pEnum;
    return hr;
}

template <class icontainer, const IID* container_iid, const GUID* libid,
class container, const CLSID* container_clsid, class icontained, const IID* contained_iid>
STDMETHODIMP CComCollect<icontainer, container_iid, libid, container,
                         container_clsid, icontained, contained_iid>::Add(icontained* inItem)
{
    HRESULT hr = S_OK;
    icontained* pContained;

    // Make sure we've got an element
    hr = inItem->QueryInterface(*contained_iid, (void **)&pContained);
    if (FAILED(hr))
        return E_UNEXPECTED;

    // Add the variant (dispatch) to the collection
    mDirty = TRUE;
    mCollect.push_back(CComVariant(inItem));
    pContained->Release();

    return hr;
}

// Removes an item at an integer index 
template <class icontainer, const IID* container_iid, const GUID* libid,
class container, const CLSID* container_clsid, class icontained, const IID* contained_iid>
STDMETHODIMP CComCollect<icontainer, container_iid, libid, container,
                         container_clsid, icontained, contained_iid>::Remove(long inIndex)
{
    HRESULT hr = S_OK;

    // Make sure the index is within range (index is 1-based)
    if (inIndex < 1 || inIndex > mCollect.size())
        return E_INVALIDARG;

    mDirty = TRUE;
    mCollect.erase(mCollect.begin() + inIndex - 1);

    return hr;
}

/////////////////////////////////////////////////////////////
//
// Remove all elements from the collection
//
template <class icontainer, const IID* container_iid, const GUID* libid,
class container, const CLSID* container_clsid, class icontained, const IID* contained_iid>
STDMETHODIMP CComCollect<icontainer, container_iid, libid, container,
                         container_clsid, icontained, contained_iid>::RemoveAll()
{
    mCollect.erase(mCollect.begin(), mCollect.end());
    return S_OK;
}

//