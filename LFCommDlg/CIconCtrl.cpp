
// CIconCtrl.cpp: Implementierung der Klasse CIconCtrl
//

#include "stdafx.h"
#include "CIconCtrl.h"


// CIconCtrl
//

CIconCtrl::CIconCtrl()
	: CWnd()
{
	WNDCLASS wndcls;
	ZeroMemory(&wndcls, sizeof(wndcls));
	wndcls.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	wndcls.lpfnWndProc = ::DefWindowProc;
	wndcls.cbClsExtra = wndcls.cbWndExtra = 0;
	wndcls.hIcon = NULL;
	wndcls.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndcls.hbrBackground = NULL;
	wndcls.lpszMenuName = NULL;
	wndcls.lpszClassName = L"CIconCtrl";

	if (!(::GetClassInfo(AfxGetInstanceHandle(), L"CIconCtrl", &wndcls)))
	{
		wndcls.hInstance = AfxGetInstanceHandle();

		if (!AfxRegisterClass(&wndcls))
			AfxThrowResourceException();
	}

	m_Icon = NULL;
	m_IconSizeX = m_IconSizeY = 0;
}

CIconCtrl::~CIconCtrl()
{
	if (m_Icon)
		DestroyIcon(m_Icon);
}

void CIconCtrl::SetIcon(HICON hIcon, INT cx, INT cy)
{
	m_Icon = hIcon;
	m_IconSizeX = cx;
	m_IconSizeY = cy;

	Invalidate();
}

void CIconCtrl::SetCoreIcon(UINT nID)
{
	CRect rect;
	GetClientRect(rect);
	INT sz = min(rect.Width(), rect.Height());
	INT IconSize = (sz>=128) ? 128 : (sz>=48) ? 48 : (sz>=32) ? 32 : (sz>=24) ? 24 : 16;

	SetIcon((HICON)LoadImage(GetModuleHandle(_T("LFCORE.DLL")), MAKEINTRESOURCE(nID), IMAGE_ICON, IconSize, IconSize, LR_DEFAULTCOLOR), IconSize, IconSize);
}

void CIconCtrl::SetSmallIcon(HINSTANCE hInst, UINT nID)
{
	CRect rect;
	GetClientRect(rect);
	INT sz = min(rect.Width(), rect.Height());
	INT IconSize = (sz>=32) ? 32 : 16;

	SetIcon((HICON)LoadImage(hInst, MAKEINTRESOURCE(nID), IMAGE_ICON, IconSize, IconSize, LR_DEFAULTCOLOR), IconSize, IconSize);
}


BEGIN_MESSAGE_MAP(CIconCtrl, CWnd)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
END_MESSAGE_MAP()

BOOL CIconCtrl::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CIconCtrl::OnPaint()
{
	CPaintDC pDC(this);

	CRect rect;
	GetClientRect(rect);

	CDC dc;
	dc.CreateCompatibleDC(&pDC);

	CBitmap buffer;
	buffer.CreateCompatibleBitmap(&pDC, rect.Width(), rect.Height());
	CBitmap* pOldBitmap = dc.SelectObject(&buffer);

	HBRUSH brush = (HBRUSH)GetParent()->SendMessage(WM_CTLCOLORBTN, (WPARAM)dc.m_hDC, (LPARAM)m_hWnd);
	if (brush)
		FillRect(dc, rect, brush);

	if (m_Icon)
		DrawIconEx(dc, 0, 0, m_Icon, m_IconSizeX, m_IconSizeY, 0, NULL, DI_NORMAL);

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldBitmap);
}
