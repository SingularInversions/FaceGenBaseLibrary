// NtMagickView.h : interface of the CNtMagickView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_NTMAGICKVIEW_H__8A45000C_6176_11D4_AC4F_400070168026__INCLUDED_)
#define AFX_NTMAGICKVIEW_H__8A45000C_6176_11D4_AC4F_400070168026__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define NTMAGICK_DEFEXT                "*.JPG;*.JPEG"
#define NTMAGICK_ALL                   "All Files (*.*)|*.*|"
#define NTMAGICK_BMP                   "Bitmaps (*.BMP;*.RLE)|*.BMP;*.RLE|"
#define NTMAGICK_GIF                   "GIF (*.GIF)|*.GIF|"
#define NTMAGICK_TIF                   "TIF (*.TIF;*.TIFF)|*.TIF;*.TIFF|"
#define NTMAGICK_JPEG                  "JPEG (*.JPG;*.JPEG)|*.JPG;*.JPEG|"
#define NTMAGICK_ICON                  "Icons (*.ICO)|*.ICO|"

class CNtMagickView : public CView
{
protected: // create from serialization only
        CNtMagickView();
        DECLARE_DYNCREATE(CNtMagickView)

// Attributes
public:
        CString       m_szFile;
        Image         m_Image;
        CNtMagickDoc* GetDocument();
        void          DoDisplayError(CString szFunction, CString szCause);
        void          DoDisplayImage();
        BOOL          DoReadImage();
        CSize         Scale(CSize sizeSrc, CSize sizeTgt);
        float         ScaleFactor(BOOL bAllowZoom, CSize sizeSrc, CSize sizeTgt);
        void          UpdateUI(CCmdUI* pCmdUI);

// Operations
public:

// Overrides
        // ClassWizard generated virtual function overrides
        //{{AFX_VIRTUAL(CNtMagickView)
        public:
        virtual void OnDraw(CDC* pDC);  // overridden to draw this view
        virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
        protected:
        //}}AFX_VIRTUAL

// Implementation
public:
        virtual ~CNtMagickView();
#ifdef _DEBUG
        virtual void AssertValid() const;
        virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
        //{{AFX_MSG(CNtMagickView)
        afx_msg void OnFileOpen();
        afx_msg void OnImageFlipHorizontal();
        afx_msg void OnUpdateImageFlipHorizontal(CCmdUI* pCmdUI);
        afx_msg void OnImageFlipVertical();
        afx_msg void OnUpdateImageFlipVertical(CCmdUI* pCmdUI);
        afx_msg void OnImageRotate180();
        afx_msg void OnUpdateImageRotate180(CCmdUI* pCmdUI);
        afx_msg void OnImageRotate90();
        afx_msg void OnUpdateImageRotate90(CCmdUI* pCmdUI);
        afx_msg void OnImageRotate90ccw();
        afx_msg void OnUpdateImageRotate90ccw(CCmdUI* pCmdUI);
        afx_msg void OnFileClear();
        afx_msg void OnUpdateFileClear(CCmdUI* pCmdUI);
        //}}AFX_MSG
        DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in NtMagickView.cpp
inline CNtMagickDoc* CNtMagickView::GetDocument()
   { return (CNtMagickDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NTMAGICKVIEW_H__8A45000C_6176_11D4_AC4F_400070168026__INCLUDED_)
