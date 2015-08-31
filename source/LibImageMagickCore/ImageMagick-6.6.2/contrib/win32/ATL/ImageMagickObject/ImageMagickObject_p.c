
#pragma warning( disable: 4049 )  /* more than 64k source lines */

/* this ALWAYS GENERATED file contains the proxy stub code */


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
#define USE_STUBLESS_PROXY


/* verify that the <rpcproxy.h> version is high enough to compile this file*/
#ifndef __REDQ_RPCPROXY_H_VERSION__
#define __REQUIRED_RPCPROXY_H_VERSION__ 440
#endif


#include "rpcproxy.h"
#ifndef __RPCPROXY_H_VERSION__
#error this stub requires an updated version of <rpcproxy.h>
#endif // __RPCPROXY_H_VERSION__


#include "ImageMagickObject_h.h"

#define TYPE_FORMAT_STRING_SIZE   1061                              
#define PROC_FORMAT_STRING_SIZE   401                               
#define TRANSMIT_AS_TABLE_SIZE    0            
#define WIRE_MARSHAL_TABLE_SIZE   2            

typedef struct _MIDL_TYPE_FORMAT_STRING
    {
    short          Pad;
    unsigned char  Format[ TYPE_FORMAT_STRING_SIZE ];
    } MIDL_TYPE_FORMAT_STRING;

typedef struct _MIDL_PROC_FORMAT_STRING
    {
    short          Pad;
    unsigned char  Format[ PROC_FORMAT_STRING_SIZE ];
    } MIDL_PROC_FORMAT_STRING;


static RPC_SYNTAX_IDENTIFIER  _RpcTransferSyntax = 
{{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}};


extern const MIDL_TYPE_FORMAT_STRING __MIDL_TypeFormatString;
extern const MIDL_PROC_FORMAT_STRING __MIDL_ProcFormatString;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO IMagickImage_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO IMagickImage_ProxyInfo;


extern const USER_MARSHAL_ROUTINE_QUADRUPLE UserMarshalRoutines[ WIRE_MARSHAL_TABLE_SIZE ];

#if !defined(__RPC_WIN32__)
#error  Invalid build platform for this stub.
#endif

#if !(TARGET_IS_NT40_OR_LATER)
#error You need a Windows NT 4.0 or later to run this stub because it uses these features:
#error   -Oif or -Oicf, [wire_marshal] or [user_marshal] attribute.
#error However, your C/C++ compilation flags indicate you intend to run this app on earlier systems.
#error This app will die there with the RPC_X_WRONG_STUB_VERSION error.
#endif


static const MIDL_PROC_FORMAT_STRING __MIDL_ProcFormatString =
    {
        0,
        {

	/* Procedure OnStartPage */

			0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/*  2 */	NdrFcLong( 0x0 ),	/* 0 */
/*  6 */	NdrFcShort( 0x7 ),	/* 7 */
/*  8 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 10 */	NdrFcShort( 0x0 ),	/* 0 */
/* 12 */	NdrFcShort( 0x8 ),	/* 8 */
/* 14 */	0x6,		/* Oi2 Flags:  clt must size, has return, */
			0x2,		/* 2 */

	/* Parameter piUnk */

/* 16 */	NdrFcShort( 0xb ),	/* Flags:  must size, must free, in, */
/* 18 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 20 */	NdrFcShort( 0x2 ),	/* Type Offset=2 */

	/* Return value */

/* 22 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 24 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 26 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure OnEndPage */

/* 28 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 30 */	NdrFcLong( 0x0 ),	/* 0 */
/* 34 */	NdrFcShort( 0x8 ),	/* 8 */
/* 36 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 38 */	NdrFcShort( 0x0 ),	/* 0 */
/* 40 */	NdrFcShort( 0x8 ),	/* 8 */
/* 42 */	0x4,		/* Oi2 Flags:  has return, */
			0x1,		/* 1 */

	/* Return value */

/* 44 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 46 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 48 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure get_Count */

/* 50 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 52 */	NdrFcLong( 0x0 ),	/* 0 */
/* 56 */	NdrFcShort( 0x9 ),	/* 9 */
/* 58 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 60 */	NdrFcShort( 0x0 ),	/* 0 */
/* 62 */	NdrFcShort( 0x24 ),	/* 36 */
/* 64 */	0x4,		/* Oi2 Flags:  has return, */
			0x2,		/* 2 */

	/* Parameter pVal */

/* 66 */	NdrFcShort( 0x2150 ),	/* Flags:  out, base type, simple ref, srv alloc size=8 */
/* 68 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 70 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Return value */

/* 72 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 74 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 76 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure Add */

/* 78 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 80 */	NdrFcLong( 0x0 ),	/* 0 */
/* 84 */	NdrFcShort( 0xa ),	/* 10 */
/* 86 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 88 */	NdrFcShort( 0x0 ),	/* 0 */
/* 90 */	NdrFcShort( 0x8 ),	/* 8 */
/* 92 */	0x7,		/* Oi2 Flags:  srv must size, clt must size, has return, */
			0x3,		/* 3 */

	/* Parameter pArrayVar */

/* 94 */	NdrFcShort( 0x11b ),	/* Flags:  must size, must free, in, out, simple ref, */
/* 96 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 98 */	NdrFcShort( 0x3f6 ),	/* Type Offset=1014 */

	/* Parameter pVar2 */

/* 100 */	NdrFcShort( 0x4113 ),	/* Flags:  must size, must free, out, simple ref, srv alloc size=16 */
/* 102 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 104 */	NdrFcShort( 0x408 ),	/* Type Offset=1032 */

	/* Return value */

/* 106 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 108 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 110 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure Remove */

/* 112 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 114 */	NdrFcLong( 0x0 ),	/* 0 */
/* 118 */	NdrFcShort( 0xb ),	/* 11 */
/* 120 */	NdrFcShort( 0x18 ),	/* x86 Stack size/offset = 24 */
/* 122 */	NdrFcShort( 0x0 ),	/* 0 */
/* 124 */	NdrFcShort( 0x8 ),	/* 8 */
/* 126 */	0x6,		/* Oi2 Flags:  clt must size, has return, */
			0x2,		/* 2 */

	/* Parameter varIndex */

/* 128 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 130 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 132 */	NdrFcShort( 0x416 ),	/* Type Offset=1046 */

	/* Return value */

/* 134 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 136 */	NdrFcShort( 0x14 ),	/* x86 Stack size/offset = 20 */
/* 138 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure Convert */

/* 140 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 142 */	NdrFcLong( 0x0 ),	/* 0 */
/* 146 */	NdrFcShort( 0xc ),	/* 12 */
/* 148 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 150 */	NdrFcShort( 0x0 ),	/* 0 */
/* 152 */	NdrFcShort( 0x8 ),	/* 8 */
/* 154 */	0x7,		/* Oi2 Flags:  srv must size, clt must size, has return, */
			0x3,		/* 3 */

	/* Parameter pArrayVar */

/* 156 */	NdrFcShort( 0x11b ),	/* Flags:  must size, must free, in, out, simple ref, */
/* 158 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 160 */	NdrFcShort( 0x3f6 ),	/* Type Offset=1014 */

	/* Parameter pVar2 */

/* 162 */	NdrFcShort( 0x4113 ),	/* Flags:  must size, must free, out, simple ref, srv alloc size=16 */
/* 164 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 166 */	NdrFcShort( 0x408 ),	/* Type Offset=1032 */

	/* Return value */

/* 168 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 170 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 172 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure Composite */

/* 174 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 176 */	NdrFcLong( 0x0 ),	/* 0 */
/* 180 */	NdrFcShort( 0xd ),	/* 13 */
/* 182 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 184 */	NdrFcShort( 0x0 ),	/* 0 */
/* 186 */	NdrFcShort( 0x8 ),	/* 8 */
/* 188 */	0x7,		/* Oi2 Flags:  srv must size, clt must size, has return, */
			0x3,		/* 3 */

	/* Parameter pArrayVar */

/* 190 */	NdrFcShort( 0x11b ),	/* Flags:  must size, must free, in, out, simple ref, */
/* 192 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 194 */	NdrFcShort( 0x3f6 ),	/* Type Offset=1014 */

	/* Parameter pVar2 */

/* 196 */	NdrFcShort( 0x4113 ),	/* Flags:  must size, must free, out, simple ref, srv alloc size=16 */
/* 198 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 200 */	NdrFcShort( 0x408 ),	/* Type Offset=1032 */

	/* Return value */

/* 202 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 204 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 206 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure Montage */

/* 208 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 210 */	NdrFcLong( 0x0 ),	/* 0 */
/* 214 */	NdrFcShort( 0xe ),	/* 14 */
/* 216 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 218 */	NdrFcShort( 0x0 ),	/* 0 */
/* 220 */	NdrFcShort( 0x8 ),	/* 8 */
/* 222 */	0x7,		/* Oi2 Flags:  srv must size, clt must size, has return, */
			0x3,		/* 3 */

	/* Parameter pArrayVar */

/* 224 */	NdrFcShort( 0x11b ),	/* Flags:  must size, must free, in, out, simple ref, */
/* 226 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 228 */	NdrFcShort( 0x3f6 ),	/* Type Offset=1014 */

	/* Parameter pVar2 */

/* 230 */	NdrFcShort( 0x4113 ),	/* Flags:  must size, must free, out, simple ref, srv alloc size=16 */
/* 232 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 234 */	NdrFcShort( 0x408 ),	/* Type Offset=1032 */

	/* Return value */

/* 236 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 238 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 240 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure Mogrify */

/* 242 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 244 */	NdrFcLong( 0x0 ),	/* 0 */
/* 248 */	NdrFcShort( 0xf ),	/* 15 */
/* 250 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 252 */	NdrFcShort( 0x0 ),	/* 0 */
/* 254 */	NdrFcShort( 0x8 ),	/* 8 */
/* 256 */	0x7,		/* Oi2 Flags:  srv must size, clt must size, has return, */
			0x3,		/* 3 */

	/* Parameter pArrayVar */

/* 258 */	NdrFcShort( 0x11b ),	/* Flags:  must size, must free, in, out, simple ref, */
/* 260 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 262 */	NdrFcShort( 0x3f6 ),	/* Type Offset=1014 */

	/* Parameter pVar2 */

/* 264 */	NdrFcShort( 0x4113 ),	/* Flags:  must size, must free, out, simple ref, srv alloc size=16 */
/* 266 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 268 */	NdrFcShort( 0x408 ),	/* Type Offset=1032 */

	/* Return value */

/* 270 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 272 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 274 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure Identify */

/* 276 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 278 */	NdrFcLong( 0x0 ),	/* 0 */
/* 282 */	NdrFcShort( 0x10 ),	/* 16 */
/* 284 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 286 */	NdrFcShort( 0x0 ),	/* 0 */
/* 288 */	NdrFcShort( 0x8 ),	/* 8 */
/* 290 */	0x7,		/* Oi2 Flags:  srv must size, clt must size, has return, */
			0x3,		/* 3 */

	/* Parameter pArrayVar */

/* 292 */	NdrFcShort( 0x11b ),	/* Flags:  must size, must free, in, out, simple ref, */
/* 294 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 296 */	NdrFcShort( 0x3f6 ),	/* Type Offset=1014 */

	/* Parameter pVar2 */

/* 298 */	NdrFcShort( 0x4113 ),	/* Flags:  must size, must free, out, simple ref, srv alloc size=16 */
/* 300 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 302 */	NdrFcShort( 0x408 ),	/* Type Offset=1032 */

	/* Return value */

/* 304 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 306 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 308 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure get__NewEnum */

/* 310 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 312 */	NdrFcLong( 0x0 ),	/* 0 */
/* 316 */	NdrFcShort( 0x11 ),	/* 17 */
/* 318 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 320 */	NdrFcShort( 0x0 ),	/* 0 */
/* 322 */	NdrFcShort( 0x8 ),	/* 8 */
/* 324 */	0x5,		/* Oi2 Flags:  srv must size, has return, */
			0x2,		/* 2 */

	/* Parameter pVal */

/* 326 */	NdrFcShort( 0x13 ),	/* Flags:  must size, must free, out, */
/* 328 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 330 */	NdrFcShort( 0x420 ),	/* Type Offset=1056 */

	/* Return value */

/* 332 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 334 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 336 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure get_Item */

/* 338 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 340 */	NdrFcLong( 0x0 ),	/* 0 */
/* 344 */	NdrFcShort( 0x12 ),	/* 18 */
/* 346 */	NdrFcShort( 0x1c ),	/* x86 Stack size/offset = 28 */
/* 348 */	NdrFcShort( 0x0 ),	/* 0 */
/* 350 */	NdrFcShort( 0x8 ),	/* 8 */
/* 352 */	0x7,		/* Oi2 Flags:  srv must size, clt must size, has return, */
			0x3,		/* 3 */

	/* Parameter varIndex */

/* 354 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 356 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 358 */	NdrFcShort( 0x416 ),	/* Type Offset=1046 */

	/* Parameter pVal */

/* 360 */	NdrFcShort( 0x4113 ),	/* Flags:  must size, must free, out, simple ref, srv alloc size=16 */
/* 362 */	NdrFcShort( 0x14 ),	/* x86 Stack size/offset = 20 */
/* 364 */	NdrFcShort( 0x408 ),	/* Type Offset=1032 */

	/* Return value */

/* 366 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 368 */	NdrFcShort( 0x18 ),	/* x86 Stack size/offset = 24 */
/* 370 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure get_Messages */

/* 372 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 374 */	NdrFcLong( 0x0 ),	/* 0 */
/* 378 */	NdrFcShort( 0x13 ),	/* 19 */
/* 380 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 382 */	NdrFcShort( 0x0 ),	/* 0 */
/* 384 */	NdrFcShort( 0x8 ),	/* 8 */
/* 386 */	0x5,		/* Oi2 Flags:  srv must size, has return, */
			0x2,		/* 2 */

	/* Parameter pVal */

/* 388 */	NdrFcShort( 0x4113 ),	/* Flags:  must size, must free, out, simple ref, srv alloc size=16 */
/* 390 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 392 */	NdrFcShort( 0x408 ),	/* Type Offset=1032 */

	/* Return value */

/* 394 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 396 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 398 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

			0x0
        }
    };

static const MIDL_TYPE_FORMAT_STRING __MIDL_TypeFormatString =
    {
        0,
        {
			NdrFcShort( 0x0 ),	/* 0 */
/*  2 */	
			0x2f,		/* FC_IP */
			0x5a,		/* FC_CONSTANT_IID */
/*  4 */	NdrFcLong( 0x0 ),	/* 0 */
/*  8 */	NdrFcShort( 0x0 ),	/* 0 */
/* 10 */	NdrFcShort( 0x0 ),	/* 0 */
/* 12 */	0xc0,		/* 192 */
			0x0,		/* 0 */
/* 14 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 16 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 18 */	0x0,		/* 0 */
			0x46,		/* 70 */
/* 20 */	
			0x11, 0xc,	/* FC_RP [alloced_on_stack] [simple_pointer] */
/* 22 */	0x8,		/* FC_LONG */
			0x5c,		/* FC_PAD */
/* 24 */	
			0x11, 0x0,	/* FC_RP */
/* 26 */	NdrFcShort( 0x3dc ),	/* Offset= 988 (1014) */
/* 28 */	
			0x13, 0x10,	/* FC_OP [pointer_deref] */
/* 30 */	NdrFcShort( 0x2 ),	/* Offset= 2 (32) */
/* 32 */	
			0x13, 0x0,	/* FC_OP */
/* 34 */	NdrFcShort( 0x3c2 ),	/* Offset= 962 (996) */
/* 36 */	
			0x2a,		/* FC_ENCAPSULATED_UNION */
			0x49,		/* 73 */
/* 38 */	NdrFcShort( 0x18 ),	/* 24 */
/* 40 */	NdrFcShort( 0xa ),	/* 10 */
/* 42 */	NdrFcLong( 0x8 ),	/* 8 */
/* 46 */	NdrFcShort( 0x6c ),	/* Offset= 108 (154) */
/* 48 */	NdrFcLong( 0xd ),	/* 13 */
/* 52 */	NdrFcShort( 0x8c ),	/* Offset= 140 (192) */
/* 54 */	NdrFcLong( 0x9 ),	/* 9 */
/* 58 */	NdrFcShort( 0xba ),	/* Offset= 186 (244) */
/* 60 */	NdrFcLong( 0xc ),	/* 12 */
/* 64 */	NdrFcShort( 0x2b2 ),	/* Offset= 690 (754) */
/* 66 */	NdrFcLong( 0x24 ),	/* 36 */
/* 70 */	NdrFcShort( 0x2da ),	/* Offset= 730 (800) */
/* 72 */	NdrFcLong( 0x800d ),	/* 32781 */
/* 76 */	NdrFcShort( 0x2f6 ),	/* Offset= 758 (834) */
/* 78 */	NdrFcLong( 0x10 ),	/* 16 */
/* 82 */	NdrFcShort( 0x30e ),	/* Offset= 782 (864) */
/* 84 */	NdrFcLong( 0x2 ),	/* 2 */
/* 88 */	NdrFcShort( 0x326 ),	/* Offset= 806 (894) */
/* 90 */	NdrFcLong( 0x3 ),	/* 3 */
/* 94 */	NdrFcShort( 0x33e ),	/* Offset= 830 (924) */
/* 96 */	NdrFcLong( 0x14 ),	/* 20 */
/* 100 */	NdrFcShort( 0x356 ),	/* Offset= 854 (954) */
/* 102 */	NdrFcShort( 0xffffffff ),	/* Offset= -1 (101) */
/* 104 */	
			0x1b,		/* FC_CARRAY */
			0x1,		/* 1 */
/* 106 */	NdrFcShort( 0x2 ),	/* 2 */
/* 108 */	0x9,		/* Corr desc: FC_ULONG */
			0x0,		/*  */
/* 110 */	NdrFcShort( 0xfffc ),	/* -4 */
/* 112 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 114 */	
			0x17,		/* FC_CSTRUCT */
			0x3,		/* 3 */
/* 116 */	NdrFcShort( 0x8 ),	/* 8 */
/* 118 */	NdrFcShort( 0xfffffff2 ),	/* Offset= -14 (104) */
/* 120 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 122 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 124 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 126 */	NdrFcShort( 0x4 ),	/* 4 */
/* 128 */	0x19,		/* Corr desc:  field pointer, FC_ULONG */
			0x0,		/*  */
/* 130 */	NdrFcShort( 0x0 ),	/* 0 */
/* 132 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 134 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 136 */	NdrFcShort( 0x4 ),	/* 4 */
/* 138 */	NdrFcShort( 0x0 ),	/* 0 */
/* 140 */	NdrFcShort( 0x1 ),	/* 1 */
/* 142 */	NdrFcShort( 0x0 ),	/* 0 */
/* 144 */	NdrFcShort( 0x0 ),	/* 0 */
/* 146 */	0x13, 0x0,	/* FC_OP */
/* 148 */	NdrFcShort( 0xffffffde ),	/* Offset= -34 (114) */
/* 150 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 152 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 154 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 156 */	NdrFcShort( 0x8 ),	/* 8 */
/* 158 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 160 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 162 */	NdrFcShort( 0x4 ),	/* 4 */
/* 164 */	NdrFcShort( 0x4 ),	/* 4 */
/* 166 */	0x11, 0x0,	/* FC_RP */
/* 168 */	NdrFcShort( 0xffffffd4 ),	/* Offset= -44 (124) */
/* 170 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 172 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 174 */	
			0x21,		/* FC_BOGUS_ARRAY */
			0x3,		/* 3 */
/* 176 */	NdrFcShort( 0x0 ),	/* 0 */
/* 178 */	0x19,		/* Corr desc:  field pointer, FC_ULONG */
			0x0,		/*  */
/* 180 */	NdrFcShort( 0x0 ),	/* 0 */
/* 182 */	NdrFcLong( 0xffffffff ),	/* -1 */
/* 186 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 188 */	NdrFcShort( 0xffffff46 ),	/* Offset= -186 (2) */
/* 190 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 192 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 194 */	NdrFcShort( 0x8 ),	/* 8 */
/* 196 */	NdrFcShort( 0x0 ),	/* 0 */
/* 198 */	NdrFcShort( 0x6 ),	/* Offset= 6 (204) */
/* 200 */	0x8,		/* FC_LONG */
			0x36,		/* FC_POINTER */
/* 202 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 204 */	
			0x11, 0x0,	/* FC_RP */
/* 206 */	NdrFcShort( 0xffffffe0 ),	/* Offset= -32 (174) */
/* 208 */	
			0x2f,		/* FC_IP */
			0x5a,		/* FC_CONSTANT_IID */
/* 210 */	NdrFcLong( 0x20400 ),	/* 132096 */
/* 214 */	NdrFcShort( 0x0 ),	/* 0 */
/* 216 */	NdrFcShort( 0x0 ),	/* 0 */
/* 218 */	0xc0,		/* 192 */
			0x0,		/* 0 */
/* 220 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 222 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 224 */	0x0,		/* 0 */
			0x46,		/* 70 */
/* 226 */	
			0x21,		/* FC_BOGUS_ARRAY */
			0x3,		/* 3 */
/* 228 */	NdrFcShort( 0x0 ),	/* 0 */
/* 230 */	0x19,		/* Corr desc:  field pointer, FC_ULONG */
			0x0,		/*  */
/* 232 */	NdrFcShort( 0x0 ),	/* 0 */
/* 234 */	NdrFcLong( 0xffffffff ),	/* -1 */
/* 238 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 240 */	NdrFcShort( 0xffffffe0 ),	/* Offset= -32 (208) */
/* 242 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 244 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 246 */	NdrFcShort( 0x8 ),	/* 8 */
/* 248 */	NdrFcShort( 0x0 ),	/* 0 */
/* 250 */	NdrFcShort( 0x6 ),	/* Offset= 6 (256) */
/* 252 */	0x8,		/* FC_LONG */
			0x36,		/* FC_POINTER */
/* 254 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 256 */	
			0x11, 0x0,	/* FC_RP */
/* 258 */	NdrFcShort( 0xffffffe0 ),	/* Offset= -32 (226) */
/* 260 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x9,		/* FC_ULONG */
/* 262 */	0x7,		/* Corr desc: FC_USHORT */
			0x0,		/*  */
/* 264 */	NdrFcShort( 0xfff8 ),	/* -8 */
/* 266 */	NdrFcShort( 0x2 ),	/* Offset= 2 (268) */
/* 268 */	NdrFcShort( 0x10 ),	/* 16 */
/* 270 */	NdrFcShort( 0x2f ),	/* 47 */
/* 272 */	NdrFcLong( 0x14 ),	/* 20 */
/* 276 */	NdrFcShort( 0x800b ),	/* Simple arm type: FC_HYPER */
/* 278 */	NdrFcLong( 0x3 ),	/* 3 */
/* 282 */	NdrFcShort( 0x8008 ),	/* Simple arm type: FC_LONG */
/* 284 */	NdrFcLong( 0x11 ),	/* 17 */
/* 288 */	NdrFcShort( 0x8001 ),	/* Simple arm type: FC_BYTE */
/* 290 */	NdrFcLong( 0x2 ),	/* 2 */
/* 294 */	NdrFcShort( 0x8006 ),	/* Simple arm type: FC_SHORT */
/* 296 */	NdrFcLong( 0x4 ),	/* 4 */
/* 300 */	NdrFcShort( 0x800a ),	/* Simple arm type: FC_FLOAT */
/* 302 */	NdrFcLong( 0x5 ),	/* 5 */
/* 306 */	NdrFcShort( 0x800c ),	/* Simple arm type: FC_DOUBLE */
/* 308 */	NdrFcLong( 0xb ),	/* 11 */
/* 312 */	NdrFcShort( 0x8006 ),	/* Simple arm type: FC_SHORT */
/* 314 */	NdrFcLong( 0xa ),	/* 10 */
/* 318 */	NdrFcShort( 0x8008 ),	/* Simple arm type: FC_LONG */
/* 320 */	NdrFcLong( 0x6 ),	/* 6 */
/* 324 */	NdrFcShort( 0xe8 ),	/* Offset= 232 (556) */
/* 326 */	NdrFcLong( 0x7 ),	/* 7 */
/* 330 */	NdrFcShort( 0x800c ),	/* Simple arm type: FC_DOUBLE */
/* 332 */	NdrFcLong( 0x8 ),	/* 8 */
/* 336 */	NdrFcShort( 0xe2 ),	/* Offset= 226 (562) */
/* 338 */	NdrFcLong( 0xd ),	/* 13 */
/* 342 */	NdrFcShort( 0xfffffeac ),	/* Offset= -340 (2) */
/* 344 */	NdrFcLong( 0x9 ),	/* 9 */
/* 348 */	NdrFcShort( 0xffffff74 ),	/* Offset= -140 (208) */
/* 350 */	NdrFcLong( 0x2000 ),	/* 8192 */
/* 354 */	NdrFcShort( 0xd4 ),	/* Offset= 212 (566) */
/* 356 */	NdrFcLong( 0x24 ),	/* 36 */
/* 360 */	NdrFcShort( 0xd6 ),	/* Offset= 214 (574) */
/* 362 */	NdrFcLong( 0x4024 ),	/* 16420 */
/* 366 */	NdrFcShort( 0xd0 ),	/* Offset= 208 (574) */
/* 368 */	NdrFcLong( 0x4011 ),	/* 16401 */
/* 372 */	NdrFcShort( 0xfe ),	/* Offset= 254 (626) */
/* 374 */	NdrFcLong( 0x4002 ),	/* 16386 */
/* 378 */	NdrFcShort( 0xfc ),	/* Offset= 252 (630) */
/* 380 */	NdrFcLong( 0x4003 ),	/* 16387 */
/* 384 */	NdrFcShort( 0xfa ),	/* Offset= 250 (634) */
/* 386 */	NdrFcLong( 0x4014 ),	/* 16404 */
/* 390 */	NdrFcShort( 0xf8 ),	/* Offset= 248 (638) */
/* 392 */	NdrFcLong( 0x4004 ),	/* 16388 */
/* 396 */	NdrFcShort( 0xf6 ),	/* Offset= 246 (642) */
/* 398 */	NdrFcLong( 0x4005 ),	/* 16389 */
/* 402 */	NdrFcShort( 0xf4 ),	/* Offset= 244 (646) */
/* 404 */	NdrFcLong( 0x400b ),	/* 16395 */
/* 408 */	NdrFcShort( 0xde ),	/* Offset= 222 (630) */
/* 410 */	NdrFcLong( 0x400a ),	/* 16394 */
/* 414 */	NdrFcShort( 0xdc ),	/* Offset= 220 (634) */
/* 416 */	NdrFcLong( 0x4006 ),	/* 16390 */
/* 420 */	NdrFcShort( 0xe6 ),	/* Offset= 230 (650) */
/* 422 */	NdrFcLong( 0x4007 ),	/* 16391 */
/* 426 */	NdrFcShort( 0xdc ),	/* Offset= 220 (646) */
/* 428 */	NdrFcLong( 0x4008 ),	/* 16392 */
/* 432 */	NdrFcShort( 0xde ),	/* Offset= 222 (654) */
/* 434 */	NdrFcLong( 0x400d ),	/* 16397 */
/* 438 */	NdrFcShort( 0xdc ),	/* Offset= 220 (658) */
/* 440 */	NdrFcLong( 0x4009 ),	/* 16393 */
/* 444 */	NdrFcShort( 0xda ),	/* Offset= 218 (662) */
/* 446 */	NdrFcLong( 0x6000 ),	/* 24576 */
/* 450 */	NdrFcShort( 0xd8 ),	/* Offset= 216 (666) */
/* 452 */	NdrFcLong( 0x400c ),	/* 16396 */
/* 456 */	NdrFcShort( 0xde ),	/* Offset= 222 (678) */
/* 458 */	NdrFcLong( 0x10 ),	/* 16 */
/* 462 */	NdrFcShort( 0x8002 ),	/* Simple arm type: FC_CHAR */
/* 464 */	NdrFcLong( 0x12 ),	/* 18 */
/* 468 */	NdrFcShort( 0x8006 ),	/* Simple arm type: FC_SHORT */
/* 470 */	NdrFcLong( 0x13 ),	/* 19 */
/* 474 */	NdrFcShort( 0x8008 ),	/* Simple arm type: FC_LONG */
/* 476 */	NdrFcLong( 0x15 ),	/* 21 */
/* 480 */	NdrFcShort( 0x800b ),	/* Simple arm type: FC_HYPER */
/* 482 */	NdrFcLong( 0x16 ),	/* 22 */
/* 486 */	NdrFcShort( 0x8008 ),	/* Simple arm type: FC_LONG */
/* 488 */	NdrFcLong( 0x17 ),	/* 23 */
/* 492 */	NdrFcShort( 0x8008 ),	/* Simple arm type: FC_LONG */
/* 494 */	NdrFcLong( 0xe ),	/* 14 */
/* 498 */	NdrFcShort( 0xbc ),	/* Offset= 188 (686) */
/* 500 */	NdrFcLong( 0x400e ),	/* 16398 */
/* 504 */	NdrFcShort( 0xc0 ),	/* Offset= 192 (696) */
/* 506 */	NdrFcLong( 0x4010 ),	/* 16400 */
/* 510 */	NdrFcShort( 0xbe ),	/* Offset= 190 (700) */
/* 512 */	NdrFcLong( 0x4012 ),	/* 16402 */
/* 516 */	NdrFcShort( 0x72 ),	/* Offset= 114 (630) */
/* 518 */	NdrFcLong( 0x4013 ),	/* 16403 */
/* 522 */	NdrFcShort( 0x70 ),	/* Offset= 112 (634) */
/* 524 */	NdrFcLong( 0x4015 ),	/* 16405 */
/* 528 */	NdrFcShort( 0x6e ),	/* Offset= 110 (638) */
/* 530 */	NdrFcLong( 0x4016 ),	/* 16406 */
/* 534 */	NdrFcShort( 0x64 ),	/* Offset= 100 (634) */
/* 536 */	NdrFcLong( 0x4017 ),	/* 16407 */
/* 540 */	NdrFcShort( 0x5e ),	/* Offset= 94 (634) */
/* 542 */	NdrFcLong( 0x0 ),	/* 0 */
/* 546 */	NdrFcShort( 0x0 ),	/* Offset= 0 (546) */
/* 548 */	NdrFcLong( 0x1 ),	/* 1 */
/* 552 */	NdrFcShort( 0x0 ),	/* Offset= 0 (552) */
/* 554 */	NdrFcShort( 0xffffffff ),	/* Offset= -1 (553) */
/* 556 */	
			0x15,		/* FC_STRUCT */
			0x7,		/* 7 */
/* 558 */	NdrFcShort( 0x8 ),	/* 8 */
/* 560 */	0xb,		/* FC_HYPER */
			0x5b,		/* FC_END */
/* 562 */	
			0x13, 0x0,	/* FC_OP */
/* 564 */	NdrFcShort( 0xfffffe3e ),	/* Offset= -450 (114) */
/* 566 */	
			0x13, 0x10,	/* FC_OP [pointer_deref] */
/* 568 */	NdrFcShort( 0x2 ),	/* Offset= 2 (570) */
/* 570 */	
			0x13, 0x0,	/* FC_OP */
/* 572 */	NdrFcShort( 0x1a8 ),	/* Offset= 424 (996) */
/* 574 */	
			0x13, 0x0,	/* FC_OP */
/* 576 */	NdrFcShort( 0x1e ),	/* Offset= 30 (606) */
/* 578 */	
			0x2f,		/* FC_IP */
			0x5a,		/* FC_CONSTANT_IID */
/* 580 */	NdrFcLong( 0x2f ),	/* 47 */
/* 584 */	NdrFcShort( 0x0 ),	/* 0 */
/* 586 */	NdrFcShort( 0x0 ),	/* 0 */
/* 588 */	0xc0,		/* 192 */
			0x0,		/* 0 */
/* 590 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 592 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 594 */	0x0,		/* 0 */
			0x46,		/* 70 */
/* 596 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 598 */	NdrFcShort( 0x1 ),	/* 1 */
/* 600 */	0x19,		/* Corr desc:  field pointer, FC_ULONG */
			0x0,		/*  */
/* 602 */	NdrFcShort( 0x4 ),	/* 4 */
/* 604 */	0x1,		/* FC_BYTE */
			0x5b,		/* FC_END */
/* 606 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 608 */	NdrFcShort( 0x10 ),	/* 16 */
/* 610 */	NdrFcShort( 0x0 ),	/* 0 */
/* 612 */	NdrFcShort( 0xa ),	/* Offset= 10 (622) */
/* 614 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 616 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 618 */	NdrFcShort( 0xffffffd8 ),	/* Offset= -40 (578) */
/* 620 */	0x36,		/* FC_POINTER */
			0x5b,		/* FC_END */
/* 622 */	
			0x13, 0x0,	/* FC_OP */
/* 624 */	NdrFcShort( 0xffffffe4 ),	/* Offset= -28 (596) */
/* 626 */	
			0x13, 0x8,	/* FC_OP [simple_pointer] */
/* 628 */	0x1,		/* FC_BYTE */
			0x5c,		/* FC_PAD */
/* 630 */	
			0x13, 0x8,	/* FC_OP [simple_pointer] */
/* 632 */	0x6,		/* FC_SHORT */
			0x5c,		/* FC_PAD */
/* 634 */	
			0x13, 0x8,	/* FC_OP [simple_pointer] */
/* 636 */	0x8,		/* FC_LONG */
			0x5c,		/* FC_PAD */
/* 638 */	
			0x13, 0x8,	/* FC_OP [simple_pointer] */
/* 640 */	0xb,		/* FC_HYPER */
			0x5c,		/* FC_PAD */
/* 642 */	
			0x13, 0x8,	/* FC_OP [simple_pointer] */
/* 644 */	0xa,		/* FC_FLOAT */
			0x5c,		/* FC_PAD */
/* 646 */	
			0x13, 0x8,	/* FC_OP [simple_pointer] */
/* 648 */	0xc,		/* FC_DOUBLE */
			0x5c,		/* FC_PAD */
/* 650 */	
			0x13, 0x0,	/* FC_OP */
/* 652 */	NdrFcShort( 0xffffffa0 ),	/* Offset= -96 (556) */
/* 654 */	
			0x13, 0x10,	/* FC_OP [pointer_deref] */
/* 656 */	NdrFcShort( 0xffffffa2 ),	/* Offset= -94 (562) */
/* 658 */	
			0x13, 0x10,	/* FC_OP [pointer_deref] */
/* 660 */	NdrFcShort( 0xfffffd6e ),	/* Offset= -658 (2) */
/* 662 */	
			0x13, 0x10,	/* FC_OP [pointer_deref] */
/* 664 */	NdrFcShort( 0xfffffe38 ),	/* Offset= -456 (208) */
/* 666 */	
			0x13, 0x10,	/* FC_OP [pointer_deref] */
/* 668 */	NdrFcShort( 0x2 ),	/* Offset= 2 (670) */
/* 670 */	
			0x13, 0x10,	/* FC_OP [pointer_deref] */
/* 672 */	NdrFcShort( 0x2 ),	/* Offset= 2 (674) */
/* 674 */	
			0x13, 0x0,	/* FC_OP */
/* 676 */	NdrFcShort( 0x140 ),	/* Offset= 320 (996) */
/* 678 */	
			0x13, 0x10,	/* FC_OP [pointer_deref] */
/* 680 */	NdrFcShort( 0x2 ),	/* Offset= 2 (682) */
/* 682 */	
			0x13, 0x0,	/* FC_OP */
/* 684 */	NdrFcShort( 0x14 ),	/* Offset= 20 (704) */
/* 686 */	
			0x15,		/* FC_STRUCT */
			0x7,		/* 7 */
/* 688 */	NdrFcShort( 0x10 ),	/* 16 */
/* 690 */	0x6,		/* FC_SHORT */
			0x1,		/* FC_BYTE */
/* 692 */	0x1,		/* FC_BYTE */
			0x8,		/* FC_LONG */
/* 694 */	0xb,		/* FC_HYPER */
			0x5b,		/* FC_END */
/* 696 */	
			0x13, 0x0,	/* FC_OP */
/* 698 */	NdrFcShort( 0xfffffff4 ),	/* Offset= -12 (686) */
/* 700 */	
			0x13, 0x8,	/* FC_OP [simple_pointer] */
/* 702 */	0x2,		/* FC_CHAR */
			0x5c,		/* FC_PAD */
/* 704 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x7,		/* 7 */
/* 706 */	NdrFcShort( 0x20 ),	/* 32 */
/* 708 */	NdrFcShort( 0x0 ),	/* 0 */
/* 710 */	NdrFcShort( 0x0 ),	/* Offset= 0 (710) */
/* 712 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 714 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 716 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 718 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 720 */	NdrFcShort( 0xfffffe34 ),	/* Offset= -460 (260) */
/* 722 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 724 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 726 */	NdrFcShort( 0x4 ),	/* 4 */
/* 728 */	0x19,		/* Corr desc:  field pointer, FC_ULONG */
			0x0,		/*  */
/* 730 */	NdrFcShort( 0x0 ),	/* 0 */
/* 732 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 734 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 736 */	NdrFcShort( 0x4 ),	/* 4 */
/* 738 */	NdrFcShort( 0x0 ),	/* 0 */
/* 740 */	NdrFcShort( 0x1 ),	/* 1 */
/* 742 */	NdrFcShort( 0x0 ),	/* 0 */
/* 744 */	NdrFcShort( 0x0 ),	/* 0 */
/* 746 */	0x13, 0x0,	/* FC_OP */
/* 748 */	NdrFcShort( 0xffffffd4 ),	/* Offset= -44 (704) */
/* 750 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 752 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 754 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 756 */	NdrFcShort( 0x8 ),	/* 8 */
/* 758 */	NdrFcShort( 0x0 ),	/* 0 */
/* 760 */	NdrFcShort( 0x6 ),	/* Offset= 6 (766) */
/* 762 */	0x8,		/* FC_LONG */
			0x36,		/* FC_POINTER */
/* 764 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 766 */	
			0x11, 0x0,	/* FC_RP */
/* 768 */	NdrFcShort( 0xffffffd4 ),	/* Offset= -44 (724) */
/* 770 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 772 */	NdrFcShort( 0x4 ),	/* 4 */
/* 774 */	0x19,		/* Corr desc:  field pointer, FC_ULONG */
			0x0,		/*  */
/* 776 */	NdrFcShort( 0x0 ),	/* 0 */
/* 778 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 780 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 782 */	NdrFcShort( 0x4 ),	/* 4 */
/* 784 */	NdrFcShort( 0x0 ),	/* 0 */
/* 786 */	NdrFcShort( 0x1 ),	/* 1 */
/* 788 */	NdrFcShort( 0x0 ),	/* 0 */
/* 790 */	NdrFcShort( 0x0 ),	/* 0 */
/* 792 */	0x13, 0x0,	/* FC_OP */
/* 794 */	NdrFcShort( 0xffffff44 ),	/* Offset= -188 (606) */
/* 796 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 798 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 800 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 802 */	NdrFcShort( 0x8 ),	/* 8 */
/* 804 */	NdrFcShort( 0x0 ),	/* 0 */
/* 806 */	NdrFcShort( 0x6 ),	/* Offset= 6 (812) */
/* 808 */	0x8,		/* FC_LONG */
			0x36,		/* FC_POINTER */
/* 810 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 812 */	
			0x11, 0x0,	/* FC_RP */
/* 814 */	NdrFcShort( 0xffffffd4 ),	/* Offset= -44 (770) */
/* 816 */	
			0x1d,		/* FC_SMFARRAY */
			0x0,		/* 0 */
/* 818 */	NdrFcShort( 0x8 ),	/* 8 */
/* 820 */	0x1,		/* FC_BYTE */
			0x5b,		/* FC_END */
/* 822 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 824 */	NdrFcShort( 0x10 ),	/* 16 */
/* 826 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 828 */	0x6,		/* FC_SHORT */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 830 */	0x0,		/* 0 */
			NdrFcShort( 0xfffffff1 ),	/* Offset= -15 (816) */
			0x5b,		/* FC_END */
/* 834 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 836 */	NdrFcShort( 0x18 ),	/* 24 */
/* 838 */	NdrFcShort( 0x0 ),	/* 0 */
/* 840 */	NdrFcShort( 0xa ),	/* Offset= 10 (850) */
/* 842 */	0x8,		/* FC_LONG */
			0x36,		/* FC_POINTER */
/* 844 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 846 */	NdrFcShort( 0xffffffe8 ),	/* Offset= -24 (822) */
/* 848 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 850 */	
			0x11, 0x0,	/* FC_RP */
/* 852 */	NdrFcShort( 0xfffffd5a ),	/* Offset= -678 (174) */
/* 854 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 856 */	NdrFcShort( 0x1 ),	/* 1 */
/* 858 */	0x19,		/* Corr desc:  field pointer, FC_ULONG */
			0x0,		/*  */
/* 860 */	NdrFcShort( 0x0 ),	/* 0 */
/* 862 */	0x1,		/* FC_BYTE */
			0x5b,		/* FC_END */
/* 864 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 866 */	NdrFcShort( 0x8 ),	/* 8 */
/* 868 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 870 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 872 */	NdrFcShort( 0x4 ),	/* 4 */
/* 874 */	NdrFcShort( 0x4 ),	/* 4 */
/* 876 */	0x13, 0x0,	/* FC_OP */
/* 878 */	NdrFcShort( 0xffffffe8 ),	/* Offset= -24 (854) */
/* 880 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 882 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 884 */	
			0x1b,		/* FC_CARRAY */
			0x1,		/* 1 */
/* 886 */	NdrFcShort( 0x2 ),	/* 2 */
/* 888 */	0x19,		/* Corr desc:  field pointer, FC_ULONG */
			0x0,		/*  */
/* 890 */	NdrFcShort( 0x0 ),	/* 0 */
/* 892 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 894 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 896 */	NdrFcShort( 0x8 ),	/* 8 */
/* 898 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 900 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 902 */	NdrFcShort( 0x4 ),	/* 4 */
/* 904 */	NdrFcShort( 0x4 ),	/* 4 */
/* 906 */	0x13, 0x0,	/* FC_OP */
/* 908 */	NdrFcShort( 0xffffffe8 ),	/* Offset= -24 (884) */
/* 910 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 912 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 914 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 916 */	NdrFcShort( 0x4 ),	/* 4 */
/* 918 */	0x19,		/* Corr desc:  field pointer, FC_ULONG */
			0x0,		/*  */
/* 920 */	NdrFcShort( 0x0 ),	/* 0 */
/* 922 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 924 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 926 */	NdrFcShort( 0x8 ),	/* 8 */
/* 928 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 930 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 932 */	NdrFcShort( 0x4 ),	/* 4 */
/* 934 */	NdrFcShort( 0x4 ),	/* 4 */
/* 936 */	0x13, 0x0,	/* FC_OP */
/* 938 */	NdrFcShort( 0xffffffe8 ),	/* Offset= -24 (914) */
/* 940 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 942 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 944 */	
			0x1b,		/* FC_CARRAY */
			0x7,		/* 7 */
/* 946 */	NdrFcShort( 0x8 ),	/* 8 */
/* 948 */	0x19,		/* Corr desc:  field pointer, FC_ULONG */
			0x0,		/*  */
/* 950 */	NdrFcShort( 0x0 ),	/* 0 */
/* 952 */	0xb,		/* FC_HYPER */
			0x5b,		/* FC_END */
/* 954 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 956 */	NdrFcShort( 0x8 ),	/* 8 */
/* 958 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 960 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 962 */	NdrFcShort( 0x4 ),	/* 4 */
/* 964 */	NdrFcShort( 0x4 ),	/* 4 */
/* 966 */	0x13, 0x0,	/* FC_OP */
/* 968 */	NdrFcShort( 0xffffffe8 ),	/* Offset= -24 (944) */
/* 970 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 972 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 974 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 976 */	NdrFcShort( 0x8 ),	/* 8 */
/* 978 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 980 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 982 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 984 */	NdrFcShort( 0x8 ),	/* 8 */
/* 986 */	0x7,		/* Corr desc: FC_USHORT */
			0x0,		/*  */
/* 988 */	NdrFcShort( 0xffd8 ),	/* -40 */
/* 990 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 992 */	NdrFcShort( 0xffffffee ),	/* Offset= -18 (974) */
/* 994 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 996 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 998 */	NdrFcShort( 0x28 ),	/* 40 */
/* 1000 */	NdrFcShort( 0xffffffee ),	/* Offset= -18 (982) */
/* 1002 */	NdrFcShort( 0x0 ),	/* Offset= 0 (1002) */
/* 1004 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 1006 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 1008 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 1010 */	NdrFcShort( 0xfffffc32 ),	/* Offset= -974 (36) */
/* 1012 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 1014 */	0xb4,		/* FC_USER_MARSHAL */
			0x83,		/* 131 */
/* 1016 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1018 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1020 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1022 */	NdrFcShort( 0xfffffc1e ),	/* Offset= -994 (28) */
/* 1024 */	
			0x11, 0x4,	/* FC_RP [alloced_on_stack] */
/* 1026 */	NdrFcShort( 0x6 ),	/* Offset= 6 (1032) */
/* 1028 */	
			0x13, 0x0,	/* FC_OP */
/* 1030 */	NdrFcShort( 0xfffffeba ),	/* Offset= -326 (704) */
/* 1032 */	0xb4,		/* FC_USER_MARSHAL */
			0x83,		/* 131 */
/* 1034 */	NdrFcShort( 0x1 ),	/* 1 */
/* 1036 */	NdrFcShort( 0x10 ),	/* 16 */
/* 1038 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1040 */	NdrFcShort( 0xfffffff4 ),	/* Offset= -12 (1028) */
/* 1042 */	
			0x12, 0x0,	/* FC_UP */
/* 1044 */	NdrFcShort( 0xfffffeac ),	/* Offset= -340 (704) */
/* 1046 */	0xb4,		/* FC_USER_MARSHAL */
			0x83,		/* 131 */
/* 1048 */	NdrFcShort( 0x1 ),	/* 1 */
/* 1050 */	NdrFcShort( 0x10 ),	/* 16 */
/* 1052 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1054 */	NdrFcShort( 0xfffffff4 ),	/* Offset= -12 (1042) */
/* 1056 */	
			0x11, 0x10,	/* FC_RP [pointer_deref] */
/* 1058 */	NdrFcShort( 0xfffffbe0 ),	/* Offset= -1056 (2) */

			0x0
        }
    };

static const USER_MARSHAL_ROUTINE_QUADRUPLE UserMarshalRoutines[ WIRE_MARSHAL_TABLE_SIZE ] = 
        {
            
            {
            LPSAFEARRAY_UserSize
            ,LPSAFEARRAY_UserMarshal
            ,LPSAFEARRAY_UserUnmarshal
            ,LPSAFEARRAY_UserFree
            },
            {
            VARIANT_UserSize
            ,VARIANT_UserMarshal
            ,VARIANT_UserUnmarshal
            ,VARIANT_UserFree
            }

        };



/* Object interface: IUnknown, ver. 0.0,
   GUID={0x00000000,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}} */


/* Object interface: IDispatch, ver. 0.0,
   GUID={0x00020400,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}} */


/* Object interface: IMagickImage, ver. 0.0,
   GUID={0x7F670536,0x00AE,0x4EDF,{0xB0,0x6F,0x13,0xBD,0x22,0xB2,0x56,0x24}} */

#pragma code_seg(".orpc")
static const unsigned short IMagickImage_FormatStringOffsetTable[] =
    {
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    0,
    28,
    50,
    78,
    112,
    140,
    174,
    208,
    242,
    276,
    310,
    338,
    372
    };

static const MIDL_STUBLESS_PROXY_INFO IMagickImage_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &IMagickImage_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO IMagickImage_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &IMagickImage_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(20) _IMagickImageProxyVtbl = 
{
    &IMagickImage_ProxyInfo,
    &IID_IMagickImage,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    0 /* (void *) (INT_PTR) -1 /* IDispatch::GetTypeInfoCount */ ,
    0 /* (void *) (INT_PTR) -1 /* IDispatch::GetTypeInfo */ ,
    0 /* (void *) (INT_PTR) -1 /* IDispatch::GetIDsOfNames */ ,
    0 /* IDispatch_Invoke_Proxy */ ,
    (void *) (INT_PTR) -1 /* IMagickImage::OnStartPage */ ,
    (void *) (INT_PTR) -1 /* IMagickImage::OnEndPage */ ,
    (void *) (INT_PTR) -1 /* IMagickImage::get_Count */ ,
    (void *) (INT_PTR) -1 /* IMagickImage::Add */ ,
    (void *) (INT_PTR) -1 /* IMagickImage::Remove */ ,
    (void *) (INT_PTR) -1 /* IMagickImage::Convert */ ,
    (void *) (INT_PTR) -1 /* IMagickImage::Composite */ ,
    (void *) (INT_PTR) -1 /* IMagickImage::Montage */ ,
    (void *) (INT_PTR) -1 /* IMagickImage::Mogrify */ ,
    (void *) (INT_PTR) -1 /* IMagickImage::Identify */ ,
    (void *) (INT_PTR) -1 /* IMagickImage::get__NewEnum */ ,
    (void *) (INT_PTR) -1 /* IMagickImage::get_Item */ ,
    (void *) (INT_PTR) -1 /* IMagickImage::get_Messages */
};


static const PRPC_STUB_FUNCTION IMagickImage_table[] =
{
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2
};

CInterfaceStubVtbl _IMagickImageStubVtbl =
{
    &IID_IMagickImage,
    &IMagickImage_ServerInfo,
    20,
    &IMagickImage_table[-3],
    CStdStubBuffer_DELEGATING_METHODS
};

static const MIDL_STUB_DESC Object_StubDesc = 
    {
    0,
    NdrOleAllocate,
    NdrOleFree,
    0,
    0,
    0,
    0,
    0,
    __MIDL_TypeFormatString.Format,
    1, /* -error bounds_check flag */
    0x20000, /* Ndr library version */
    0,
    0x600015b, /* MIDL Version 6.0.347 */
    0,
    UserMarshalRoutines,
    0,  /* notify & notify_flag routine table */
    0x1, /* MIDL flag */
    0, /* cs routines */
    0,   /* proxy/server info */
    0   /* Reserved5 */
    };

const CInterfaceProxyVtbl * _ImageMagickObject_ProxyVtblList[] = 
{
    ( CInterfaceProxyVtbl *) &_IMagickImageProxyVtbl,
    0
};

const CInterfaceStubVtbl * _ImageMagickObject_StubVtblList[] = 
{
    ( CInterfaceStubVtbl *) &_IMagickImageStubVtbl,
    0
};

PCInterfaceName const _ImageMagickObject_InterfaceNamesList[] = 
{
    "IMagickImage",
    0
};

const IID *  _ImageMagickObject_BaseIIDList[] = 
{
    &IID_IDispatch,
    0
};


#define _ImageMagickObject_CHECK_IID(n)	IID_GENERIC_CHECK_IID( _ImageMagickObject, pIID, n)

int __stdcall _ImageMagickObject_IID_Lookup( const IID * pIID, int * pIndex )
{
    
    if(!_ImageMagickObject_CHECK_IID(0))
        {
        *pIndex = 0;
        return 1;
        }

    return 0;
}

const ExtendedProxyFileInfo ImageMagickObject_ProxyFileInfo = 
{
    (PCInterfaceProxyVtblList *) & _ImageMagickObject_ProxyVtblList,
    (PCInterfaceStubVtblList *) & _ImageMagickObject_StubVtblList,
    (const PCInterfaceName * ) & _ImageMagickObject_InterfaceNamesList,
    (const IID ** ) & _ImageMagickObject_BaseIIDList,
    & _ImageMagickObject_IID_Lookup, 
    1,
    2,
    0, /* table of [async_uuid] interfaces */
    0, /* Filler1 */
    0, /* Filler2 */
    0  /* Filler3 */
};


#endif /* !defined(_M_IA64) && !defined(_M_AMD64)*/

