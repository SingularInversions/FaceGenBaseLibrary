// NtMagick.h : main header file for the NTMAGICK application
//

#if !defined(AFX_NTMAGICK_H__8A450004_6176_11D4_AC4F_400070168026__INCLUDED_)
#define AFX_NTMAGICK_H__8A450004_6176_11D4_AC4F_400070168026__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
        #error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols
#include <Magick++.h>
#include <string>
using namespace std;
using namespace Magick;

/////////////////////////////////////////////////////////////////////////////
// CNtMagickApp:
// See NtMagick.cpp for the implementation of this class
//

class CNtMagickApp : public CWinApp
{
public:
        CNtMagickApp();

// Overrides
        // ClassWizard generated virtual function overrides
        //{{AFX_VIRTUAL(CNtMagickApp)
        public:
        virtual BOOL InitInstance();
        //}}AFX_VIRTUAL

// Implementation
        //{{AFX_MSG(CNtMagickApp)
        afx_msg void OnAppAbout();
                // NOTE - the ClassWizard will add and remove member functions here.
                //    DO NOT EDIT what you see in these blocks of generated code !
        //}}AFX_MSG
        DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NTMAGICK_H__8A450004_6176_11D4_AC4F_400070168026__INCLUDED_)
