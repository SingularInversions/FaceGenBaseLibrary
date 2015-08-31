VERSION 5.00
Begin VB.Form Form1 
   Caption         =   "Form1"
   ClientHeight    =   3195
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   4680
   LinkTopic       =   "Form1"
   ScaleHeight     =   3195
   ScaleWidth      =   4680
   StartUpPosition =   3  'Windows Default
End
Attribute VB_Name = "Form1"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
Option Compare Text

Declare Function WriteFile Lib "kernel32" _
  (ByVal hFile As Long, ByVal lpBuffer As String, ByVal nNumberOfBytesToWrite As Long, _
  lpNumberOfBytesWritten As Long, lpOverlapped As Any) As Long
Declare Function GetStdHandle Lib "kernel32" (ByVal nStdHandle As Long) As Long
Private Sub Main()
On Error GoTo Main_Err

Const STD_OUTPUT_HANDLE = -11&
Dim hStdOut As Long ' handle of Standard Output
Dim iArgCount As Integer
Dim argIter As Integer

' Early binding - img is an interface indentifier (IID)
' Dim img As ImageMagickObject.MagickImage
' Late binding - img is accessed via Dispatch interface
Dim img As Object

Dim myarray(1) As String
Dim output(1) As String
Dim info As String
Dim msgs As String
Dim elem As String

hStdOut = GetStdHandle(STD_OUTPUT_HANDLE)
SendConsoleMessage "Got: " & Command$
iArgCount = GetParamCount()

' If no arguments are supplied then the exe has been called for the purpose
' of setting values, not automation
If iArgCount < 0 Then
    Exit Sub
End If
For argIter = 1 To argCount
    SendConsoleMessage GetParam(argIter)
Next argIter

' Create the object using the ProgId of the class
Set img = CreateObject("ImageMagickObject.MagickImage.1")
' Create the object using a class identifier (The class CLSID)
' Set img = New ImageMagickObject.MagickImage

' If you create an object via the New keyword, the CLSID is read from the
' referenced component type library at build time and hardcoded in your
' component. If you use CreateObject VB queries the registry at run-time
' the map the ProgId to the CLSID (The CLSID is what you have to pass to
' the COM run-time when you ask it to create an object). As you may know
' you can even write:

myarray(0) = "8BIM:"
msgs = img.Convert("logo:", "-profile", "8BIMTEXT:iptctext.txt", myarray)
If Err.Number <> ERROR_SUCCESS Then ShowError: WScript.Quit
If IsArray(msgs) Then
  For Each elem In msgs
    SendConsoleMessage elem
  Next
End If
SendConsoleMessage "array: " & LenB(myarray)
output(0) = "APP1:"
msgs = img.Convert("logo:", "-profile", myarray, "-profile", "APP1JPEG:0000380556-005.jpg", output)
If Err.Number <> ERROR_SUCCESS Then ShowError: WScript.Quit
If IsArray(msgs) Then
  For Each elem In msgs
    SendConsoleMessage elem
  Next
End If
SendConsoleMessage "output: " & LenB(output)
'info = img("test")
'MsgBox "info: " & info
       
Main_Exit:
  Set img = Nothing
  Exit Sub

Main_Err:
    SendConsoleMessage "ERROR: " & Err.Description
    Resume Load_Exit
End Sub

Private Sub SendConsoleMessage(sMessage As String)
    Dim rc As Long
    Dim lBytesWritten As Long

    sMessage = sMessage & vbCrLf
    rc = WriteFile(hStdOut, sMessage, Len(sMessage), lBytesWritten, ByVal 0&)
End Sub

Public Function GetParam(Count As Integer) As String
    Dim i As Long
    Dim j As Integer
    Dim c As String
    Dim bInside As Boolean
    Dim bQuoted As Boolean

    j = 1
    bInside = False
    bQuoted = False
    GetParam = ""
    For i = 1 To Len(Command$)
        c = Mid$(Command$, i, 1)
        If bInside And bQuoted Then
            If c = """" Then
                j = j + 1
                bInside = False
                bQuoted = False
            End If
        ElseIf bInside And Not bQuoted Then
            If c = " " Then
                j = j + 1
                bInside = False
                bQuoted = False
            End If
        Else
            If c = """" Then
                If j > Count Then Exit Function
                bInside = True
                bQuoted = True
            ElseIf c <> " " Then
                If j > Count Then Exit Function
                bInside = True
                bQuoted = False
            End If
        End If
        If bInside And j = Count And c <> """" Then GetParam = GetParam & c
    Next i
End Function

Public Function GetParamCount() As Integer
    Dim i As Long
    Dim sNextChar As String
    Dim bInside As Boolean
    Dim bQuoted As Boolean
    Dim sCommand As String
        
    GetParamCount = 0
    bInsideParameter = False
    bQuoted = False
    sCommand = Command$
    
    For i = 1 To Len(sCommand)
        sNextChar = Mid$(sCommand, i, 1)
        If bInsideParameter Then
            If bQuoted Then
                If sNextChar = """" Then
                    GetParamCount = GetParamCount + 1
                    bInsideParameter = False
                    bQuoted = False
                End If
            Else
                If sNextChar = " " Then
                    GetParamCount = GetParamCount + 1
                    bInsideParameter = False
                    bQuoted = False
                End If
            End If
        Else
            bInsideParameter = True
            If sNextChar = """" Then
                bQuoted = True
            ElseIf sNextChar <> " " Then
                bQuoted = False
            End If
        End If
    Next i
    
    If bInsideParameter Then GetParamCount = GetParamCount + 1
End Function

Private Sub Form_Load()

End Sub
