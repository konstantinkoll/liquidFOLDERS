
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
	HINSTANCE hInst = LFCommDlgDLL.hModule;

	if (!(::GetClassInfo(hInst, L"CGroupBox", &wndcls)))
	{
		wndcls.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
		wndcls.lpfnWndProc = ::DefWindowProc;
		wndcls.cbClsExtra = wndcls.cbWndExtra = 0;
		wndcls.hInstance = hInst;
		wndcls.hIcon = NULL;
		wndcls.hCursor = LoadCursor(NULL, IDC_ARROW);
		wndcls.hbrBackground = NULL;
		wndcls.lpszMenuName = NULL;
		wndcls.lpszClassName = L"CGroupBox";

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
	COLORREF clr = 0x000000;

	CString caption;
	GetWindowText(caption);
	CSize sz = dc.GetTextExtent(caption);

	CRect rectBounds(rect);
	rectBounds.top += sz.cy/2;

	if (!Themed)
	{
		rectBounds.left++;
		rectBounds.top++;
		dc.Draw3dRect(rectBounds, GetSysColor(COLOR_3DHIGHLIGHT), GetSysColor(COLOR_3DHIGHLIGHT));

		rectBounds.OffsetRect(-1, -1);
		dc.Draw3dRect(rectBounds, GetSysColor(COLOR_3DSHADOW), GetSysColor(COLOR_3DSHADOW));
	}
	else
	{
		Graphics g(dc);
		g.SetCompositingMode(CompositingModeSourceOver);
		g.SetSmoothingMode(SmoothingModeAntiAlias);

		switch (((LFDialog*)GetParent())->GetDesign())
		{
		case LFDS_Blue:
			{
				rectBounds.right -= 3;
				rectBounds.bottom -= 3;

				Matrix m1;
				m1.Translate(2.0, 2.0);

				Matrix m2;
				m2.Translate(-1.0, -1.0);

				GraphicsPath path;
				CreateRoundRectangle(rectBounds, 2, path);

				Pen pen(Color(224, 196, 240, 248));
				g.DrawPath(&pen, &path);

				path.Transform(&m1);
				pen.SetColor(Color(128, 255, 255, 255));
				g.DrawPath(&pen, &path);

				path.Transform(&m2);
				pen.SetColor(Color(64, 60, 96, 112));
				g.DrawPath(&pen, &path);

				clr = 0xCC6600;
				break;
			}
		case LFDS_White:
		case LFDS_UAC:
			{
				rectBounds.right -= 1;
				rectBounds.bottom -= 1;

				GraphicsPath path;
				CreateRoundRectangle(rectBounds, 2, path);

				Pen pen(Color(204, 204, 204));
				g.DrawPath(&pen, &path);

				clr = 0x808080;
				break;
			}
		}
	}

	// Caption
	CRect rectCaption(rect);
	rectCaption.left = rectBounds.left+6;
	rectCaption.right = min(rectCaption.left+sz.cx+4, rectBounds.right-6);
	rectCaption.bottom = rectCaption.top+sz.cy;

	if (brush)
		FillRect(dc, rectCaption, brush);

	dc.SetTextColor(clr);
	dc.DrawText(caption, rectCaption, DT_VCENTER | DT_CENTER | DT_END_ELLIPSIS | DT_SINGLELINE);

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldFont);
	dc.SelectObject(pOldBitmap);
}
