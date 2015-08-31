/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                         I S A P I M A G I C K                               %
%                                                                             %
%                                                                             %
%      A shim layer to give command line utilities an ISAPI interface.        %
%                                                                             %
%                                                                             %
%                                                                             %
%                              Software Design                                %
%                           William T. Radcliffe                              %
%                                June 2000                                    %
%                                                                             %
%                                                                             %
%  Copyright (C) 2000 ImageMagick Studio, a non-profit organization dedicated %
%  to making software imaging solutions freely available.                     %
%                                                                             %
%  Permission is hereby granted, free of charge, to any person obtaining a    %
%  copy of this software and associated documentation files ("ImageMagick"),  %
%  to deal in ImageMagick without restriction, including without limitation   %
%  the rights to use, copy, modify, merge, publish, distribute, sublicense,   %
%  and/or sell copies of ImageMagick, and to permit persons to whom the       %
%  ImageMagick is furnished to do so, subject to the following conditions:    %
%                                                                             %
%  The above copyright notice and this permission notice shall be included in %
%  all copies or substantial portions of ImageMagick.                         %
%                                                                             %
%  The software is provided "as is", without warranty of any kind, express or %
%  implied, including but not limited to the warranties of merchantability,   %
%  fitness for a particular purpose and noninfringement.  In no event shall   %
%  ImageMagick Studio be liable for any claim, damages or other liability,    %
%  whether in an action of contract, tort or otherwise, arising from, out of  %
%  or in connection with ImageMagick or the use or other dealings in          %
%  ImageMagick.                                                               %
%                                                                             %
%  Except as contained in this notice, the name of the ImageMagick Studio     %
%  shall not be used in advertising or otherwise to promote the sale, use or  %
%  other dealings in ImageMagick without prior written authorization from the %
%  ImageMagick Studio.                                                        %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  This is a simple shim that provides an ISAPI layer for calling into any of
%  the normal command line utilities.
*/

#include "magick.h"
#include "define.h"

#include <httpext.h>

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

// A critical section handle is used to protect global
// state properties
CRITICAL_SECTION gCS;
// Global path to this DLL
TCHAR gszAppPath[MAX_PATH];

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  D l l M a i n                                                              %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DllMain - ISAPI / Win32 API method. This the the entry and exit
%  point for the extension. It is called when the extension is loaded
%  and unloaded by IIS. This is where state properties need to be 
%  retrieved from and store on persistant storage.
%
%
*/
BOOL APIENTRY DllMain( HANDLE hModule,
      DWORD ul_reason_for_call, LPVOID lpReserved )
{
  switch( ul_reason_for_call ) {
  case DLL_PROCESS_ATTACH: 
    {
      InitializeCriticalSection(&gCS);
      gszAppPath[0]='\0';
      if (!GetModuleFileName (hModule, gszAppPath, MAX_PATH))
        return FALSE;
      InitializeMagick(gszAppPath);
      break;
    }
  case DLL_PROCESS_DETACH:
		{
		  DeleteCriticalSection(&gCS);
		  break;
		}
  case DLL_THREAD_ATTACH:
  case DLL_THREAD_DETACH:
  default:
    break;
  }
  return TRUE;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  G e t E x t e n s i o n V e r s i o n                                      %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetExtensionVersion - ISAPI / Win32 API method. This method
%  is required by IIS. It is called following the process load
%  to ensure that the  extension is compatable with the server.
%
*/
BOOL WINAPI GetExtensionVersion( HSE_VERSION_INFO *pVer )
{
  pVer->dwExtensionVersion = MAKELONG(	HSE_VERSION_MINOR,
										HSE_VERSION_MAJOR );
  lstrcpyn( pVer->lpszExtensionDesc,
            "ISAPIMagick for IIS", HSE_MAX_EXT_DLL_NAME_LEN );
  return TRUE;
} 

/*
  Include the convert mainline as a subroutine
*/
#define Usage convert_Usage
#define main convert_main
#include "utilities\convert.c"
#undef Usage
#undef main
/*
  Include the combine mainline as a subroutine
*/
#define Usage combine_Usage
#define main combine_main
#include "utilities\composite.c"
#undef Usage
#undef main

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  C G I F i f o                                                              %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method CGIFifo is the default output stream handler for CGI usage. It is
%  called by the stream subsystem whenever a buffer of data needs to be sent
%  to the output.It receives a pointer to the image as well as the buffer
%  of data and its length.
%
%  The format of the HttpUnescape method is:
%
%      int CGIFifo(const Image *image,const void *data,const size_t length)
%
%  A description of each parameter follows:
%
%    o image:  A pointer to the image being sent to the output stream.
%
%    o data: A pointer to the buffer of data to be sent.
%
%    o length: The length of the buffer of data to be sent.
%
*/
int CGIFifo(const Image *image,const void *data,const size_t length)
{
  EXTENSION_CONTROL_BLOCK *pECB;
  size_t tlen=length;
  pECB = (EXTENSION_CONTROL_BLOCK *)image->client_data;
  pECB->WriteClient( pECB->ConnID, (char *)data, &tlen, 0);
  return(tlen);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  D e c o d e H e x                                                          %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method DecodeHex decodes a hexadecimal string. It stops after outmax
%  characters or when an invalid hex character is reached. It also sets
%  the input pointer to the first unprocessed character.  Returns the
%  the result as a binary number.
%
%  The format of the HttpUnescape method is:
%
%      int DecodeHex(const char **input,size_t outmax)
%
%  A description of each parameter follows:
%
%    o input:  Specifies the string to be converted.
%
%    o outmax: Specifies the maximum number of characters to process.
%
*/
int DecodeHex(const char **input,size_t outmax)
{
  static char
    hex_to_bin [128] = {
      -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,    /*            */
      -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,    /*            */
      -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,    /*            */
       0, 1, 2, 3, 4, 5, 6, 7, 8, 9,-1,-1,-1,-1,-1,-1,    /*   0..9     */
      -1,10,11,12,13,14,15,-1,-1,-1,-1,-1,-1,-1,-1,-1,    /*   A..F     */
      -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,    /*            */
      -1,10,11,12,13,14,15,-1,-1,-1,-1,-1,-1,-1,-1,-1,    /*   a..f     */
      -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 };  /*            */

  int
    nextch;

  size_t
    index,
    result;

  assert(input != (const char **) NULL);
  assert((*input) != (const char *) NULL);

  index  = 0;
  result = 0;
  while (outmax == 0 || index < outmax)
  {
    nextch = (*input) [index] & 127;
    if (nextch && hex_to_bin [nextch] != -1)
    {
      result = result * 16 + hex_to_bin [nextch];
      index++;
    }
    else
      break;
  }
  (*input) += index;
  return (result);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  H t t p U n e s c a p e                                                    %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method HttpUnescape removes HTTP escaping from a string. If the result
%  string is NULL, it modifies the source string in place, else fills-in
%  the result string. This method returns the resulting string. End-of-line
%  sequences (%0A%0D) are stored as a single new-line character, i.e.
%  carriage-returns (%0D) are not stored.
%
%  The format of the HttpUnescape method is:
%
%      char *HttpUnescape(char *string,char *result)
%
%  A description of each parameter follows:
%
%    o string:  Specifies the string to be converted.
%
%    o result:  Specifies the destination for the converted result. A NULL
%               indicates that the conversion happens "in-place".
%
*/
char *HttpUnescape(char *string,char *result)
{
  char
    *target; /* Where we store the result */

  assert(string != (char *) NULL);
  if (!result) /* If result string is null, */
    result = string; /* modify in place */
  target = result;

  while (*string)
  {
    if (*string == '%'  /* Unescape %xx sequence */
          && isxdigit(string [1]) && isxdigit(string [2]))
      {
        string++;
        *target = DecodeHex ((const char **) &string, 2);
        if (*target != '\r')
          target++; /* We do not store CR's */
      }
    else
      {
        if (*string == '+') /* Spaces are escaped as '+' */
          *target++ = ' ';
        else
          *target++ = *string; /* Otherwise just copy */

        string++;
      }
  }
  *target = '\0'; /* Terminate target string */
  return (result);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  C G I T o A r g v                                                          %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method CGIToArgv converts a text string passed as part of a CGI request
%  into command line arguments.
%
%  The format of the CGIToArgv method is:
%
%      char **CGIToArgv(const char *text,int *argc,char ***argv)
%
%  A description of each parameter follows:
%
%    o text:  Specifies the string to segment into a list.
%
%    o argc:  This integer pointer returns the number of arguments in the
%             list.
%
%    o argv:  This array of pointers returns the string list unless an error
%             occurs, otherwise NULL.
%
*/
#define IsCGIDelimiter(c)  (((c) == '&') || ((c) == '=') || ((c) == '\0'))

unsigned int CGIToArgv(const char *text,int *argc,char ***argv)
{
  char
    **vector;

  const char
    *p,
    *q;

  register int
    i;

  int
    count;

  if (text == (char *) NULL)
    return(False);
  /*
    Determine the number of arguments by scanning for delimiters
  */
  q=text;
  count=0;
  while (1)
  {
    int
      len;

    p=q;
    while (!IsCGIDelimiter(*q))
      q++;
    len=q-p;
    if (len > 0)
      count++;
    if (*q == '\0')
      break;
    q++;
  }
  vector=(char **) AcquireMemory((count+2)*sizeof(char *));
  if (vector == (char **) NULL)
    {
      MagickError(ResourceLimitError,"Unable to convert string to argv",
        "Memory allocation failed");
      return(False);
    }
  /*
    Convert string to an ASCII list.
  */
  vector[0]=AllocateString("isapimagick");
  vector[count+1]=(char *) NULL;
  q=text;
  i=1;
  while (i <= count)
  {
    int
      len;

    p=q;
    while (!IsCGIDelimiter(*q))
      q++;
    /*
       Skip an zero length tokens. This typically happens for the case
       of xxx=& on a CGI GET or POST were the name value pair has no
       value
    */
    len=q-p;
    if (len > 0)
      {
        vector[i]=(char *) AcquireMemory(q-p+1);
        if (vector[i] == (char *) NULL)
          {
            MagickError(ResourceLimitError,"Unable to convert string to argv",
              "Memory allocation failed");
            return(False);
          }
        (void) strncpy(vector[i],p,q-p);
        vector[i][q-p]='\0';
        /*
          Convert any special HTML codes in place back to ASCII
        */
        HttpUnescape(vector[i], (char *) NULL);
        i++;
      }
    q++;
  }
  *argc=count+1;
  *argv=vector;
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  G e t F i l e M i m e T y p e                                              %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method GetFileMimeType given the file name, obtain MIME type for
%  "Content-type:" header field. We try to find MIME type string under
%  HCR\.xyz key, "Content Type" value. If that fails, we return default
%  "application/ocetet-stream".
%
%  The format of the GetFileMimeType method is:
%
%      void GetFileMimeType(LPCSTR pszPath, LPSTR pszType, DWORD cbMax)
%
%  A description of each parameter follows:
%
%    o pszPath:  file path
%
%    o pszType:  points to the buffer that will receive MIME type string
%
%    o cbMax:    specifies the maximum number of characters to copy to the
%                buffer, including the NUL character. If the text exceeds
%                this limit, it will be truncated.
%
*/
typedef struct _mime_spec
{
  char
    *extn,
    *type;
} mime_spec;

static mime_spec specs[] = {
  ".", "text/plain",
  ".htm", "text/html",
  ".html", "text/html",
  ".txt", "text/plain",
  ".gif", "image/gif",
  ".jpe", "image/jpeg",
  ".jpeg", "image/jpeg",
  ".jpg", "image/jpeg",
  ".pbm", "image/x-portable-bitmap",
  ".pgm", "image/x-portable-graymap",
  ".png", "image/png",
  ".pnm", "image/x-portable-anymap",
  ".ppm", "image/x-portable-pixmap",
  ".ras", "image/x-cmu-raster",
  ".rgb", "image/x-rgb",
  ".tif", "image/tiff",
  ".tiff", "image/tiff",
  ".xbm", "image/x-xbitmap",
  ".xpm", "image/x-xpixmap",
  ".xwd", "image/x-xwindowdump",
  ".avi", "video/msvideo",
  ".mov", "video/quicktime",
  ".mpe", "video/mpeg",
  ".mpeg", "video/mpeg",
  ".mpg", "video/mpeg",
  ".mp3", "audio/mpeg",
  ".wav", "audio/wav",
  ".bin", "application/octet-stream",
  ".eps", "application/postscript",
  ".exe", "application/octet-stream",
  ".gtar", "application/x-gtar",
  ".gz", "application/x-gzip",
  ".hdf", "application/x-hdf",
  ".jar", "application/java-archive",
  ".lzh", "application/x-lzh",
  ".pdf", "application/pdf",
  ".ps", "application/postscript",
  ".tar", "application/tar",
  ".tgz", "application/x-gzip",
  ".zip", "application/zip"
};

void GetFileMimeType(const char *pszPath, char *pszType, unsigned long cbMax)
{
  char
    *pszExt;

  /*
    set MIME type to empty string
  */
  *pszType = '\0';
  /*
    try to locate file extension
  */
  pszExt = strrchr( pszPath, '.');    
  if (pszExt != NULL)
    {
#ifdef USE_REGISTRY   
      HKEY
        hKey;

      unsigned long
        value_type;

      long
        result;

      /* 
        for file extension .xyz, MIME Type can be found
        HKEY_CLASSES_ROOT\.xyz key in the registry
      */
      result = RegOpenKeyEx(HKEY_CLASSES_ROOT,pszExt,0L,KEY_READ,&hKey);                     
      if (result == ERROR_SUCCESS)
        {
          /*
            we sucessfully opened the key.
            try getting the "Content Type" value
          */            
          result = RegQueryValueEx(hKey,"Content Type",NULL,&value_type, 
	                   (BYTE *)pszType,&cbMax);
          /*
            if we failed to get the value or it is not string,
            clear content-type field
          */            
          if (result != ERROR_SUCCESS || value_type != REG_SZ)
              *pszType = '\0';
          RegCloseKey( hKey );
        }
#else
      int
        i,
        tagcount = sizeof(specs) / sizeof(mime_spec);

      /* try to match this record to one of the ones in our named table */
      for (i=0; i< tagcount; i++)
      {
        if (LocaleCompare(specs[i].extn,pszExt) == 0)
          strncpy(pszType, specs[i].type, cbMax);
      }
#endif
    }    
  /*
    if at this point we don't have MIME type, use default
  */    
  if (*pszType == '\0')
    strncpy(pszType, "application/octet_stream", cbMax);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  H t t p E x t e n s i o n P r o c                                          %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  HttpExtensionProc - ISAPI / Win32 API method. This method is
%  required by IIS.  It is called once per client request. This
%  is where the extension accomplishes its purpose in life. See
%  Microsofts ISAPI API documentation for all the gory details.
%
*/
DWORD WINAPI HttpExtensionProc(EXTENSION_CONTROL_BLOCK *pECB)
{
  char
    **argv,
    **argv_hw,
    *errmsg;

  int
    argc,
    argc_hw,
    i;

  unsigned int
    impersonating,
    status;

  CHAR
    *lpszQuery,
    *lpszHeader=NULL,
    *lpszTemp=NULL;

  BOOL
    Success=FALSE;

	DWORD
    dwTotalBytes=0,
    dwLen=0;

  impersonating=False;
  pECB->dwHttpStatusCode=0; // 0 Failure
  if (!stricmp(pECB->lpszMethod, "get"))
    {
      lpszQuery = pECB->lpszQueryString;
      dwTotalBytes = lstrlen(lpszQuery);
	    Success=TRUE;
    }
  else if (!stricmp(pECB->lpszMethod, "post"))
    {
      if(pECB->cbTotalBytes > 0)
        {
          DWORD
            cbQuery;

          dwTotalBytes=pECB->cbTotalBytes;
          lpszTemp = (char*)LocalAlloc(LPTR, dwTotalBytes+1);
          if (!lpszTemp)
            return HSE_STATUS_ERROR;
          cbQuery = pECB->cbTotalBytes - pECB->cbAvailable;
          if (cbQuery > 0)
            {
              pECB->ReadClient(pECB->ConnID, 
                (LPVOID) (lpszTemp + pECB->cbAvailable),&cbQuery);
            }
          strncpy(lpszTemp, pECB->lpbData, pECB->cbAvailable);
          lpszTemp[dwTotalBytes]='\0';
          lpszQuery = lpszTemp;
	        Success=TRUE;
        }
    }
  if (Success)
    {
      status=CGIToArgv(lpszQuery,&argc,&argv);
      if (status == True)
        {
          char
            szMimeType[MaxTextExtent],
            szPathTranslated[MaxTextExtent];

          DWORD
            dwBuffSize;

          HSE_SEND_HEADER_EX_INFO HeaderExInfo;

	        dwBuffSize = MaxTextExtent;
          szPathTranslated[0] = '\0';
		      if (pECB->GetServerVariable(pECB->ConnID, 
                "PATH_TRANSLATED",szPathTranslated,&dwBuffSize))
            {
              GetFileMimeType(szPathTranslated,
                szMimeType,MaxTextExtent);
            }
          else
            strcpy(szMimeType,"application/ocetet-stream");
          for (argc_hw=1; argc_hw < argc; argc_hw++)
          {
            char
              *blob_data,
              szHeaders[MaxTextExtent];

            size_t
              blob_length;

            Image
              *image;

            ImageInfo
              *image_info;

            int
              mode=0;

            argv_hw = &argv[argc_hw];
            if (LocaleNCompare("-login",argv[argc_hw],8) == 0)
              {
                for (i=argc_hw; i < argc; i++)
                {
                  if (LocaleNCompare("login-",argv[i],8) == 0)
                    break;
                }
#if defined(WIN32)
                if ((i-argc_hw)>3)
                  {
                    char
                      *domain=argv_hw[1],
                      *userid=argv_hw[2],
                      *passwd=argv_hw[3];

                    HANDLE
                      hToken;

                    BOOL
                      status;

                    status = LogonUser(userid,domain,passwd,
                       LOGON32_LOGON_INTERACTIVE,LOGON32_PROVIDER_DEFAULT,&hToken);
                    if (status)
                      {
                        status = ImpersonateLoggedOnUser(hToken);
                        if (status)
                          impersonating=True;
                        else
                          errmsg = NTGetLastError(); 
                      }
                    else
                      errmsg = NTGetLastError();
                  }
#endif
                argc_hw = i+1;
                argv_hw = &argv[argc_hw];
              }
            if (LocaleNCompare("-convert",argv[argc_hw],8) == 0)
              {
                for (i=argc_hw; i < argc; i++)
                {
                  if (LocaleNCompare("-xbdat",argv[i],6) == 0)
                    {
                      argv[i+1]=(char *)(&blob_data);
                      mode=1;
                    }
                  else if (LocaleNCompare("-xblen",argv[i],6) == 0)
                    {
                      argv[i+1]=(char *)(&blob_length);
                      mode=1;
                    }
                  else if (LocaleNCompare("-xfunc",argv[i],6) == 0)
                    {
                      argv[i+1]=(char *)CGIFifo;
                      mode=2;
                    }
                  else if (LocaleNCompare("-xctxt",argv[i],6) == 0)
                    {
                      argv[i+1]=(char *)pECB;
                      mode=2;
                    }
                  else if (LocaleNCompare("-xinfo",argv[i],6) == 0)
                    {
                      argv[i+1]=(char *)(&image_info);
                      mode=3;
                    }
                  else if (LocaleNCompare("-ximag",argv[i],6) == 0)
                    {
                      argv[i+1]=(char *)(&image);
                      mode=3;
                    }
                  else if (LocaleNCompare("convert-",argv[i],8) == 0)
                    break;
                }
                if (mode==0)
                  {
                    convert_main(i-argc_hw,argv_hw);
                  }
                if (mode==1)
                  {
                    blob_length=8192;
                    convert_main(i-argc_hw,argv_hw);
                    dwLen=blob_length;

                    sprintf(szHeaders, 
                    "Content-Length: %u\r\nContent-Type: %s\r\n\r\n",
                      dwLen,szMimeType);
                    HeaderExInfo.pszStatus = "200 OK";
                    HeaderExInfo.pszHeader = szHeaders;
                    HeaderExInfo.cchStatus = strlen( HeaderExInfo.pszStatus );
                    HeaderExInfo.cchHeader = strlen( szHeaders );
                    HeaderExInfo.fKeepConn = FALSE;   
                    if (!pECB->ServerSupportFunction(pECB->ConnID,
                      HSE_REQ_SEND_RESPONSE_HEADER_EX,&HeaderExInfo,NULL,NULL))
		                    return HSE_STATUS_ERROR;
                    if (!pECB->WriteClient( pECB->ConnID, blob_data, &dwLen, 0))
		                    return HSE_STATUS_ERROR;
                  }
                if (mode==2)
                  {
                    sprintf(szHeaders,"Content-Type: %s\r\n\r\n",szMimeType);
                    HeaderExInfo.pszStatus = "200 OK";
                    HeaderExInfo.pszHeader = szHeaders;
                    HeaderExInfo.cchStatus = strlen( HeaderExInfo.pszStatus );
                    HeaderExInfo.cchHeader = strlen( szHeaders );
                    HeaderExInfo.fKeepConn = FALSE;   
                    if (!pECB->ServerSupportFunction(pECB->ConnID,
                      HSE_REQ_SEND_RESPONSE_HEADER_EX,&HeaderExInfo,NULL,NULL))
		                    return HSE_STATUS_ERROR;
                    convert_main(i-argc_hw,argv_hw);
                  }
                if (mode==3)
                  {
                    convert_main(i-argc_hw,argv_hw);
                  }
                argc_hw = i+1;
                argv_hw = &argv[argc_hw];
              }
            if (LocaleNCompare("-combine",argv[argc_hw],8) == 0)
              {
                for (i=argc_hw; i < argc; i++)
                {
                  if (LocaleNCompare("-xbdat",argv[i],6) == 0)
                    {
                      argv[i+1]=(char *)(&blob_data);
                      mode=1;
                    }
                  else if (LocaleNCompare("-xblen",argv[i],6) == 0)
                    {
                      argv[i+1]=(char *)(&blob_length);
                      mode=1;
                    }
                  else if (LocaleNCompare("-xfunc",argv[i],6) == 0)
                    {
                      argv[i+1]=(char *)CGIFifo;
                      mode=2;
                    }
                  else if (LocaleNCompare("-xctxt",argv[i],6) == 0)
                    {
                      argv[i+1]=(char *)pECB;
                      mode=2;
                    }
                  else if (LocaleNCompare("-xinfo",argv[i],6) == 0)
                    {
                      argv[i+1]=(char *)(&image_info);
                      mode=3;
                    }
                  else if (LocaleNCompare("-ximag",argv[i],6) == 0)
                    {
                      argv[i+1]=(char *)(&image);
                      mode=3;
                    }
                  else if (LocaleNCompare("combine-",argv[i],8) == 0)
                    break;
                }
                if (mode==0)
                  {
                    combine_main(i-argc_hw,argv_hw);
                  }
                if (mode==1)
                  {
                    blob_length=8192;
                    combine_main(i-argc_hw,argv_hw);
                    dwLen=blob_length;

                    sprintf(szHeaders, 
                    "Content-Length: %u\r\nContent-Type: %s\r\n\r\n",
                      dwLen,szMimeType);
                    HeaderExInfo.pszStatus = "200 OK";
                    HeaderExInfo.pszHeader = szHeaders;
                    HeaderExInfo.cchStatus = strlen( HeaderExInfo.pszStatus );
                    HeaderExInfo.cchHeader = strlen( szHeaders );
                    HeaderExInfo.fKeepConn = FALSE;   
                    if (!pECB->ServerSupportFunction(pECB->ConnID,
                      HSE_REQ_SEND_RESPONSE_HEADER_EX,&HeaderExInfo,NULL,NULL))
		                    return HSE_STATUS_ERROR;
                    if (!pECB->WriteClient( pECB->ConnID, blob_data, &dwLen, 0))
		                    return HSE_STATUS_ERROR;
                  }
                if (mode==2)
                  {
                    sprintf(szHeaders,"Content-Type: %s\r\n\r\n",szMimeType);
                    HeaderExInfo.pszStatus = "200 OK";
                    HeaderExInfo.pszHeader = szHeaders;
                    HeaderExInfo.cchStatus = strlen( HeaderExInfo.pszStatus );
                    HeaderExInfo.cchHeader = strlen( szHeaders );
                    HeaderExInfo.fKeepConn = FALSE;   
                    if (!pECB->ServerSupportFunction(pECB->ConnID,
                      HSE_REQ_SEND_RESPONSE_HEADER_EX,&HeaderExInfo,NULL,NULL))
		                    return HSE_STATUS_ERROR;
                    combine_main(i-argc_hw,argv_hw);
                  }
                if (mode==3)
                  {
                    combine_main(i-argc_hw,argv_hw);
                  }
                argc_hw = i+1;
                argv_hw = &argv[argc_hw];
              }
          }
        }
      // BUG: must free all the strings pointed to by argv not just argv
      LiberateMemory((void **) &argv);
    }

  if (impersonating==True)
    {
#if defined(WIN32)
      status = RevertToSelf();
      if (status)
        impersonating=False;
      else
        errmsg = NTGetLastError();
#endif
    }
  pECB->dwHttpStatusCode=200; // 200 OK
  if (lpszTemp)
    LocalFree(lpszTemp);

  return HSE_STATUS_SUCCESS;
}
