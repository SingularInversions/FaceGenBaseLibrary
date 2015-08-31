// comvector.h: Wrapper around a single-dim SAFEARRAY and a lock on a SAFEARRAY
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 1999-2001, Chris Sells. All rights reserved. No warranties extended.
// Comments to csells@sellsbrothers.com.
/////////////////////////////////////////////////////////////////////////////
// CComVector is a wrapper around a single-dim SAFEARRAY.
// CComVectorData is a wrapper around a lock on a SAFEARRAY, acquired via
// SafeArrayAccessData.
//
// By splitting the two concepts into two classes, each can be managed
// separately, therefore avoiding the overhead and inconvenience of
// SafeArrayGetElement and SafeArrayPutElement.
//
// By specializing on one dimension, the language mapping is easy.
//
// Also added CComVectorItf, a SAFEARRAY of VARIANTs holding some interface.
/////////////////////////////////////////////////////////////////////////////
// History:
//  1/10/01:    Fixed an ASSERT bug in CComVector::Create. Thanks to
//              Phil Atkin [phil.atkin@synoptics.co.uk].
//  1/3/01:     Fixed a compile bug with the suggested usage pointed
//              out by Claus Michelsen [clausmic@btinternet.com].
//              Also removed the restriction on operator&, because SAs are
//              passed [in, out] all the time.
//  10/25/00:   Added support for VARIANTs holding SAFEARRAYs and
//              auto-coercing SAFEARRAYs of VARIANTS to a specific itf.
//  9/26/00:    Assert on VT_BYREF, VT_ARRAY, VT_NULL and VT_EMPTY in Create,
//              as per the docs (as pointed out by Brian Moelk
//              [bmoelk@answerlogic.com]).
//  7/29/00:    Added RawData method to get access to raw SA data.
//  5/13/99:    Initial release.
/////////////////////////////////////////////////////////////////////////////
// Usage:
/*
STDMETHODIMP CFoo::UseSafeArray(SAFEARRAY** ppsa) // [in]
{
    CComVectorData<BSTR> rg(*ppsa);
    if( !rg ) return E_UNEXPECTED;

    for( int i = 0; i < rg.Length(); ++i )
    {
        TCHAR   sz[32];
        wsprintf(sz, __T("rg[%d]= %S\n"), i, (rg[i] ? rg[i] : OLESTR("")));
        OutputDebugString(sz);
    }

    // Assume data member: 'CComVector<BSTR> m_rg;'
    m_rg = *ppsa;   // Cache a copy

    return S_OK;
}

STDMETHODIMP CFoo::GetSafeArray(SAFEARRAY** ppsa) // [in, out]
{
    CComVector<BSTR> v(100);
    if( !v ) return E_OUTOFMEMORY;

    CComVectorData<BSTR> rg(v);
    if( !rg ) return E_UNEXPECTED;

    for( int i = 0; i < 100; ++i )
    {
        char    sz[16];
        rg[i] = A2BSTR(itoa(i, sz, 10));
    }

    return v.DetachTo(ppsa);
}
*/

#pragma once
#ifndef INC_COMVECTOR
#define INC_COMVECTOR

#include <crtdbg.h>

#ifndef HR
#define HR(_ex) { HRESULT _hr = _ex; if( FAILED(_hr) ) return _hr; }
#endif

/////////////////////////////////////////////////////////////////////////////
// CComVector wraps a SAFEARRAY*

class CComVectorBase
{
public:
    CComVectorBase(const SAFEARRAY* psa) : m_psa(0)
    {
        if( psa ) Copy(psa);
    }

    CComVectorBase(const VARIANT& var) : m_psa(0)
    {
        if( var.vt | VT_ARRAY ) Copy(var.parray);
    }

    CComVectorBase(const CComVectorBase& v) : m_psa(0)
    {
        if( v.m_psa ) Copy(v.m_psa);
    }

    ~CComVectorBase()
    {
        Destroy();
    }

    HRESULT Copy(const SAFEARRAY* psa)
    {
        Destroy();

        if( psa )
        {
            _ASSERTE(SafeArrayGetDim(const_cast<SAFEARRAY*>(psa)) == 1);
            HR(SafeArrayCopy(const_cast<SAFEARRAY*>(psa), &m_psa));
        }

        return S_OK;
    }

    HRESULT Destroy()
    {
        if( !m_psa ) return S_OK;

        HRESULT hr = SafeArrayDestroy(m_psa);
        m_psa = 0;
        return hr;
    }

    void Attach(SAFEARRAY* psa)
    {
        Destroy();
        m_psa = psa;
    }

    SAFEARRAY* Detach()
    {
        SAFEARRAY*  psa = m_psa;
        m_psa = 0;
        return psa;
    }

    HRESULT DetachTo(/*[in, out]*/ SAFEARRAY** ppsa)
    {
        if( !ppsa ) HR(E_POINTER);

        // If the [in] array is already created, destroy it
        if( *ppsa ) HR(SafeArrayDestroy(*ppsa));

        *ppsa = m_psa;
        m_psa = 0;
        return S_OK;
    }

    HRESULT CopyTo(/*[in, out]*/ SAFEARRAY** ppsa)
    {
        if( !ppsa ) HR(E_POINTER);

        // If the [in] array is already created, destroy it
        if( *ppsa ) HR(SafeArrayDestroy(*ppsa));

        HR(SafeArrayCopy(m_psa, ppsa));
        return S_OK;
    }

    operator SAFEARRAY*()
    {
        return m_psa;
    }

    SAFEARRAY** operator&()
    {
        // NOTE: This isn't reasonable, as SAs are passed [in, out] all the time
        //_ASSERTE(!m_psa);
        return &m_psa;
    }

    long Length()
    {
        return Length(m_psa);
    }

public:
    static
    long Length(const SAFEARRAY* psa)
    {
        if( !psa ) return 0;
        _ASSERTE(SafeArrayGetDim(const_cast<SAFEARRAY*>(psa)) == 1);

        long    ub; SafeArrayGetUBound(const_cast<SAFEARRAY*>(psa), 1, &ub);
        long    lb; SafeArrayGetLBound(const_cast<SAFEARRAY*>(psa), 1, &lb);
        return ub - lb + 1;
    }

protected:
    template <typename T> VARTYPE VarType(T*);

    template<> VARTYPE VarType(LONG*) { return VT_I4; }
    template<> VARTYPE VarType(BYTE*) { return VT_UI1; }
    template<> VARTYPE VarType(SHORT*) { return VT_I2; }
    template<> VARTYPE VarType(FLOAT*) { return VT_R4; }
    template<> VARTYPE VarType(DOUBLE*) { return VT_R8; }
    template<> VARTYPE VarType(VARIANT_BOOL*) { return VT_BOOL; }
    template<> VARTYPE VarType(SCODE*) { return VT_ERROR; }
    template<> VARTYPE VarType(CY*) { return VT_CY; }
    template<> VARTYPE VarType(DATE*) { return VT_DATE; }
    template<> VARTYPE VarType(BSTR*) { return VT_BSTR; }
    template<> VARTYPE VarType(IUnknown **) { return VT_UNKNOWN; }
    template<> VARTYPE VarType(IDispatch **) { return VT_DISPATCH; }
    template<> VARTYPE VarType(SAFEARRAY **) { return VT_ARRAY; }
    template<> VARTYPE VarType(CHAR*) { return VT_I1; }
    template<> VARTYPE VarType(USHORT*) { return VT_UI2; }
    template<> VARTYPE VarType(ULONG*) { return VT_UI4; }
    template<> VARTYPE VarType(INT*) { return VT_INT; }
    template<> VARTYPE VarType(UINT*) { return VT_UINT; }
    template<> VARTYPE VarType(VARIANT*) { return VT_VARIANT; }

    template<> VARTYPE VarType(BYTE **) { return VT_BYREF|VT_UI1; }
    template<> VARTYPE VarType(SHORT **) { return VT_BYREF|VT_I2; }
    template<> VARTYPE VarType(LONG **) { return VT_BYREF|VT_I4; }
    template<> VARTYPE VarType(FLOAT **) { return VT_BYREF|VT_R4; }
    template<> VARTYPE VarType(DOUBLE **) { return VT_BYREF|VT_R8; }
    template<> VARTYPE VarType(VARIANT_BOOL **) { return VT_BYREF|VT_BOOL; }
    template<> VARTYPE VarType(SCODE **) { return VT_BYREF|VT_ERROR; }
    template<> VARTYPE VarType(CY **) { return VT_BYREF|VT_CY; }
    template<> VARTYPE VarType(DATE **) { return VT_BYREF|VT_DATE; }
    template<> VARTYPE VarType(BSTR **) { return VT_BYREF|VT_BSTR; }
    template<> VARTYPE VarType(IUnknown ***) { return VT_BYREF|VT_UNKNOWN; }
    template<> VARTYPE VarType(IDispatch ***) { return VT_BYREF|VT_DISPATCH; }
    template<> VARTYPE VarType(SAFEARRAY ***) { return VT_BYREF|VT_ARRAY; }
    template<> VARTYPE VarType(VARIANT **) { return VT_BYREF|VT_VARIANT; }
    template<> VARTYPE VarType(PVOID*) { return VT_BYREF; }
    template<> VARTYPE VarType(DECIMAL **) { return VT_BYREF|VT_DECIMAL; }
    template<> VARTYPE VarType(CHAR **) { return VT_BYREF|VT_I1; }
    template<> VARTYPE VarType(USHORT **) { return VT_BYREF|VT_UI2; }
    template<> VARTYPE VarType(ULONG **) { return VT_BYREF|VT_UI4; }
    template<> VARTYPE VarType(INT **) { return VT_BYREF|VT_INT; }
    template<> VARTYPE VarType(UINT **) { return VT_BYREF|VT_UINT; }
    template<> VARTYPE VarType(VARIANT **) { return VT_BYREF|VT_VARIANT; }

protected:
    SAFEARRAY*  m_psa;
};

template <typename T>
class CComVector : public CComVectorBase
{
public:
    CComVector(SAFEARRAY* psa = 0) : CComVectorBase(psa)
    {
    }

    CComVector(const CComVector& v) : CComVectorBase(v)
    {
    }

    CComVector(long celt, long nLowerBound = 0, bool bZeroMemory = true) : CComVectorBase(0)
    {
        Create(celt, nLowerBound, bZeroMemory);
    }

    // TODO: Hook up Copy policy classes
    /*
    CComVector(const T* rg, long celt) : m_psa(0)
    {
        Copy(rg, celt);
    }
    */

    CComVector& operator=(const SAFEARRAY* psa)
    {
        if( psa != m_psa ) Copy(psa);
        return *this;
    }

    CComVector& operator=(const CComVector& v)
    {
        if( &v != this ) Copy(v.m_psa);
        return *this;
    }

    HRESULT Create(long celt, long nLowerBound = 0, bool bZeroMemory = true)
    {
#ifdef _DEBUG
        VARTYPE vt = VarType((T*)0);
        _ASSERTE(((vt & VT_BYREF) == 0) && "Can't create a SA using VT_BYREF");
        _ASSERTE(((vt & VT_ARRAY) == 0) && "Can't create a SA using VT_ARRAY");
        _ASSERTE(((vt & VT_TYPEMASK) != VT_EMPTY) && "Can't create a SA using VT_EMPTY");
        _ASSERTE(((vt & VT_TYPEMASK) != VT_NULL) && "Can't create a SA using VT_NULL");
#endif

        SAFEARRAYBOUND  sab = { celt, nLowerBound };
        m_psa = SafeArrayCreate(VarType((T*)0), 1, &sab);
        if( !m_psa ) HR(E_OUTOFMEMORY);

        if( bZeroMemory )
        {
            T*      rg = 0;
            HR(SafeArrayAccessData(m_psa, (void**)&rg));
            ZeroMemory(rg, sizeof(T) * celt);
            SafeArrayUnaccessData(m_psa);
        }

        return S_OK;
    }

    T* RawData()
    {
        _ASSERTE(m_psa);
        return reinterpret_cast<T*>(m_psa->pvData);
    }

    // For some reason, VC6 won't bind to the base class's
    // implementation, so we do it ourselves
    HRESULT DetachTo(/*[in, out]*/ SAFEARRAY** ppsa)
    {
        return CComVectorBase::DetachTo(ppsa);
    }

    // For some reason, VC6 won't bind to the base class's
    // implementation, so we do it ourselves
    HRESULT CopyTo(/*[in, out]*/ SAFEARRAY** ppsa)
    {
        return CComVectorBase::CopyTo(ppsa);
    }

    HRESULT DetachTo(/*[out, retval]*/ VARIANT* pvar)
    {
        if( !pvar ) return E_POINTER;
        pvar->vt = VT_ARRAY | VarType((T*)0);
        pvar->parray = 0;
        return CComVectorBase::DetachTo(&pvar->parray);
    }

    HRESULT CopyTo(/*[out, retval]*/ VARIANT* pvar)
    {
        if( !pvar ) return E_POINTER;
        pvar->vt = VT_ARRAY | VarType((T*)0);
        pvar->parray = 0;
        return CComVectorBase::CopyTo(&pvar->parray);
    }
};

/////////////////////////////////////////////////////////////////////////////
// CComVectorData represents a lock on SAFEARRAY data

class CComVectorDataBase
{
public:
    CComVectorDataBase(SAFEARRAY* psa) : m_psa(0), m_pv(0), m_celt(0)
    {
        if( psa ) AccessData(psa);
    }

    CComVectorDataBase(VARIANT& var) : m_psa(0), m_pv(0), m_celt(0)
    {
        AccessData(var);
    }

    ~CComVectorDataBase()
    {
        UnaccessData();
    }

    HRESULT AccessData(SAFEARRAY* psa)
    {
        if( !psa || (SafeArrayGetDim(psa) != 1) ) return E_INVALIDARG;
        UnaccessData();

        HR(SafeArrayAccessData(psa, &m_pv));
        m_psa = psa;
        m_celt = CComVectorBase::Length(m_psa);
        return S_OK;
    }

    HRESULT AccessData(VARIANT& var)
    {
        if( !(var.vt & VT_ARRAY) ) return E_INVALIDARG;
        return AccessData(var.parray);
    }

    HRESULT UnaccessData()
    {
        if( m_psa && m_pv ) HR(SafeArrayUnaccessData(m_psa));
        m_psa = 0;
        m_pv = 0;
        m_celt = 0;
        return S_OK;
    }

    long Length()
    {
        return m_celt;
    }

    operator bool()
    {
        return (m_pv ? true : false);
    }

protected:
    SAFEARRAY*  m_psa;
    void*       m_pv;
    long        m_celt;
};

template <typename T>
class CComVectorData : public CComVectorDataBase
{
public:
    CComVectorData(SAFEARRAY* psa = 0) : CComVectorDataBase(psa)
    {
    }

    CComVectorData(VARIANT& var) : CComVectorDataBase(var)
    {
    }

    T& operator[](long n)
    {
#ifdef _DEBUG
        if( n < 0 || n >= m_celt )
        {
            _ASSERTE(false && "Accessing data out of bounds");
        }
#endif
        return reinterpret_cast<T*>(m_pv)[n];
    }

    /* This conflicts with operator[]
    operator T*()
    {
        return reinterpret_cast<T*>(m_pv);
    }
    */
};

// A SAFEARRAY of VARIANTs holding some interface
template <typename T>
class CComVectorItf : public CComVectorData<VARIANT>
{
public:
    CComVectorItf(SAFEARRAY* psa = 0) : CComVectorData<VARIANT>(psa)
    {
    }

    CComVectorItf(VARIANT& var) : CComVectorData<VARIANT>(var)
    {
    }

    HRESULT QueryInterface(long n, T** pp)
    {
        VARIANT&    var = (*this)[n];
        if( !((var.vt & VT_UNKNOWN) || (var.vt & VT_DISPATCH)) ) return E_NOINTERFACE;
        return var.punkVal->QueryInterface(pp);
    }
};

#endif  // INC_COMVECTOR
