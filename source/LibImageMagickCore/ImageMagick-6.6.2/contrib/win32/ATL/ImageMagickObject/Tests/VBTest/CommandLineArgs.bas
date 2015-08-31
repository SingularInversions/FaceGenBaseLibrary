Attribute VB_Name = "CommandLineArgs"
'=========================================================
'File: CommandLineArgs.bas
'
'Summary: Demonstrates how to access and parse the command
'         line arguments in Visual Basic.
'
'Classes: None
'
'Functions: GetParamCount, GetParam
'=========================================================

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

