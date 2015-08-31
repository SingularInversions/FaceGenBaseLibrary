The ImageMagickObject OLE Control may be invoked from Windows Script
WSH (cscript, wscript, Internet Explorer, etc.) or Visual Basic. It is
currently *not* usable with Internet Information Server (IIS) due
to threading issues. Please do not attempt to use this OLE control
with IIS.

The ImageMagickObject OLE Control provides access to the convert,
mogrify, etc., command lines from a Windows Script or Visual Basic
environment except that the utility code is embedded in the
ImageMagickObject DLL so no external program is run..

Sample code is provided for Visual Basic and WSH that accomplishes the
same result as

 convert logo: logo.jpg

Since the ImageMagick utility command line parsers are incorporated
within ImageMagickObject, please refer to the command line utility
documentation to learn how to use it.

The ImageMagickObject OLE Control is written by
Bill Radcliffe <BillR@corbis.com>
