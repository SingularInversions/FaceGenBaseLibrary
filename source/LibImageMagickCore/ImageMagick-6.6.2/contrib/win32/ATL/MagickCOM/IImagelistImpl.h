	#ifndef _IIMAGELISTIMPL_H
#define _IIMAGELISTIMPL_H
#include "cltnadaptors.h"
#include <list>
using namespace std;
typedef CComEnumOnSTL<IEnumVARIANT, &IID_IEnumVARIANT, VARIANT,
                        _CopyVariantFromAdaptItf<IImage>,
                        list < CAdapt< CComPtr<IImage> > > >
        CComEnumIImageVariantOnSTLlist;
#endif
