// NtMagickDoc.cpp : implementation of the CNtMagickDoc class
//

#include "stdafx.h"
#include "NtMagick.h"

#include "NtMagickDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNtMagickDoc

IMPLEMENT_DYNCREATE(CNtMagickDoc, CDocument)

BEGIN_MESSAGE_MAP(CNtMagickDoc, CDocument)
	//{{AFX_MSG_MAP(CNtMagickDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNtMagickDoc construction/destruction

CNtMagickDoc::CNtMagickDoc()
{
	// TODO: add one-time construction code here

}

CNtMagickDoc::~CNtMagickDoc()
{
}

BOOL CNtMagickDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CNtMagickDoc serialization

void CNtMagickDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CNtMagickDoc diagnostics

#ifdef _DEBUG
void CNtMagickDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CNtMagickDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CNtMagickDoc commands
