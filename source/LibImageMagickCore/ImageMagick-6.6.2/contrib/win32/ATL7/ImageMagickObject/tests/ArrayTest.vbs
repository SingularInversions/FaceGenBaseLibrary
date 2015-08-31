Option Explicit

'On Error Resume Next
Const ERROR_SUCCESS = 0

Dim img
Dim myarray(1)
Dim output(1)
Dim persist(2)
Dim info
Dim msgs
Dim elem
Dim sMsgs

'While 1
'
' This is a complex example of how to work with images as blobs
' as well as how to do lossless embedding of textual information
' into an existing JPEG image.
'
Set img = CreateObject("ImageMagickObject.MagickImage.1")
'
' Writing an image out to a VBSCRIPT array as a blob requires a
' way to tell IM what image format to use. The way this is done
' is to stuff the magick type into the array that will be used
' to store the results.
'
myarray(0)="8BIM:"
'
' This command says to take a null input image and load some 8BIM
' format text data into it as a profile. Then just send the null
' image to the output as 8BIM binary format. Pretty funky stuff I
' admit.
'
msgs = img.Convert("null:","-profile","8BIMTEXT:iptctext.txt",myarray)
'If Err.Number <> ERROR_SUCCESS Then ShowError: WScript.Quit
MsgBox "array: " & (ubound(myarray) - lbound(myarray) + 1)
'
' Now that we have the binary 8BIM data in our array as a blob we
' can stuff it into a JPEG. You could just do that with the normal
' convert -profile commmand, but if the input image is a JPEG, it
' would decompress the JPEG and recompress is, which is a quality
' hit. To avoid that we use a special feature and load both our
' text info as well as out input JPEG into another NULL image as
' profiles. The special APP1JPEG profile tells IM to embed the text
' into the JPEG losslessly. We then store the result into another
' output array marked as APP1. This essentially just send the data
' stored in the APP1 profile out untouched. The net result is a
' completely unharmed JPEG with new text data embedded in it.
'
output(0)="APP1:"
msgs = img.Convert("null:","-profile",myarray,"-profile","APP1JPEG:bill_meets_gorilla_screen.jpg",output)
'If Err.Number <> ERROR_SUCCESS Then ShowError: WScript.Quit
MsgBox "output: " & (ubound(output) - lbound(output) + 1)
'
' Last we want to save our output array into a disk file, which we
' could do with standard VBS file techniques, but we can also do it
' with convert directly - again using the APP1 profile type to both
' read the data and also to write it out to disk. This step is here
' to show how to force convert to use a specific image type for a
' blob. Normally convert would automatically detect the type and it
' would notice that this is a JPEG and decompress it. We don't want
' that in this case.
'
persist(0)="APP1:"
persist(1)=output
msgs = img.Convert("null:","-profile",persist,"APP1:bill_meets_gorilla_screen_iptc.jpg")
'
' The following statements are the sequence you would need to free
' the memory being used by this sequence. It is not really needed
' by this example, but if you are writing some kind of loop were a
' number of images are being processed, it becomes very important to
' free memory, or you will leak and eventually crash - or worse just
' degrade the resources of the rest of the system.
Set img=Nothing
Erase persist
myArray = Empty
Redim myarray(1)
output = Empty
Redim output(1)
'Wend
WScript.Quit(0)

Sub ShowError
  sMsgs = ""
  If BasicError(Err.Number) > 5000 Then
    msgs = img.Messages
    If isArray(msgs) Then
      For Each elem In msgs
        sMsgs = sMsgs & elem & vbCrLf
      Next
    End If
  End If
  WScript.Echo Err.Number & ": " & Err.Description & vbCrLf & sMsgs
End Sub

Function BasicError(e)
  BasicError = e And &HFFFF&
End Function
