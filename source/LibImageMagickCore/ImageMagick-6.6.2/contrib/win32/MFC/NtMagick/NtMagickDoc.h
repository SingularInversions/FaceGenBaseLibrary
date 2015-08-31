// NtMagickDoc.h : interface of the CNtMagickDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_NTMAGICKDOC_H__8A45000A_6176_11D4_AC4F_400070168026__INCLUDED_)
#define AFX_NTMAGICKDOC_H__8A45000A_6176_11D4_AC4F_400070168026__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CNtMagickDoc : public CDocument
{
protected: // create from serialization only
	CNtMagickDoc();
	DECLARE_DYNCREATE(CNtMagickDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNtMagickDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CNtMagickDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CNtMagickDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NTMAGICKDOC_H__8A45000A_6176_11D4_AC4F_400070168026__INCLUDED_)
