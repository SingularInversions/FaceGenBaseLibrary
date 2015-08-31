This is a basic ATL/COM wrapper for the Image object using the
ImageMagick++ classes. There isn't much in the way of comments in here,
since the code is pretty cookie-cutter. Lots of "TODO" comments that
should be removed or replaced with something a little more descriptive.

The Composite() method shows one way to handle multiple image objects.
As the comments point out, this is a COM hack. It's no reallu a valid way
to use COM, even though it works. I haven't been able to come up with
a "legal" way to do this. 

TODO: Methods that require a Color object haven't been implemented yet.
      The basic idea was for the user to instanciate a MagickCOM.Color
      object, set it's properties, then pass this object to the Image 
      method(s). Those methods would then reconstruct a C++ version of
      the class which finally gets used.

      A lot of the overloaded methods haven't been implemented either. 
      I picked the most common ones to implement first. The overloaded
      versions will require a new method name for them to work in COM. 

Written by: Paul Mrozowski
            Senior Applications Developer
    
            Kirtland Associates, Inc.
            1220 Morse
            Suite 200
            Royal Oak, MI 48067
            Phone: 248.542.2675

            Email: pcm@kirtlandsys.com

            Copyright(C) 2002 - Kirtland Associates

            See the file Copyright.txt for usage restrictions.


