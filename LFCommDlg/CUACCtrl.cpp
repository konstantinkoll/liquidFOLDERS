
// CUACCtrl.cpp: Implementierung der Klasse CUACCtrl
//

#include "stdafx.h"
#include "CUACCtrl.h"
#include "LFApplication.h"
#include "Resource.h"


// CUACCtrl
//

extern AFX_EXTENSION_MODULE LFCommDlgDLL;

CUACCtrl::CUACCtrl()
	: CWnd()
{
	m_hIcon = NULL;
	m_IconSz = m_Border = 0;
}

CUACCtrl::~CUACCtrl()
{
}

void CUACCtrl::Create(CRect &rect, CWnd* pParentWnd, UINT nID)
{
	int height = rect.Height()-6;
	m_IconSz = (height<24) ? 16 : (height<32) ? 24 : (height<48) ? 32 : 48;
	m_Border = MulDiv(7, LOWORD(GetDialogBaseUnits()), 4)-4;

	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW, NULL, NULL, NULL);

	const DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE;
	CWnd::Create(className, _T(""), dwStyle, rect, pParentWnd, nID);
}


BEGIN_MESSAGE_MAP(CUACCtrl, CWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
END_MESSAGE_MAP()

int CUACCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	m_hIcon = (HICON)LoadImage(LFCommDlgDLL.hResource, IDI_SHIELD, IMAGE_ICON, m_IconSz, m_IconSz, LR_LOADTRANSPARENT);

	return 0;
}

void CUACCtrl::OnDestroy()
{
	if (m_hIcon)
		DestroyIcon(m_hIcon);
}

BOOL CUACCtrl::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CUACCtrl::OnPaint()
{
	CPaintDC pDC(this);

	CRect rect;
	GetClientRect(rect);

	CDC dc;
	dc.CreateCompatibleDC(&pDC);
	dc.SetBkMode(TRANSPARENT);

	CBitmap buffer;
	buffer.CreateCompatibleBitmap(&pDC, rect.Width(), rect.Height());
	CBitmap* pOldBitmap = dc.SelectObject(&buffer);

	Graphics g(dc.m_hDC);
	g.SetCompositingMode(CompositingModeSourceOver);

	LinearGradientBrush brush(Point(0, 0), Point(rect.Width(), 0), Color(4, 80, 130), Color(28, 120, 133));
	g.FillRectangle(&brush, 0, 0, rect.Width(), rect.Height());

	DrawIconEx(dc.m_hDC, m_Border, (rect.Height()-m_IconSz)/2, m_hIcon, m_IconSz, m_IconSz, 0, NULL, DI_NORMAL);

	CRect rectText(rect);
	rectText.left = m_Border+m_IconSz+3;

	CFont* pOldFont = dc.SelectObject(&((LFApplication*)AfxGetApp())->m_Fonts[FALSE][TRUE]);
	dc.SetTextColor(0xFFFFFF);
	dc.DrawText(_T("liquidFOLDERS needs your permission to continue"), -1, rectText, DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_LEFT);
	dc.SelectObject(pOldFont);

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldBitmap);
}
