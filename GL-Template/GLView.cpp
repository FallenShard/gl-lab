// GLView.cpp : implementation of the CGLView class
//

#include "stdafx.h"
#include "GL.h"

#include "GLDoc.h"
#include "GLView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGLView

IMPLEMENT_DYNCREATE(CGLView, CView)

BEGIN_MESSAGE_MAP(CGLView, CView)
    //{{AFX_MSG_MAP(CGLView)
    ON_WM_CREATE()
    ON_WM_DESTROY()
    ON_WM_ERASEBKGND()
    ON_WM_SIZE()
    //}}AFX_MSG_MAP
    // Standard printing commands
    ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
    ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
    ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
    ON_WM_KEYDOWN()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGLView construction/destruction

CGLView::CGLView()
{
    // TODO: add construction code here

}

CGLView::~CGLView()
{
}

BOOL CGLView::PreCreateWindow(CREATESTRUCT& cs)
{
    // TODO: Modify the Window class or styles here by modifying
    //  the CREATESTRUCT cs

    return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CGLView drawing

void CGLView::OnDraw(CDC* pDC)
{
    CGLDoc* pDoc = GetDocument();
    ASSERT_VALID(pDoc);
    // TODO: add draw code for native data here
    m_glRenderer.DrawScene(pDC);
}

/////////////////////////////////////////////////////////////////////////////
// CGLView printing

BOOL CGLView::OnPreparePrinting(CPrintInfo* pInfo)
{
    // default preparation
    return DoPreparePrinting(pInfo);
}

void CGLView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
    // TODO: add extra initialization before printing
}

void CGLView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
    // TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// CGLView diagnostics

#ifdef _DEBUG
void CGLView::AssertValid() const
{
    CView::AssertValid();
}

void CGLView::Dump(CDumpContext& dc) const
{
    CView::Dump(dc);
}

CGLDoc* CGLView::GetDocument() // non-debug version is inline
{
    ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CGLDoc)));
    return (CGLDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CGLView message handlers

int CGLView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
    if (CView::OnCreate(lpCreateStruct) == -1)
        return -1;
    
    CDC* pDC = GetDC();
    m_glRenderer.CreateGLContext(pDC);
    ReleaseDC(pDC);
    
    return 0;
}

void CGLView::OnDestroy() 
{
    CView::OnDestroy();
    
    CDC* pDC = GetDC();
    m_glRenderer.DestroyScene(pDC);
    ReleaseDC(pDC);	
}

BOOL CGLView::OnEraseBkgnd(CDC* pDC) 
{
    return TRUE;
    
    //return CView::OnEraseBkgnd(pDC);
}

void CGLView::OnSize(UINT nType, int cx, int cy) 
{
    CView::OnSize(nType, cx, cy);
    
    CDC* pDC = GetDC();
    m_glRenderer.Reshape(pDC, cx, cy);
    ReleaseDC(pDC);
}

void CGLView::OnInitialUpdate() 
{
    CView::OnInitialUpdate();
    
    CDC* pDC = GetDC();
    m_glRenderer.PrepareScene(pDC);
    ReleaseDC(pDC);
    
}


void CGLView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    // TODO: Add your message handler code here and/or call default
    if (m_glRenderer.onKeyDown(nChar) == true)
        Invalidate();

    CView::OnKeyDown(nChar, nRepCnt, nFlags);
}
