
// CIconCtrl.cpp: Implementierung der Klasse CIconCtrl
//

#include "stdafx.h"
#include "LFCommDlg.h"


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
	wndcls.hCursor = LFGetApp()->LoadStandardCursor(IDC_ARROW);
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

void CIconCtrl::SetIcon(HICON hIcon, INT cx, INT cy, BOOL Center)
{
	m_Icon = hIcon;
	m_IconSizeX = cx;
	m_IconSizeY = cy;
	m_Center = Center;

	Invalidate();
}

void CIconCtrl::SetCoreIcon(UINT nID, BOOL Center)
{
	CRect rect;
	GetClientRect(rect);
	INT sz = min(rect.Width(), rect.Height());
	INT IconSize = (sz>=128) ? 128 : (sz>=96) ? 96 : (sz>=48) ? 48 : (sz>=32) ? 32 : (sz>=24) ? 24 : 16;

	SetIcon((HICON)LoadImage(GetModuleHandle(_T("LFCORE.DLL")), MAKEINTRESOURCE(nID), IMAGE_ICON, IconSize, IconSize, LR_SHARED), IconSize, IconSize, Center);
}

void CIconCtrl::SetSmallIcon(HINSTANCE hInst, UINT nID, BOOL Center)
{
	CRect rect;
	GetClientRect(rect);
	INT sz = min(rect.Width(), rect.Height());
	INT IconSize = (sz>=32) ? 32 : 16;

	SetIcon((HICON)LoadImage(hInst, MAKEINTRESOURCE(nID), IMAGE_ICON, IconSize, IconSize, LR_SHARED), IconSize, IconSize, Center);
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

	CBitmap MemBitmap;
	MemBitmap.CreateCompatibleBitmap(&pDC, rect.Width(), rect.Height());
	CBitmap* pOldBitmap = dc.SelectObject(&MemBitmap);

	HBRUSH brush = (HBRUSH)GetParent()->SendMessage(WM_CTLCOLORBTN, (WPARAM)dc.m_hDC, (LPARAM)m_hWnd);
	if (brush)
		FillRect(dc, rect, brush);

	if (m_Icon)
		DrawIconEx(dc, 0, m_Center ? (rect.Height()-m_IconSizeY)/2 : 0, m_Icon, m_IconSizeX, m_IconSizeY, 0, NULL, DI_NORMAL);

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldBitmap);
}
