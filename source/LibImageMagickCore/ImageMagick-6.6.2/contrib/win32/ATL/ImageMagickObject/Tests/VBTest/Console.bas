Attribute VB_Name = "Console"
'=========================================================
'File: Console.bas
'
'Summary: Demonstrates how to create and manipulate a
'         console from Visual Basic.
'
'Classes: None
'
'Functions: ConsolePrint, ConsoleRead
'=========================================================

Option Explicit

Public Declare Function AllocConsole Lib "kernel32" () As Long

Public Declare Function FreeConsole Lib "kernel32" () As Long

Public Declare Function GetStdHandle Lib "kernel32" _
(ByVal nStdHandle As Long) As Long

Public Declare Function ReadConsole Lib "kernel32" Alias _
"ReadConsoleA" (ByVal hConsoleInput As Long, _
ByVal lpBuffer As String, ByVal nNumberOfCharsToRead As Long, _
lpNumberOfCharsRead As Long, lpReserved As Any) As Long

Public Declare Function SetConsoleMode Lib "kernel32" (ByVal _
hConsoleOutput As Long, dwMode As Long) As Long

Public Declare Function SetConsoleTextAttribute Lib _
"kernel32" (ByVal hConsoleOutput As Long, ByVal _
wAttributes As Long) As Long

Public Declare Function SetConsoleTitle Lib "kernel32" Alias _
"SetConsoleTitleA" (ByVal lpConsoleTitle As String) As Long

Public Declare Function WriteConsole Lib "kernel32" Alias _
"WriteConsoleA" (ByVal hConsoleOutput As Long, _
ByVal lpBuffer As Any, ByVal nNumberOfCharsToWrite As Long, _
lpNumberOfCharsWritten As Long, lpReserved As Any) As Long

Declare Function ReadFile Lib "kernel32" _
    (ByVal hFile As Long, _
    lpBuffer As Any, _
    ByVal nNumberOfBytesToRead As Long, _
    lpNumberOfBytesRead As Long, _
    lpOverlapped As Any) As Long
    
Declare Function WriteFile Lib "kernel32" _
    (ByVal hFile As Long, _
    ByVal lpBuffer As String, _
    ByVal nNumberOfBytesToWrite As Long, _
    lpNumberOfBytesWritten As Long, _
    lpOverlapped As Any) As Long
    
Declare Function SetFilePointer Lib "kernel32" _
   (ByVal hFile As Long, _
   ByVal lDistanceToMove As Long, _
   lpDistanceToMoveHigh As Long, _
   ByVal dwMoveMethod As Long) As Long
   
Declare Function SetEndOfFile Lib "kernel32" _
   (ByVal hFile As Long) As Long
    
'I/O handlers for the console window. These are much like the
'hWnd handlers to form windows.

Public Const STD_INPUT_HANDLE = -10&
Public Const STD_OUTPUT_HANDLE = -11&
Public Const STD_ERROR_HANDLE = -12&

'Color values for SetConsoleTextAttribute.
Public Const FOREGROUND_BLUE = &H1
Public Const FOREGROUND_GREEN = &H2
Public Const FOREGROUND_RED = &H4
Public Const FOREGROUND_INTENSITY = &H8
Public Const BACKGROUND_BLUE = &H10
Public Const BACKGROUND_GREEN = &H20
Public Const BACKGROUND_RED = &H40
Public Const BACKGROUND_INTENSITY = &H80

'For SetConsoleMode (input)
Public Const ENABLE_LINE_INPUT = &H2
Public Const ENABLE_ECHO_INPUT = &H4
Public Const ENABLE_MOUSE_INPUT = &H10
Public Const ENABLE_PROCESSED_INPUT = &H1
Public Const ENABLE_WINDOW_INPUT = &H8
'For SetConsoleMode (output)
Public Const ENABLE_PROCESSED_OUTPUT = &H1
Public Const ENABLE_WRAP_AT_EOL_OUTPUT = &H2

Public hConsoleIn As Long 'The console's input handle
Public hConsoleOut As Long 'The console's output handle
Public hConsoleErr As Long 'The console's error handle

'=========================================================
'Function: ConsolePrint
'
'Summary: Prints the output of a string
'
'Args: String ConsolePrint
'The string to be printed to the console's ouput buffer.
'
'Returns: None
'=========================================================

Public Sub ConsolePrint(szOut As String)
WriteConsole hConsoleOut, szOut, Len(szOut), vbNull, vbNull
End Sub

'=========================================================
'Function: ConsoleRead
'
'Summary: Gets a line of input from the user.
'
'Args: None
'
'Returns: String ConsoleRead
'The line of input from the user.
'=========================================================

Public Function ConsoleRead() As String
Dim sUserInput As String * 256
Call ReadConsole(hConsoleIn, sUserInput, Len(sUserInput), vbNull, vbNull)
'Trim off the NULL charactors and the CRLF.
ConsoleRead = Left$(sUserInput, InStr(sUserInput, Chr$(0)) - 3)
End Function
