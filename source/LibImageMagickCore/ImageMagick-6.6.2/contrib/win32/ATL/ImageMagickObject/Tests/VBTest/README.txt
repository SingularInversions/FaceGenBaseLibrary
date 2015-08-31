The ImageMagickObject OLE Control may be invoked from Visual Basic
programs.

The ImageMagickObject OLE Control provides access to the convert,
mogrify, etc., command lines from the Visual Basic environment.

The sample provided in this directory (SimpleTest.bas) accomplishes
the same result as

 convert logo: logo.jpg

except that the "convert" code is embedded in the ImageMagickObject DLL
so no external program is run.

Since the ImageMagick utility command line parsers are incorporated
within ImageMagickObject, please refer to the command line utility
documentation to learn how to use it.

