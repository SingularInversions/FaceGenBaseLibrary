Attribute VB_Name = "SimpleTest"
Option Explicit
Option Compare Text

Declare Function GetStdHandle Lib "kernel32" _
  (ByVal nStdHandle As Long) As Long
Declare Function WriteFile Lib "kernel32" _
  (ByVal hFile As Long, ByVal lpBuffer As String, _
  ByVal nNumberOfBytesToWrite As Long, _
  lpNumberOfBytesWritten As Long, lpOverlapped As Any) As Long
Declare Sub CopyMemory Lib "kernel32" Alias "RtlMoveMemory" _
  (pDst As Any, pSrc As Any, ByVal ByteLen As Long)
Private Declare Function lstrlenW Lib "kernel32" _
  (lpString As Any) As Long
Public Const STD_OUTPUT_HANDLE = -11&
Public Const ERROR_SUCCESS = 0
Public hStdOut As Long ' handle of Standard Output

Private Sub Main()
On Error GoTo Main_Err

Dim iArgCount As Integer
Dim iArgIter As Integer

' Early binding - img is an interface indentifier (IID)
Dim img As MagickImage
' Late binding - img is accessed via Dispatch interface
' Dim img As Object

Dim msgs As Variant
Dim strMsgs As String
Dim elem As Variant

hStdOut = GetStdHandle(STD_OUTPUT_HANDLE)
'SendConsoleMessage "Got: " & Command$
iArgCount = GetParamCount()

' If no arguments are supplied then the exe has been called for the purpose
' of setting values, not automation
'If iArgCount < 1 Then
'   Exit Sub
'End If
'For iArgIter = 1 To iArgCount
'    SendConsoleMessage GetParam(iArgIter)
'Next iArgIter

' Create the object using the ProgId of the class
' Set img = CreateObject("ImageMagickObject.MagickImage.1")
' Create the object using a class identifier (The class CLSID)
Set img = New MagickImage

msgs = img.Convert("logo:", "logo.jpg")
'If Err.Number <> ERROR_SUCCESS Then GoTo Main_Err
           
Main_Exit:
  Set img = Nothing
  Exit Sub

Main_Err:
  strMsgs = ""
  ' All ImageMagickObject errors will be above 5000
  If BasicError(Err.Number) > 5000 Then
    msgs = img.Messages
    If IsArray(msgs) Then
      For Each elem In msgs
        strMsgs = strMsgs & elem & vbCrLf
      Next
    End If
  End If
  SendConsoleMessage "ERROR: " & BasicError(Err.Number) & ": " & Err.Description & vbCrLf & strMsgs
  'Resume Main_Exit
  Resume Next
End Sub

Private Sub SendConsoleMessage(sMessage As String)
    Dim rc As Long
    Dim lBytesWritten As Long

    MsgBox sMessage
    Exit Sub
    sMessage = sMessage & vbCrLf
    rc = WriteFile(hStdOut, sMessage, Len(sMessage), lBytesWritten, ByVal 0&)
End Sub

Function BasicError(ByVal e As Long) As Long
    BasicError = e And &HFFFF&
End Function

Function COMError(e As Long) As Long
    COMError = e Or vbObjectError
End Function

