
#pragma warning( disable: 4049 )  /* more than 64k source lines */

/* this ALWAYS GENERATED file contains the IIDs and CLSIDs */

/* link this file in with the server and any clients */


 /* File created by MIDL compiler version 6.00.0347 */
/* at Sun Nov 10 17:50:54 2002
 */
/* Compiler settings for \home\bfriesen\ImageMagick\contrib\win32\ATL\ImageMagickObject\ImageMagickObject.idl:
    Oicf, W1, Zp8, env=Win32 (32b run)
    protocol : dce , ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

#if !defined(_M_IA64) && !defined(_M_AMD64)

#ifdef __cplusplus
extern "C"{
#endif 


#include <rpc.h>
#include <rpcndr.h>

#ifdef _MIDL_USE_GUIDDEF_

#ifndef INITGUID
#define INITGUID
#include <guiddef.h>
#undef INITGUID
#else
#include <guiddef.h>
#endif

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8)

#else // !_MIDL_USE_GUIDDEF_

#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}

#endif !_MIDL_USE_GUIDDEF_

MIDL_DEFINE_GUID(IID, IID_IMagickImage,0x7F670536,0x00AE,0x4EDF,0xB0,0x6F,0x13,0xBD,0x22,0xB2,0x56,0x24);


MIDL_DEFINE_GUID(IID, LIBID_IMAGEMAGICKOBJECTLib,0xA2A30AD0,0x24E7,0x450F,0x8B,0x51,0xF1,0x54,0xBC,0x0C,0xD8,0x3A);


MIDL_DEFINE_GUID(IID, DIID__IMagickImageEvents,0x01834743,0xE151,0x45C9,0x9C,0x43,0x2F,0xC8,0x01,0x14,0xE5,0x39);


MIDL_DEFINE_GUID(CLSID, CLSID_MagickImage,0x5630BE5A,0x3F5F,0x4BCA,0xA5,0x11,0xAD,0x6A,0x63,0x86,0xCA,0xC1);

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif



#endif /* !defined(_M_IA64) && !defined(_M_AMD64)*/

