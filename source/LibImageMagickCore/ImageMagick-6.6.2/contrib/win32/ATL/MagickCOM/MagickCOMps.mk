
MagickCOMps.dll: dlldata.obj MagickCOM_p.obj MagickCOM_i.obj
	link /dll /out:MagickCOMps.dll /def:MagickCOMps.def /entry:DllMain dlldata.obj MagickCOM_p.obj MagickCOM_i.obj \
		kernel32.lib rpcndr.lib rpcns4.lib rpcrt4.lib oleaut32.lib uuid.lib \

.c.obj:
	cl /c /Ox /DWIN32 /D_WIN32_WINNT=0x0400 /DREGISTER_PROXY_DLL \
		$<

clean:
	@del MagickCOMps.dll
	@del MagickCOMps.lib
	@del MagickCOMps.exp
	@del dlldata.obj
	@del MagickCOM_p.obj
	@del MagickCOM_i.obj
