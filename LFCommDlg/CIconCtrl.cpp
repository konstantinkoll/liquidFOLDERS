
// CIconCtrl.cpp: Implementierung der Klasse CIconCtrl
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CIconCtrl
//

CIconCtrl::CIconCtrl()
	: CFrontstageWnd()
{
	WNDCLASS wndcls;
	ZeroMemory(&wndcls, sizeof(wndcls));
	wndcls.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	wndcls.lpfnWndProc = ::DefWindowProc;
	wndcls.hCursor = LFGetApp()->LoadStandardCursor(IDC_ARROW);
	wndcls.lpszClassName = L"CIconCtrl";

	if (!(::GetClassInfo(AfxGetInstanceHandle(), L"CIconCtrl", &wndcls)))
	{
		wndcls.hInstance = AfxGetInstanceHandle();

		if (!AfxRegisterClass(&wndcls))
			AfxThrowResourceException();
	}

	m_IconSize = 0;
	m_hIcon = NULL;
}

void CIconCtrl::SetIcon(HICON hIcon, INT IconSize, BOOL Center)
{
	m_IconSize = IconSize;
	m_hIcon = hIcon;
	m_Center = Center;

	Invalidate();
}

void CIconCtrl::SetCoreIcon(UINT nID, BOOL Center)
{
	CRect rect;
	GetClientRect(rect);

	const INT Size = min(rect.Width(), rect.Height());
	const INT IconSize = (Size>=128) ? 128 : (Size>=96) ? 96 : (Size>=64) ? 64 : (Size>=48) ? 48 : (Size>=32) ? 32 : (Size>=24) ? 24 : 16;

	SetIcon((HICON)LoadImage(GetModuleHandle(_T("LFCORE.DLL")), MAKEINTRESOURCE(nID), IMAGE_ICON, IconSize, IconSize, LR_SHARED), IconSize, Center);
}

void CIconCtrl::SetTaskIcon(HINSTANCE hInst, UINT nID, BOOL Center)
{
	CRect rect;
	GetClientRect(rect);

	const INT Size = min(rect.Width(), rect.Height());
	const INT IconSize = (Size>=64) ? 64 : (Size>=48) ? 48 : (Size>=32) ? 32 : (Size>=24) ? 24 : 16;

	SetIcon((HICON)LoadImage(hInst, MAKEINTRESOURCE(nID), IMAGE_ICON, IconSize, IconSize, LR_SHARED), IconSize, Center);
}


BEGIN_MESSAGE_MAP(CIconCtrl, CFrontstageWnd)
	ON_WM_PAINT()
END_MESSAGE_MAP()

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

	FillRect(dc, rect, (HBRUSH)GetParent()->SendMessage(WM_CTLCOLORSTATIC, (WPARAM)dc.m_hDC, (LPARAM)m_hWnd));

	DrawIconEx(dc, 0, m_Center ? (rect.Height()-m_IconSize)/2 : 0, m_hIcon, m_IconSize, m_IconSize, 0, NULL, DI_NORMAL);

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);

	dc.SelectObject(pOldBitmap);
}
