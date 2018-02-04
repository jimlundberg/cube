// GLview.h : OpenGL View class
//
// John Davin, Terrence Wong, Jim Lundberg
// Copyright (c) 2002-2004 JRL Innovations  All Rights Reserved
// 3D5 - The FIVE level 3D tic-tac-toe game
//

#ifndef GLVIEW_H
#define GLVIEW_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// GLView.h : header file

#include "glbox.h"

/////////////////////////////////////////////////////////////////////////////
// CGLView window

class CGLView : public CWnd
{
// Construction
public:
	CGLView(CWnd *pclWnd, GLBox *glboxptr);

// Attributes
public:
	HDC     m_hDC;		// GDI Device Context 
    HGLRC	m_hglRC;		// Rendering Context

    CWnd *  m_pclWnd;
    HWND    m_hWnd;

// Operations
public:
    GLvoid ReSizeGLScene(GLsizei width, GLsizei height);
    int InitGL(GLvoid);
    int DrawGLScene(GLvoid);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGLView)
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CGLView();
	int OnCreate();

	// Generated message map functions
protected:
	HGLRC   m_hGLContext;
	int     m_GLPixelIndex;
    bool    CreateViewGLContext(HDC hDC);
    bool    SetWindowPixelFormat(HDC hdc);
	GLBox * glbox;

	//{{AFX_MSG(CGLView)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif
