The ImageMagickObject OLE Control may be invoked from Windows Script
(wsh).

The ImageMagickObject OLE Control provides access to the convert,
mogrify, etc., command lines from a WSH scripted environment.

The sample provided in this directory accomplishes the same result as

  convert logo: logo.jpg

except that the "convert" code is embedded in the ImageMagickObject DLL
so no external program is run.

You may execute this sample program from the Windows Command Shell like:

  cscript SimpleTest.vbs

The output can be viewed like:

  imdisplay logo.jpg

Since the ImageMagick utility command line parsers are incorporated
within ImageMagickObject, please refer to the command line utility
documentation to learn how to use it.

