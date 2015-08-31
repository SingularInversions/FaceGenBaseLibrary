' IMExec.bas - cover functions to find the ImageMagick installation
'          and invoke the "Convert.exe" in a silent sub-shell.
'          Could be extended to other IM executables.
'          The IMConvert function will return whatever
'          convert.exe returns, which seems to be 0 if
'          there is no error.
'
'
' Original:  Tom Bishop
' (granted to the public domain, 20-Oct-2003)



Option Explicit

Private imPath As String
Const IM_NOT_INSTALLED = 112233


Public Function IMConvert(ifn As String, cmds As String, ofn As String) As Long
    If IMCheck Then
        IMConvert = ShellAndWait("""" & imPath & "convert.exe"" """ _
                      & ifn & """ " & cmds & " """ & ofn & """")
    Else
        IMConvert = IM_NOT_INSTALLED
    End If
End Function
    
    
Public Function IMCheck() As Boolean
    If Len(imPath) > 0 Then
        IMCheck = True
        Exit Function
    End If
    
    On Error Resume Next
    
    Dim a() As String, P As String, l As Long, i As Long
    a = Split(Environ("path"), ";")
    For i = 0 To UBound(a)
        P = Trim(a(i))
        If Right(P, 1) <> "\" Then P = P & "\"
        l = 0: l = FileLen(P & "conjure.exe")
        If l > 50000 Then
            l = 0: l = FileLen(P & "convert.exe")
            If l > 50000 Then
                imPath = P
                IMCheck = True
                Exit Function
            End If
        End If
    Next
End Function

    

