
ImageMagickObjectps.dll: dlldata.obj ImageMagickObject_p.obj ImageMagickObject_i.obj
	link /dll /out:ImageMagickObjectps.dll /def:ImageMagickObjectps.def /entry:DllMain dlldata.obj ImageMagickObject_p.obj ImageMagickObject_i.obj \
		kernel32.lib rpcndr.lib rpcns4.lib rpcrt4.lib oleaut32.lib uuid.lib \

.c.obj:
	cl /c /Ox /DWIN32 /D_WIN32_WINNT=0x0400 /DREGISTER_PROXY_DLL \
		$<

clean:
	@del ImageMagickObjectps.dll
	@del ImageMagickObjectps.lib
	@del ImageMagickObjectps.exp
	@del dlldata.obj
	@del ImageMagickObject_p.obj
	@del ImageMagickObject_i.obj
