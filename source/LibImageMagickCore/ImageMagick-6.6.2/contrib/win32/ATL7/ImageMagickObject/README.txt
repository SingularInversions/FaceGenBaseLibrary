The ImageMagickObject is a COM+ compatible component that may be invoked
from any langugae capable of using COM objects. The intended use it for
Windows Scripting Host VBS scripts and VIsual Basic, but it can also be
used from C++, ASP, and other lanaguages like Perl and PHP. 

The strategy with the ImageMagickObject COM+ component is not to the same
as with PerlMagick interface provided with ImageMagick. PerlMagick is a
low level API based system that defines a whole new way of scripintg IM
operations. The IM COM+ component simply provides access to the convert,
composite, mogrify, identify, and montage tools. The way you use it is
exactly the same. You pass it a list of strings including filenames and
various options and it does the job. In fact, you can take any exising
batch scripts that use the command line tools and translate them into the
equivalent calls to the COM+ object in a matter of minutes. Beyond that,
there is also a way to pass in and retrieve images in memory in the form
of standard smart arrays (byte arrays). Samples are provided, to show
both the simple and more elaborate forms of access.

You may execute the sample program from the Windows Command Shell like:

  cscript SimpleTest.vbs

Since the ImageMagick utility command line parsers are incorporated
within ImageMagickObject, please refer to the command line utility
documentation to learn how to use it. The sample VBS scripts show how
the object should be called and used and have lots of comments.

For C++ programmers - have a look at the MagickCMD.cpp command line
utility for an example of how to call the object from C++. This is a
bit complex because the object requires a variable size list of BSTR's
to emulate the command line argc, argv style calling conventions of the
COM component which is more complex in C++ then in VBS or VB.

Other goodies...

MagickCMD is a C++ sample, but it can also server as a replacement for
all the other command line utilities in most applications. Instead of
using "convert xxxx yyyy" you can use "MagickCMD convert xxxx yyyy"
instead. MagickCMD calls the COM object to get the job done. This small
tight combination replaces the entire usual binary distribution in just
a few megabytes. 


