Option Explicit

On Error Resume Next
Const ERROR_SUCCESS = 0

Dim img
Dim info
Dim msgs
Dim elem
Dim sMsgs

'
' This is the simplest sample I could come up with. It creates
' the ImageMagick COM object and then sends a copy of the IM
' logo out to a JPEG image files on disk.
'
Set img = CreateObject("ImageMagickObject.MagickImage.1")
'
' The methods for the IM COM object are identical to utility
' command line utilities. You have convert, composite, identify,
' mogrify, and montage. We did not bother with animate, and
' display since they have no purpose in this context.
'
' The argument list is exactly the same as the utility programs
' as a list of strings. In fact you should just be able to
' copy and past - do simple editing and it will work. See the
' other samples for more elaborate command sequences and the
' documentation for the utility programs for more details.
'
msgs = img.Convert("logo:","-format","%m,%h,%w","info:")
'
' By default - the string returned is the height, width, and the
' type of the image that was output. You can control this using
' the -format "xxxxxx" command as documented by identify.
'
If Err.Number <> ERROR_SUCCESS Then ShowError: WScript.Quit
MsgBox "info: " & msgs
Set img=Nothing
WScript.Quit(0)

Sub ShowError
  sMsgs = ""
  If BasicError(Err.Number) > 5000 Then
    sMsgs = " ImageMagickObject" & vbCrLf
  End If
  WScript.Echo Err.Number & ": " & Err.Description & vbCrLf & sMsgs
End Sub

Function BasicError(e)
  BasicError = e And &HFFFF&
End Function
