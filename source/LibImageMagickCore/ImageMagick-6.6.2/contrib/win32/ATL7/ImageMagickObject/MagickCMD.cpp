#include <iostream>
#include <atlbase.h>
#include <atlsafe.h>
#import "ImageMagickObject.tlb" no_namespace

typedef enum
{
  cmdUnknown,
  cmdCompare,
  cmdComposite,
  cmdConvert,
  cmdIdentify,
  cmdMogrify,
  cmdMontage,
  cmdStream
} CommandType;

static struct
  Commands
  {
    LPCTSTR name;
    CommandType code;
  } Commands[] =
  {
    { _T(""),          cmdUnknown   },
    { _T("compare"),   cmdCompare   },
    { _T("composite"), cmdComposite },
    { _T("convert"),   cmdConvert   },
    { _T("identify"),  cmdIdentify  },
    { _T("mogrify"),   cmdMogrify   },
    { _T("montage"),   cmdMontage   },
    { _T("stream"),    cmdStream   }
  };

int _tmain(int argc, TCHAR* argv[])
{
  int
    index,
    status = 0;

  LPCTSTR
    name;

  CommandType
    code = cmdUnknown;

  // We must have at least a command, input, and output
  if (argc < 4)
    return 0;

  index = 1;
  while ((name = Commands[index].name))
  {
    if (_tcsicmp(name,argv[1]) == 0)
    {
      code = Commands[index].code;
      break;
    }
    index++;
  }
  if (code == cmdUnknown)
    return 0;

  CoInitialize(NULL);

  try
  {
    CComVariant
      result;

    SAFEARRAY
      **ppsa = (SAFEARRAY**) NULL;

    IMagickImagePtr pImageProc(__uuidof(MagickImage));
    if (pImageProc == NULL)
      {
        status = 1;
      }
    else
    {
      {
        // Define the array bound structure
        CComSafeArrayBound bound[1];
        bound[0].SetCount(argc-2);
        bound[0].SetLowerBound(0);
        CComSafeArray<VARIANT> args(bound);
        if( !args )
          status = 2;
        else
        {
          for(index = 2; index < argc; ++index)
          {
            CComVariant vt(argv[index]);
            HRESULT hr = vt.Detach(&args[index-2]);
          }

          switch(code)
          {
            case cmdCompare:
              result = pImageProc->Compare(args.GetSafeArrayPtr());
              break;
            case cmdComposite:
              result = pImageProc->Composite(args.GetSafeArrayPtr());
              break;
            case cmdConvert:
              result = pImageProc->Convert(args.GetSafeArrayPtr());
              break;
            case cmdIdentify:
              result = pImageProc->Identify(args.GetSafeArrayPtr());
              break;
            case cmdMogrify:
              result = pImageProc->Mogrify(args.GetSafeArrayPtr());
              break;
            case cmdMontage:
              result = pImageProc->Montage(args.GetSafeArrayPtr());
              break;
            case cmdStream:
              result = pImageProc->Stream(args.GetSafeArrayPtr());
              break;
          }
          pImageProc.Release();
        }
      }
    }
  }
  catch(_com_error ex)
  {
    HRESULT res = ex.Error();  
    _bstr_t desc = ex.Description();  
    _ftprintf( stderr, _T("Error %ws (0x%08x)\n"), (wchar_t*)desc , res );
    status = 4;
  }

  CoUninitialize();
  return status;
}

