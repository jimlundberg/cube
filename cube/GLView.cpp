// GLView.cpp : OpenGL implementation file
//
// John Davin, Terrence Wong, Jim Lundberg
// Copyright (c) 2002-2004 JRL Innovations  All Rights Reserved
// 3D5 - The FIVE level 3D tic-tac-toe game
//

#include "StdAfx.h"
#include "3D5.h"
#include "GLView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGLView

CGLView::CGLView(CWnd *pclWnd, GLBox *glboxptr)
{
	m_pclWnd = pclWnd;
    m_hWnd   = pclWnd->m_hWnd;
    m_hDC    = ::GetDC(m_hWnd);
	m_hGLContext = NULL;
	m_GLPixelIndex = 0;
	glbox = glboxptr;

	OnCreate();
}

CGLView::~CGLView()
{
	if (wglGetCurrentContext()!=NULL) 
	{
		// make the rendering context not current
		wglMakeCurrent(NULL, NULL);
	}
	
	if (m_hGLContext!=NULL)
	{
		wglDeleteContext(m_hGLContext);
		m_hGLContext = NULL;
	}
}


BEGIN_MESSAGE_MAP(CGLView, CWnd)
	//{{AFX_MSG_MAP(CGLView)
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEWHEEL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CGLView message handlers


GLvoid CGLView::ReSizeGLScene( GLsizei width, GLsizei height )
{
	glbox->resizeGL( width, height );

	glViewport( -10, -65, GLAREA_WIDTH, GLAREA_HEIGHT );
    // gl area must stay a fixed size so that the mouse mapping will work
    // if you want to change this size, then a new coordinate map must be built!

	glDrawBuffer( GL_BACK );
	TRACE( "[OnSize]: Resized GL window.\n" );
}

int CGLView::InitGL(GLvoid)
{
	TRACE( "[GLView::InitGL]\n" );

	glbox->initializeGL();

	return 0;
}

int CGLView::DrawGLScene(GLvoid)
{
	glbox->paintGL(NULL);
	SwapBuffers(m_hDC);
	return 0;
}

bool CGLView::CreateViewGLContext( HDC hDC )
{
   m_hGLContext = wglCreateContext( hDC );
   if (m_hGLContext == NULL)
   {
      return FALSE;
   }

   if (wglMakeCurrent( hDC, m_hGLContext ) == FALSE)
   {
      return FALSE;
   }

   return TRUE;
}

BOOL CGLView::OnEraseBkgnd( CDC* pDC )
{
	TRACE( "OnEraseBkgnd called\n" );
	return TRUE;	
//  return CWnd::OnEraseBkgnd( pDC );
}


int CGLView::OnCreate() 
{
	TRACE( "[CGLView::OnCreate]" );
    m_hDC = ::GetDC( this->m_hWnd );

    if(!SetWindowPixelFormat( m_hDC ))
    {
		::MessageBox( ::GetFocus(), "SetPixelformat Failed!", "Error", MB_OK );
		return -1;
    }

    if (CreateViewGLContext( m_hDC ) == FALSE)
      return -1;

	InitGL();	

	TRACE( "[CGLView::OnCreate]: GL initialized\n" );

	return 0;	
}

bool CGLView::SetWindowPixelFormat( HDC hdc )
{
  PIXELFORMATDESCRIPTOR pixelDesc;
   pixelDesc.nSize = sizeof( PIXELFORMATDESCRIPTOR );
   pixelDesc.nVersion = 1;
   pixelDesc.dwFlags = PFD_DRAW_TO_WINDOW | 
                       PFD_SUPPORT_OPENGL | 
                       PFD_DOUBLEBUFFER | 
                       PFD_STEREO_DONTCARE;
   pixelDesc.iPixelType = PFD_TYPE_RGBA;
   pixelDesc.cColorBits = 32;
   pixelDesc.cRedBits = 8; 
   pixelDesc.cRedShift = 16;
   pixelDesc.cGreenBits = 8;
   pixelDesc.cGreenShift = 8;
   pixelDesc.cBlueBits = 8;
   pixelDesc.cBlueShift = 0;
   pixelDesc.cAlphaBits = 0;
   pixelDesc.cAlphaShift = 0;
   pixelDesc.cAccumBits = 64;    
   pixelDesc.cAccumRedBits = 16;
   pixelDesc.cAccumGreenBits = 16;
   pixelDesc.cAccumBlueBits = 16;
   pixelDesc.cAccumAlphaBits = 0;
   pixelDesc.cDepthBits = 32;
   pixelDesc.cStencilBits = 8;
   pixelDesc.cAuxBuffers = 0;
   pixelDesc.iLayerType = PFD_MAIN_PLANE;
   pixelDesc.bReserved = 0;
   pixelDesc.dwLayerMask = 0;
   pixelDesc.dwVisibleMask = 0;
   pixelDesc.dwDamageMask = 0;

   m_GLPixelIndex = ChoosePixelFormat(hdc, &pixelDesc);
   if (m_GLPixelIndex == 0) // Let's choose a default index.
   {
      m_GLPixelIndex = 1;    
      if (DescribePixelFormat(hdc, m_GLPixelIndex,
          sizeof(PIXELFORMATDESCRIPTOR), &pixelDesc) == 0)
      {
         return FALSE;
      }
   }

   if (SetPixelFormat(hdc, m_GLPixelIndex, &pixelDesc) == FALSE)
   {
      return FALSE;
   }

   return TRUE;
}

BOOL CGLView::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.style |= WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	return CWnd::PreCreateWindow(cs);
}

BOOL CGLView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    printf("[CGLView::OnMouseWheel]\n");
    return CWnd::OnMouseWheel(nFlags, zDelta, pt);
}
