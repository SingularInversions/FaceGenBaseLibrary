// NtMagickView.cpp : implementation of the CNtMagickView class

//



#include "stdafx.h"

#include "NtMagick.h"



#include "NtMagickDoc.h"

#include "NtMagickView.h"



#ifdef _DEBUG

#define new DEBUG_NEW

#undef THIS_FILE

static char THIS_FILE[] = __FILE__;

#endif



/////////////////////////////////////////////////////////////////////////////

// CNtMagickView



IMPLEMENT_DYNCREATE(CNtMagickView, CView)



BEGIN_MESSAGE_MAP(CNtMagickView, CView)

        //{{AFX_MSG_MAP(CNtMagickView)

        ON_COMMAND(ID_FILE_OPEN, OnFileOpen)

        ON_COMMAND(ID_IMAGE_FLIP_HORIZONTAL, OnImageFlipHorizontal)

        ON_UPDATE_COMMAND_UI(ID_IMAGE_FLIP_HORIZONTAL, OnUpdateImageFlipHorizontal)

        ON_COMMAND(ID_IMAGE_FLIP_VERTICAL, OnImageFlipVertical)

        ON_UPDATE_COMMAND_UI(ID_IMAGE_FLIP_VERTICAL, OnUpdateImageFlipVertical)

        ON_COMMAND(ID_IMAGE_ROTATE_180, OnImageRotate180)

        ON_UPDATE_COMMAND_UI(ID_IMAGE_ROTATE_180, OnUpdateImageRotate180)

        ON_COMMAND(ID_IMAGE_ROTATE_90, OnImageRotate90)

        ON_UPDATE_COMMAND_UI(ID_IMAGE_ROTATE_90, OnUpdateImageRotate90)

        ON_COMMAND(ID_IMAGE_ROTATE_90CCW, OnImageRotate90ccw)

        ON_UPDATE_COMMAND_UI(ID_IMAGE_ROTATE_90CCW, OnUpdateImageRotate90ccw)

        ON_COMMAND(ID_FILE_CLEAR, OnFileClear)

        ON_UPDATE_COMMAND_UI(ID_FILE_CLEAR, OnUpdateFileClear)

        //}}AFX_MSG_MAP

END_MESSAGE_MAP()



/////////////////////////////////////////////////////////////////////////////

// CNtMagickView construction/destruction



CNtMagickView::CNtMagickView()

{

}



CNtMagickView::~CNtMagickView()

{

}



BOOL CNtMagickView::PreCreateWindow(CREATESTRUCT& cs)

{

        // TODO: Modify the Window class or styles here by modifying

        //  the CREATESTRUCT cs



        return CView::PreCreateWindow(cs);

}



/////////////////////////////////////////////////////////////////////////////

// CNtMagickView drawing



void CNtMagickView::OnDraw(CDC* pDC)

{

  CNtMagickDoc* pDoc = GetDocument();

  ASSERT_VALID(pDoc);

  CWnd* pMainWnd = AfxGetMainWnd();

  pMainWnd->SetWindowText("ImageMagick Win32 Image Viewer");

  DoDisplayImage();

}



/////////////////////////////////////////////////////////////////////////////

// CNtMagickView diagnostics



#ifdef _DEBUG

void CNtMagickView::AssertValid() const

{

        CView::AssertValid();

}



void CNtMagickView::Dump(CDumpContext& dc) const

{

        CView::Dump(dc);

}



CNtMagickDoc* CNtMagickView::GetDocument() // non-debug version is inline

{

        ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CNtMagickDoc)));

        return (CNtMagickDoc*)m_pDocument;

}

#endif //_DEBUG



/////////////////////////////////////////////////////////////////////////////

// CNtMagickView message handlers



//-----------------------------------------------------------------------

// DoFileOpen()

// Select the image to be displayed

//-----------------------------------------------------------------------



void CNtMagickView::OnFileOpen()

{

  CString   szFolder;

  CString   szFilter;

  szFilter += NTMAGICK_JPEG;

  szFilter += NTMAGICK_BMP;

  szFilter += NTMAGICK_GIF;

  szFilter += NTMAGICK_TIF;

  szFilter += NTMAGICK_ICON;

  szFilter += NTMAGICK_ALL;

  szFilter += "|";



  szFolder = AfxGetApp()->GetProfileString("Image","Path","");



  CFileDialog fileDlg(TRUE,NULL,NULL,NULL,szFilter,NULL);

  fileDlg.m_ofn.Flags|=OFN_FILEMUSTEXIST | OFN_EXPLORER | OFN_READONLY;

  fileDlg.m_ofn.lpstrTitle="Choose the image to view";

  fileDlg.m_ofn.lpstrInitialDir= szFolder;



  if (fileDlg.DoModal()== IDOK)

  {

    m_szFile = fileDlg.GetPathName();

    if (DoReadImage())

    {

      DoDisplayImage();



      CFile     fileImage;



      fileImage.SetFilePath(m_szFile);

      szFolder = m_szFile;

      szFolder.TrimRight(fileImage.GetFileName());

      szFolder.TrimRight("\\");

      AfxGetApp()->WriteProfileString("Image","Path", szFolder);

    }

  }

}



//-----------------------------------------------------------------------

// DoReadImage()

// Read image.

//-----------------------------------------------------------------------



BOOL CNtMagickView::DoReadImage()

{

  // Release image object memory

  m_Image.isValid(FALSE);



  // Read the image and handle any exceptions

  try

  {

    m_Image.read(m_szFile.GetBuffer(MAX_PATH+1));

  }



  catch(Exception e)

  {

    m_Image.isValid(FALSE);

    DoDisplayError("Read",e.what());

    return FALSE;

  }



  catch(exception e)

  {

    m_Image.isValid(FALSE);

    DoDisplayError("Read",e.what());

    return FALSE;

  }



  return TRUE;

}



//-----------------------------------------------------------------------

// DoDisplayError()

// Display the cause of any unhandle exceptions.

//-----------------------------------------------------------------------



void CNtMagickView::DoDisplayError(CString szFunction, CString szCause)

{

  CString szMsg;

  szMsg.Format("NtMagickView function [%s] encountered an error.\n%s",szFunction,szCause);

  AfxMessageBox(szMsg,MB_OK);

}



//-----------------------------------------------------------------------

// DoDisplayImage()

// Display the image in the client window.  Scale the image to fit.

//-----------------------------------------------------------------------



void CNtMagickView::DoDisplayImage()

{

  CDC *pDC = GetDC();

  if (pDC != NULL && m_Image.isValid() ) 

    {

      CRect rectClient;

      GetClientRect(rectClient);



      // Clear the background

      pDC->FillSolidRect(rectClient,pDC->GetBkColor());



      // Set up the Windows bitmap header

      BITMAPINFOHEADER bmi;

      bmi.biSize = sizeof(BITMAPINFOHEADER);    // Size of structure

      bmi.biWidth = m_Image.columns();          // Bitmaps width in pixels

      bmi.biHeight = (-1)*m_Image.rows();       // Bitmaps height n pixels

      bmi.biPlanes = 1;                         // Number of planes in the image

      bmi.biBitCount = 32;                      // The number of bits per pixel

      bmi.biCompression = BI_RGB;               // The type of compression used

      bmi.biSizeImage = 0;                      // The size of the image in bytes

      bmi.biXPelsPerMeter = 0;                  // Horizontal resolution

      bmi.biYPelsPerMeter = 0;                  // Veritical resolution

      bmi.biClrUsed = 0;                        // Number of colors actually used

      bmi.biClrImportant = 0;                   // Colors most important



      // Extract the pixels from Magick++ image object and convert to a DIB section

      PixelPacket *pPixels = m_Image.getPixels(0,0,m_Image.columns(),m_Image.rows());



      RGBQUAD *prgbaDIB = 0;

      HBITMAP hBitmap = CreateDIBSection

        (

         pDC->m_hDC,            // handle to device context

         (BITMAPINFO *)&bmi,    // pointer to structure containing bitmap size, format, and color data

         DIB_RGB_COLORS,        // color data type indicator: RGB values or palette indices

         (void**)&prgbaDIB,     // pointer to variable to receive a pointer to the bitmap's bit values

         NULL,                  // optional handle to a file mapping object

         0                      // offset to the bitmap bit values within the file mapping object

         );



      if ( !hBitmap )

        return;



      unsigned long nPixels = m_Image.columns() * m_Image.rows();

      RGBQUAD *pDestPixel = prgbaDIB;

#if QuantumDepth == 8

      // Form of PixelPacket is identical to RGBQUAD when QuantumDepth==8

      memcpy((void*)pDestPixel,(const void*)pPixels,sizeof(PixelPacket)*nPixels);

#elif QuantumDepth == 16

      // Transfer pixels, scaling to Quantum

      for( unsigned long nPixelCount = nPixels; nPixelCount ; nPixelCount-- )

        {

          pDestPixel->rgbRed = (pPixels->red)/257;

          pDestPixel->rgbGreen = (pPixels->green)/257;

          pDestPixel->rgbBlue = (pPixels->blue)/257;

          pDestPixel->rgbReserved = 0;

          ++pDestPixel;

          ++pPixels;

        }

#endif



      // Now copy the bitmap to device.

        HDC     hMemDC = CreateCompatibleDC( pDC->m_hDC );

        SelectObject( hMemDC, hBitmap );

        BitBlt( pDC->m_hDC, 0, 0, m_Image.columns(), m_Image.rows(), hMemDC, 0, 0, SRCCOPY );

        DeleteObject( hMemDC );

    }

}



//-------------------------------------------------------------------

// Scale()

// Proportionally scale the first rectangle so that it occupies the

// maximum area of the target rectangle

//-------------------------------------------------------------------



CSize CNtMagickView::Scale(CSize sizeSrc, CSize sizeTgt)

{



  CSize size;



  // Obtain the scaling factor

  float fScale = ScaleFactor(FALSE,sizeSrc,sizeTgt);



  // Calculate the size of the sized rectangle

  size.cx = ((float) sizeSrc.cx * fScale) + 0.5;

  size.cy = ((float) sizeSrc.cy * fScale) + 0.5;



  // Ensure roundings errors don't make scaled rectangle too large

  if (size.cx > sizeTgt.cx)

  {

    size.cx = sizeTgt.cx;

  }



  if (size.cy > sizeTgt.cy)

  {

    size.cy = sizeTgt.cy;

  }



  // Return dimensions of the rectangle

  return size;

}



//-------------------------------------------------------------------

// ScaleFactor()

// Return the ratio required to proprtionally scale a rectangle

//-------------------------------------------------------------------



float CNtMagickView::ScaleFactor(BOOL bAllowZoom, CSize sizeSrc, CSize sizeTgt)

{

  float fScale = 1;



  // If the image is smaller than the target don't scale zoom



  if ((sizeSrc.cx < sizeTgt.cx)&& (sizeSrc.cy < sizeTgt.cy))

  {

    if (bAllowZoom)

    {

      float fScaleW = (float) sizeSrc.cx / (float) sizeTgt.cx;

      float fScaleH = (float)  sizeSrc.cy / (float) sizeTgt.cy;



      if (fScaleH < fScaleW)

      {

        fScale = fScaleH;

      }

      else

      {

        fScale = fScaleW;

      }

    }

    else

    {

      fScale = 1;

    }

  }



  // If the image is equal to the target don't scale



  if ((sizeSrc.cx == sizeTgt.cx) && (sizeSrc.cy == sizeTgt.cy))

  {

    fScale = 1;

  }



  // If the current image is wider than the target image,

  // reduce the size of the image whilst preserving the aspect

  // ratio of the original image.



  if ((sizeSrc.cx > sizeTgt.cx) && (sizeSrc.cy <= sizeTgt.cy))

  {

    fScale = (float) sizeTgt.cx / (float) sizeSrc.cx;

  }



  // If the current image is taller than the target image,

  // reduce the size of the image whilst preserving the aspect

  // ratio of the original image.



  if (((int)sizeSrc.cx <= sizeTgt.cx) && ((int)sizeSrc.cy > sizeTgt.cy))

    {

      fScale = (float) sizeTgt.cy / (float) sizeSrc.cy;

    }



  // If the Jpeg image is wider and taller than the client rectangle,

  // reduce the size of the image whilst preserving the aspect

  // ratio of the original image.



  if (((int)sizeSrc.cx > sizeTgt.cx) && ((int)sizeSrc.cy > sizeTgt.cy))

  {

    float fScaleW = (float) sizeTgt.cx / (float) sizeSrc.cx;

    float fScaleH = (float) sizeTgt.cy / (float) sizeSrc.cy;



    if (fScaleH < fScaleW)

      {

        fScale = fScaleH;

      }

    else

      {

        fScale = fScaleW;

      }

  }



  return fScale;

}



//-----------------------------------------------------------------------

//  UpdateUI()

//  User interface controls are only active when an image has been read.

//-----------------------------------------------------------------------



void CNtMagickView::UpdateUI(CCmdUI *pCmdUI)

{

  if (m_Image.isValid())

  {

    pCmdUI->Enable(TRUE);

  }

  else

  {

    pCmdUI->Enable(FALSE);

  }

}



//-----------------------------------------------------------------------

//  OnImageFlipHorizontal()

//  Flip horizontally

//-----------------------------------------------------------------------



void CNtMagickView::OnImageFlipHorizontal()

{

  try

  {

    m_Image.flop();

  }



  catch(Exception e)

  {

    DoDisplayError("Flop",e.what());

    return;

  }



  catch(exception e)

  {

    DoDisplayError("Flop",e.what());

    return;

  }



  DoDisplayImage();

}



void CNtMagickView::OnUpdateImageFlipHorizontal(CCmdUI* pCmdUI)

{

  UpdateUI(pCmdUI);

}



//-----------------------------------------------------------------------

//  OnImageFlipVertical()

//  Flip vertically

//-----------------------------------------------------------------------



void CNtMagickView::OnImageFlipVertical()

{

  try

  {

    m_Image.flip();

  }



  catch(Exception e)

  {

    DoDisplayError("Flip",e.what());

    return;

  }



  catch(exception e)

  {

    DoDisplayError("Flip",e.what());

    return;

  }



  DoDisplayImage();

}



void CNtMagickView::OnUpdateImageFlipVertical(CCmdUI* pCmdUI)

{

  UpdateUI(pCmdUI);

}



//-----------------------------------------------------------------------

//  OnImageRotate180()

//  Rotate image 180 degrees

//-----------------------------------------------------------------------



void CNtMagickView::OnImageRotate180()

{

  try

  {

    m_Image.rotate(180);

  }



  catch(Exception e)

  {

    DoDisplayError("Rotate 180",e.what());

    return;

  }



  catch(exception e)

  {

    DoDisplayError("Rotate 180",e.what());

    return;

  }



  DoDisplayImage();

}



void CNtMagickView::OnUpdateImageRotate180(CCmdUI* pCmdUI)

{

  UpdateUI(pCmdUI);

}



//-----------------------------------------------------------------------

//  OnImageRotate90()

//  Rotate image 90 degrees clockwise

//-----------------------------------------------------------------------



void CNtMagickView::OnImageRotate90()

{

  try

  {

    m_Image.rotate(90);

  }



  catch(Exception e)

  {

    DoDisplayError("Rotate 90",e.what());

    return;

  }



  catch(exception e)

  {

    DoDisplayError("Rotate 90",e.what());

    return;

  }



  DoDisplayImage();

}



void CNtMagickView::OnUpdateImageRotate90(CCmdUI* pCmdUI)

{

  UpdateUI(pCmdUI);

}



//-----------------------------------------------------------------------

//  OnImageRotate90ccw()

//  Rotate image 90 degrees clockwise

//-----------------------------------------------------------------------



void CNtMagickView::OnImageRotate90ccw()

{

  try

  {

    m_Image.rotate(-90);

  }



  catch(Exception e)

  {

    DoDisplayError("Rotate -90",e.what());

    return;

  }



  catch(exception e)

  {

    DoDisplayError("Rotate -90",e.what());

    return;

  }



  DoDisplayImage();

}



void CNtMagickView::OnUpdateImageRotate90ccw(CCmdUI* pCmdUI)

{

  UpdateUI(pCmdUI);

}



//-----------------------------------------------------------------------

//  OnFileClear()

//  Clear image

//-----------------------------------------------------------------------



void CNtMagickView::OnFileClear()

{

  CRect rectClient;

  CDC * pDC;



  // Remove image and clear client area

  m_Image.isValid(FALSE);

  pDC = GetDC();

  GetClientRect(rectClient);

  if (pDC != NULL)

  {

    pDC->FillSolidRect(rectClient,pDC->GetBkColor());

  }

}



void CNtMagickView::OnUpdateFileClear(CCmdUI* pCmdUI)

{

  UpdateUI(pCmdUI);

}

