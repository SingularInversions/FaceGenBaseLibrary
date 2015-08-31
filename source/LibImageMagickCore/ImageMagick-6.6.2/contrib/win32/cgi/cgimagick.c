/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                             C G I M A G I C K                               %
%                                                                             %
%                                                                             %
%        A shim layer to give command line utilities a CGI interface.         %
%                                                                             %
%                                                                             %
%                                                                             %
%                              Software Design                                %
%                           William T. Radcliffe                              %
%                                June 2001                                    %
%                                                                             %
%                                                                             %
%  Copyright (C) 2001 ImageMagick Studio, a non-profit organization dedicated %
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
%  This is a simple shim that provides a CGI layer for calling into any of the
%  normal command line utilities.
*/

/*
  Include declarations.
*/
#include "magick/magick.h"
#include "magick/define.h"
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
  fwrite(data,1,length,stdout);
  return(length);
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
          && isxdigit((int) string [1]) && isxdigit((int) string [2]))
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
%  C G I G e t I n p u t                                                      %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method CGIGetInput returns the result of a CGI GET or POST request to the
%  caller as a simple string buffer.
%
%  The format of the CGIGetInput method is:
%
%      char *CGIGetInput(CGIAccessType method)
%
%  A description of each parameter follows:
%
%    o method:  Specifies the type of access to look for. This can be GET,
%               POST, or both (the most typical).
%
*/

typedef enum
{
  CGI_GET,
  CGI_POST,
  CGI_ANY
} CGIAccessType;

char *CGIGetInput(CGIAccessType iMethod)
{
  char
    *strHead,
    *strRetBuf;

  int
    iStdinLen = 0,
    iMethodWas = 0;

  if (iMethod == CGI_POST || iMethod == CGI_ANY)
    {
      if (getenv ("CONTENT_LENGTH"))
        {
          iStdinLen = atoi (getenv ("CONTENT_LENGTH"));
          iMethodWas = CGI_POST;
        }
    }
  if (iMethod == CGI_GET || (iMethod == CGI_ANY && !iStdinLen))
    {
      if (getenv ("QUERY_STRING"))
        {
          iStdinLen = strlen (getenv ("QUERY_STRING"));
          iMethodWas = CGI_GET;
        }
    }
  if (!iStdinLen)
      return (NULL);

  strHead = strRetBuf = (char *) AcquireMemory(sizeof (char) * iStdinLen + 1);
  if (strHead == (char *) NULL)
      return (NULL);

  memset (strRetBuf, 0, iStdinLen + 1);
  if (iMethodWas == CGI_POST)
    {
#ifdef WIN32
			setmode(fileno(stdin), O_BINARY);
#endif
      fread (strRetBuf, sizeof (char), iStdinLen, stdin);
    }
  else
    strncpy (strRetBuf, getenv ("QUERY_STRING"), (iStdinLen + 1));

  return (*strHead? strHead: NULL);
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

  assert(argc != (int *) NULL);
  assert(argv != (char ***) NULL);
  if (text == (char *) NULL)
    return(False);
  if (argc == (int *) NULL)
    return(False);
  if (argv == (char ***) NULL)
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
  vector[0]=AllocateString("cgimagick");
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
  { (char *) ".", (char *) "text/plain" },
  { (char *) ".htm", (char *) "text/html" },
  { (char *) ".html", (char *) "text/html" },
  { (char *) ".txt", (char *) "text/plain" },
  { (char *) ".gif", (char *) "image/gif" },
  { (char *) ".jpe", (char *) "image/jpeg" },
  { (char *) ".jpeg", (char *) "image/jpeg" },
  { (char *) ".jpg", (char *) "image/jpeg" },
  { (char *) ".pbm", (char *) "image/x-portable-bitmap" },
  { (char *) ".pgm", (char *) "image/x-portable-graymap" },
  { (char *) ".png", (char *) "image/png" },
  { (char *) ".pnm", (char *) "image/x-portable-anymap" },
  { (char *) ".ppm", (char *) "image/x-portable-pixmap" },
  { (char *) ".ras", (char *) "image/x-cmu-raster" },
  { (char *) ".rgb", (char *) "image/x-rgb" },
  { (char *) ".tif", (char *) "image/tiff" },
  { (char *) ".tiff", (char *) "image/tiff" },
  { (char *) ".xbm", (char *) "image/x-xbitmap" },
  { (char *) ".xpm", (char *) "image/x-xpixmap" },
  { (char *) ".xwd", (char *) "image/x-xwindowdump" },
  { (char *) ".avi", (char *) "video/msvideo" },
  { (char *) ".mov", (char *) "video/quicktime" },
  { (char *) ".mpe", (char *) "video/mpeg" },
  { (char *) ".mpeg", (char *) "video/mpeg" },
  { (char *) ".mpg", (char *) "video/mpeg" },
  { (char *) ".mp3", (char *) "audio/mpeg" },
  { (char *) ".wav", (char *) "audio/wav" },
  { (char *) ".bin", (char *) "application/octet-stream" },
  { (char *) ".eps", (char *) "application/postscript" },
  { (char *) ".exe", (char *) "application/octet-stream" },
  { (char *) ".gtar", (char *) "application/x-gtar" },
  { (char *) ".gz", (char *) "application/x-gzip" },
  { (char *) ".hdf", (char *) "application/x-hdf" },
  { (char *) ".jar", (char *) "application/java-archive" },
  { (char *) ".lzh", (char *) "application/x-lzh" },
  { (char *) ".pdf", (char *) "application/pdf" },
  { (char *) ".ps", (char *) "application/postscript" },
  { (char *) ".tar", (char *) "application/tar" },
  { (char *) ".tgz", (char *) "application/x-gzip" },
  { (char *) ".zip", (char *) "application/zip" }
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
  pszExt = (char *) strrchr( pszPath, '.');    
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
%  M a i n                                                                    %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
*/
#define IMAGELIST_SIZE  8

int main(int argc,char **argv)
{
  char
    **argv_hw,
    *errmsg,
    prefix[MaxTextExtent];

  int
    argc_hw,
    i,
    list_position;

  unsigned int
    impersonating,
    status;

  Image
    *imag_list[IMAGELIST_SIZE];

  ImageInfo
    *info_list[IMAGELIST_SIZE];

  /*
    Initialize command line arguments.
  */
  impersonating=False;
  errmsg=(char *) NULL;
  ReadCommandlLine(argc,&argv);
  InitializeMagick(*argv);

  for (i=0; i < IMAGELIST_SIZE; i++)
  {
    imag_list[i]=(Image *) NULL;
    info_list[i]=(ImageInfo *) NULL;
  }
  list_position=0;

#if defined(_VISUALC_)
  _setmode(_fileno(stdout),_O_BINARY);
#endif
	if (getenv("GATEWAY_INTERFACE") || (argc>1))
    {
      if (!getenv("GATEWAY_INTERFACE"))
        status=CGIToArgv(argv[1],&argc,&argv);
      else
        {
          char
            *query;

          query=CGIGetInput(CGI_ANY);
          status=CGIToArgv(query,&argc,&argv);
          LiberateMemory((void **) &query);
        }
      if (status == True)
        {
          char
            szMimeType[MaxTextExtent],
            szPathTranslated[MaxTextExtent];

          szPathTranslated[0] = '\0';          
		      if (getenv("PATH_TRANSLATED") != (char *) NULL)
            {
              strcpy(szPathTranslated,getenv("PATH_TRANSLATED"));
              GetFileMimeType(szPathTranslated,
                szMimeType,MaxTextExtent);
            }
          else
            strcpy(szMimeType,"application/ocetet-stream");
          for (argc_hw=1; argc_hw < argc; argc_hw++)
          {
            char
              *blob_data;

            size_t
              blob_length;

            int
              mode=0;

            argv_hw = &argv[argc_hw];
            if (LocaleNCompare("-login",argv[argc_hw],8) == 0)
              {
                for (i=argc_hw+1; i < argc; i++)
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
                argc_hw = i;
                argv_hw = &argv[argc_hw];
              }
            if (LocaleNCompare("-convert",argv[argc_hw],8) == 0)
              {
                char
                  *option;

                Image
                  *image;

                ImageInfo
                  *image_info;

                image=(Image *) NULL;
                image_info=(ImageInfo *) NULL;

                for (i=argc_hw+1; i < argc; i++)
                {
                  option=argv[i];
                  if (LocaleNCompare("convert-",option,8) == 0)
                    break;
                  if ((list_position > 0) && (*option != '-') && (*option != '+'))
                    {
                      char
                        *text;

                      text=TranslateText(info_list[list_position-1],
                                        imag_list[list_position-1],option);
                      if (text != (char *) NULL)
                        {
                          LiberateMemory((void **) &(argv[i]));
                          argv[i]=text;
                        }
                    }
                  else if (LocaleNCompare("-xbdat",option,6) == 0)
                    {
                      i++;
                      LiberateMemory((void **) &(argv[i]));
                      argv[i]=(char *)(&blob_data);
                      mode=1;
                    }
                  else if (LocaleNCompare("-xblen",option,6) == 0)
                    {
                      i++;
                      LiberateMemory((void **) &(argv[i]));
                      argv[i]=(char *)(&blob_length);
                      mode=1;
                    }
                  else if (LocaleNCompare("-xfunc",option,6) == 0)
                    {
                      i++;
                      LiberateMemory((void **) &(argv[i]));
                      argv[i]=(char *) CGIFifo;
                      mode=2;
                    }
                  else if (LocaleNCompare("-xctxt",option,6) == 0)
                    {
                      i++;
                      LiberateMemory((void **) &(argv[i]));
                      argv[i]=(char *) NULL;
                      mode=2;
                    }
                  else if (LocaleNCompare("-xinfo",option,6) == 0)
                    {
                      i++;
                      LiberateMemory((void **) &(argv[i]));
                      argv[i]=(char *)(&image_info);
                      mode=3;
                    }
                  else if (LocaleNCompare("-ximag",option,6) == 0)
                    {
                      i++;
                      LiberateMemory((void **) &(argv[i]));
                      argv[i]=(char *)(&image);
                      mode=3;
                    }
                }
                if (mode==0)
                  {
                    convert_main(i-argc_hw,argv_hw);
                    /* normal case were image was written to a disk file */
                  }
                else if (mode==1)
                  {
                    blob_length=8192;
                    convert_main(i-argc_hw,argv_hw);
                    /* returns an a blob and its length */
                    (void) FormatMagickString(prefix,MaxTextExtent,
                      "HTTP/1.0 200 Ok\nContent-Length: %u\r\nContent-Type: %s\n\n",
                        blob_length,szMimeType);
                    fwrite(prefix,1,strlen(prefix),stdout);
                    fwrite(blob_data,1,blob_length,stdout);
                  }
                else if (mode==2)
                  {
                    (void) FormatMagickString(prefix,MaxTextExtent,
                      "HTTP/1.0 200 Ok\nContent-Type: %s\n\n",szMimeType);
                    fwrite(prefix,1,strlen(prefix),stdout);
                    convert_main(i-argc_hw,argv_hw);
                    /* returns nothing - image has been stream already */
                  }
                else if (mode==3)
                  {
                    convert_main(i-argc_hw,argv_hw);
                    /* returns an image_info and image structure */

                    if ((list_position >= 0) &&
                         (list_position < IMAGELIST_SIZE) &&
                            (image != (Image *) NULL) &&
                              (image_info != (ImageInfo *) NULL))
                      {
                        status=WriteImage(image_info,image);
                        if (status == True)
                          {
                            imag_list[list_position]=image;
                            image=(Image *) NULL;
                            info_list[list_position]=image_info;
                            image_info=(ImageInfo *) NULL;
                            list_position++;
                          }
                      }
                  }
                argc_hw = i;
                argv_hw = &argv[argc_hw];
              }
            if (LocaleNCompare("-combine",argv[argc_hw],8) == 0)
              {
                char
                  *option;

                Image
                  *image;

                ImageInfo
                  *image_info;

                image=(Image *) NULL;
                image_info=(ImageInfo *) NULL;

                for (i=argc_hw+1; i < argc; i++)
                {
                  option=argv[i];
                  if (LocaleNCompare("combine-",argv[i],8) == 0)
                    break;
                  if ((list_position > 0) && (*option != '-') && (*option != '+'))
                    {
                      char
                        *text;

                      text=TranslateText(info_list[list_position-1],
                                        imag_list[list_position-1],option);
                      if (text != (char *) NULL)
                        {
                          LiberateMemory((void **) &(argv[i]));
                          argv[i]=text;
                        }
                    }
                  else if (LocaleNCompare("-xbdat",option,6) == 0)
                    {
                      i++;
                      LiberateMemory((void **) &(argv[i]));
                      argv[i]=(char *)(&blob_data);
                      mode=1;
                    }
                  else if (LocaleNCompare("-xblen",option,6) == 0)
                    {
                      i++;
                      LiberateMemory((void **) &(argv[i]));
                      argv[i]=(char *)(&blob_length);
                      mode=1;
                    }
                  else if (LocaleNCompare("-xfunc",option,6) == 0)
                    {
                      i++;
                      LiberateMemory((void **) &(argv[i]));
                      argv[i]=(char *)CGIFifo;
                      mode=2;
                    }
                  else if (LocaleNCompare("-xctxt",option,6) == 0)
                    {
                      i++;
                      LiberateMemory((void **) &(argv[i]));
                      argv[i]=(char *)NULL;
                      mode=2;
                    }
                  else if (LocaleNCompare("-xinfo",option,6) == 0)
                    {
                      i++;
                      LiberateMemory((void **) &(argv[i]));
                      argv[i]=(char *)(&image_info);
                      mode=3;
                    }
                  else if (LocaleNCompare("-ximag",option,6) == 0)
                    {
                      i++;
                      LiberateMemory((void **) &(argv[i]));
                      argv[i]=(char *)(&image);
                      mode=3;
                    }
                }
                if (mode==0)
                  {
                    combine_main(i-argc_hw,argv_hw);
                    /* normal case were image was written to a disk file */
                  }
                else if (mode==1)
                  {
                    blob_length=8192;
                    combine_main(i-argc_hw,argv_hw);
                    /* returns an a blob and its length */
                    (void) FormatMagickString(prefix,MaxTextExtent,
                      "HTTP/1.0 200 Ok\nContent-Length: %u\r\nContent-Type: %s\n\n",
                        blob_length,szMimeType);
                    fwrite(prefix,1,strlen(prefix),stdout);
                    fwrite(blob_data,1,blob_length,stdout);
                  }
                else if (mode==2)
                  {
                    (void) FormatMagickString(prefix,MaxTextExtent,
                      "HTTP/1.0 200 Ok\nContent-Type: %s\n\n",szMimeType);
                    fwrite(prefix,1,strlen(prefix),stdout);
                    combine_main(i-argc_hw,argv_hw);
                    /* returns nothing - image has been stream already */
                  }
                else if (mode==3)
                  {
                    combine_main(i-argc_hw,argv_hw);
                    /* returns an image_info and image structure */

                    if ((list_position >= 0) &&
                         (list_position < IMAGELIST_SIZE) &&
                            (image != (Image *) NULL) &&
                              (image_info != (ImageInfo *) NULL))
                      {
                        status=WriteImage(image_info,image);
                        if (status == True)
                          {
                            imag_list[list_position]=image;
                            image=(Image *) NULL;
                            info_list[list_position]=image_info;
                            image_info=(ImageInfo *) NULL;
                            list_position++;
                          }
                      }
                  }
                argc_hw = i;
                argv_hw = &argv[argc_hw];
              }
          }
        }
    }
  for (i=0; i < IMAGELIST_SIZE; i++)
  {
    if (imag_list[i] != (Image *) NULL)
      DestroyImage(imag_list[i]);
    if (info_list[i] != (ImageInfo *) NULL)
      DestroyImageInfo(info_list[i]);
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
  LiberateMemory((void **) &argv);
  Exit(0);
  return(False);
}
