
// CGroupBox.cpp: Implementierung der Klasse CGroupBox
//

#include "stdafx.h"
#include "LFCommDlg.h"
#include "resource.h"


// CGroupBox
//

extern AFX_EXTENSION_MODULE LFCommDlgDLL;

CGroupBox::CGroupBox()
	: CStatic()
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
	wndcls.lpszClassName = L"CGroupBox";

	if (!(::GetClassInfo(AfxGetInstanceHandle(), L"CGroupBox", &wndcls)))
	{
		wndcls.hInstance = AfxGetInstanceHandle();

		if (!AfxRegisterClass(&wndcls))
			AfxThrowResourceException();
	}
	if (!(::GetClassInfo(LFCommDlgDLL.hModule, L"CGroupBox", &wndcls)))
	{
		wndcls.hInstance = LFCommDlgDLL.hModule;

		if (!AfxRegisterClass(&wndcls))
			AfxThrowResourceException();
	}
}

void CGroupBox::PreSubclassWindow()
{
	CWnd::PreSubclassWindow();

	ModifyStyle(0, WS_CLIPSIBLINGS | WS_DISABLED);
}


BEGIN_MESSAGE_MAP(CGroupBox, CStatic)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
END_MESSAGE_MAP()

BOOL CGroupBox::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CGroupBox::OnPaint()
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

	BOOL Themed = IsCtrlThemed();

	// Background
	HBRUSH brush = (HBRUSH)GetParent()->SendMessage(WM_CTLCOLORBTN, (WPARAM)dc.m_hDC, (LPARAM)m_hWnd);
	if (brush)
		FillRect(dc, rect, brush);

	// Border
	CFont* pOldFont = (CFont*)dc.SelectStockObject(DEFAULT_GUI_FONT);
	COLORREF clr;

	CString caption;
	GetWindowText(caption);
	CSize sz = dc.GetTextExtent(caption);

	CRect rectBounds(rect);
	rectBounds.top += sz.cy/2;

	Graphics g(dc);
	g.SetCompositingMode(CompositingModeSourceOver);
	g.SetSmoothingMode(SmoothingModeAntiAlias);

	if (!Themed || (LFGetApp()->OSVersion==OS_Eight))
	{
		rectBounds.left++;
		rectBounds.top++;
		dc.Draw3dRect(rectBounds, GetSysColor(COLOR_3DHIGHLIGHT), GetSysColor(COLOR_3DHIGHLIGHT));

		rectBounds.OffsetRect(-1, -1);
		dc.Draw3dRect(rectBounds, GetSysColor(COLOR_3DSHADOW), GetSysColor(COLOR_3DSHADOW));

		clr = GetSysColor(COLOR_WINDOWTEXT);
	}
	else
	{
		rectBounds.right -= 1;
		rectBounds.bottom -= 1;

		GraphicsPath path;
		CreateRoundRectangle(rectBounds, 2, path);

		Pen pen(Color(204, 204, 204));
		g.DrawPath(&pen, &path);

		clr = (((LFDialog*)GetParent())->GetDesign()==LFDS_WHITE) ? 0xCB3300 : 0xCC6600;
	}

	// Caption
	CRect rectCaption(rect);
	rectCaption.left = rectBounds.left+6;
	rectCaption.right = min(rectCaption.left+sz.cx+4, rectBounds.right-6);
	rectCaption.bottom = rectCaption.top+sz.cy;

	if (brush)
		FillRect(dc, rectCaption, brush);

	dc.SetTextColor(clr);
	dc.DrawText(caption, rectCaption, DT_VCENTER | DT_CENTER | DT_END_ELLIPSIS | DT_SINGLELINE | DT_NOPREFIX);

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldFont);
	dc.SelectObject(pOldBitmap);
}
