
// CFrontstagePane.cpp: Implementierung der Klasse CFrontstagePane
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CFrontstagePane
//

CFrontstagePane::CFrontstagePane()
	: CFrontstageWnd()
{
	m_MaxWidth = GetMinWidth();
}

BOOL CFrontstagePane::Create(CWnd* pParentWnd, UINT nID, BOOL IsLeft, INT PreferredWidth, BOOL Shadow)
{
	m_IsLeft = IsLeft;
	m_PreferredWidth = PreferredWidth;
	m_Shadow = Shadow;

	CString className = AfxRegisterWndClass(CS_DBLCLKS, LFGetApp()->LoadStandardCursor(IDC_ARROW));

	return CFrontstageWnd::CreateEx(WS_EX_CONTROLPARENT | WS_EX_NOACTIVATE, className, _T(""), WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE, CRect(0, 0, 0, 0), pParentWnd, nID);
}

INT CFrontstagePane::GetMinWidth() const
{
	return 240+PANEGRIPPER;
}

void CFrontstagePane::AdjustLayout(CRect /*rectLayout*/)
{
}

void CFrontstagePane::SetMaxWidth(INT MaxWidth)
{
	m_MaxWidth = max(MaxWidth, GetMinWidth())-PANEGRIPPER;
}

void CFrontstagePane::GetLayoutRect(LPRECT lpRect) const
{
	GetClientRect(lpRect);

	if (m_IsLeft)
	{
		lpRect->right -= PANEGRIPPER;
	}
	else
	{
		lpRect->left += PANEGRIPPER;
	}
}


BEGIN_MESSAGE_MAP(CFrontstagePane, CFrontstageWnd)
	ON_WM_NCHITTEST()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

LRESULT CFrontstagePane::OnNcHitTest(CPoint point)
{
	LRESULT HitTest = CFrontstageWnd::OnNcHitTest(point);

	if (HitTest==HTCLIENT)
	{
		CRect rectLayout;
		GetLayoutRect(rectLayout);

		if (!rectLayout.PtInRect(point))
			HitTest = m_IsLeft ? HTRIGHT : HTLEFT;
	}

	return HitTest;
}

BOOL CFrontstagePane::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CFrontstagePane::OnPaint()
{
	CPaintDC pDC(this);

	CRect rect;
	GetClientRect(rect);

	CDC dc;
	dc.CreateCompatibleDC(&pDC);
	dc.SetBkMode(TRANSPARENT);

	CBitmap MemBitmap;
	MemBitmap.CreateCompatibleBitmap(&pDC, rect.Width(), rect.Height());
	CBitmap* pOldBitmap = dc.SelectObject(&MemBitmap);

	const BOOL Themed = IsCtrlThemed();

	dc.FillSolidRect(rect, Themed ? 0xFFFFFF : GetSysColor(COLOR_WINDOW));

	const INT GripperLeft = m_IsLeft ? rect.right-PANEGRIPPER : 0;

	if (Themed)
	{
		ASSERT(PANEGRIPPER==4);
		BYTE Colors[] = { 0x88, 0xC8, 0xE8, 0xF8 };

		const INT Line = rect.Height()*2/5;

		Graphics g(dc);
		g.SetPixelOffsetMode(PixelOffsetModeHalf);

		for (INT a=0; a<PANEGRIPPER; a++)
		{
			const BYTE clr = Colors[m_IsLeft ? PANEGRIPPER-1-a : a];

			LinearGradientBrush brush1(Point(0, 0), Point(0, Line), Color(0xFF, 0xFF, 0xFF), Color(clr, clr, clr));
			g.FillRectangle(&brush1, GripperLeft+a, 0, 1, Line);

			LinearGradientBrush brush2(Point(0, Line), Point(0, rect.Height()), Color(clr, clr, clr), Color(0xFF, 0xFF, 0xFF));
			g.FillRectangle(&brush2, GripperLeft+a, Line, 1, rect.Height()-Line);
		}

		if (m_Shadow)
			CTaskbar::DrawTaskbarShadow(g, rect);
	}
	else
	{
		dc.FillSolidRect(GripperLeft, rect.top, PANEGRIPPER, rect.Height(), GetSysColor(COLOR_3DFACE));
	}

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);

	dc.SelectObject(pOldBitmap);
}

void CFrontstagePane::OnSize(UINT nType, INT cx, INT cy)
{
	if (GetCapture()==this)
		m_PreferredWidth = cx;

	CFrontstageWnd::OnSize(nType, cx, cy);

	CRect rectLayout;
	GetLayoutRect(rectLayout);
	AdjustLayout(rectLayout);

	GetParent()->PostMessage(WM_ADJUSTLAYOUT);

	Invalidate();
}

void CFrontstagePane::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	CFrontstageWnd::OnGetMinMaxInfo(lpMMI);

	lpMMI->ptMinTrackSize.x = GetMinWidth();
	lpMMI->ptMaxTrackSize.x = max(GetMinWidth(), m_MaxWidth+PANEGRIPPER);
}
