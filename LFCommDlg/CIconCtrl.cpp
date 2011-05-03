
// CIconCtrl.cpp: Implementierung der Klasse CIconCtrl
//

#include "stdafx.h"
#include "CIconCtrl.h"


// CIconCtrl
//

extern AFX_EXTENSION_MODULE LFCommDlgDLL;

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
	if (!(::GetClassInfo(LFCommDlgDLL.hModule, L"CIconCtrl", &wndcls)))
	{
		wndcls.hInstance = LFCommDlgDLL.hModule;

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

void CIconCtrl::SetIcon(HICON _icon, INT _cx, INT _cy)
{
	m_Icon = _icon;
	m_IconSizeX = _cx;
	m_IconSizeY = _cy;

	Invalidate();
}

void CIconCtrl::SetCoreIcon(UINT nID)
{
	CRect rect;
	GetClientRect(rect);
	INT sz = min(rect.Width(), rect.Height());

	HICON hIcon = NULL;
	m_IconSizeX = m_IconSizeY = (sz>=64) ? 64 : (sz>=48) ? 48 : (sz>=32) ? 32 : (sz>=24) ? 24 : 16;

	HINSTANCE hModIcons = LoadLibrary(_T("LFCORE.DLL"));
	if (hModIcons)
	{
		hIcon = (HICON)LoadImage(hModIcons, MAKEINTRESOURCE(nID), IMAGE_ICON, m_IconSizeX, m_IconSizeY, LR_DEFAULTCOLOR);

		FreeLibrary(hModIcons);
	}

	SetIcon(hIcon, m_IconSizeX, m_IconSizeY);
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

	dc.FillSolidRect(rect, GetSysColor(COLOR_3DFACE));

	if (m_Icon)
		DrawIconEx(dc, 0, 0, m_Icon, m_IconSizeX, m_IconSizeY, 0, NULL, DI_NORMAL);

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldBitmap);
}
