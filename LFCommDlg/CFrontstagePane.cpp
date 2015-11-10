
// CFrontstagePane.cpp: Implementierung der Klasse CFrontstagePane
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CFrontstagePane
//

#define GRIPPER     4

CFrontstagePane::CFrontstagePane()
	: CFrontstageWnd()
{
	m_MaxWidth = 128+GRIPPER;
}

BOOL CFrontstagePane::Create(CWnd* pParentWnd, UINT nID, BOOL IsLeft, INT PreferredWidth)
{
	m_IsLeft = IsLeft;
	m_PreferredWidth = PreferredWidth;

	CString className = AfxRegisterWndClass(CS_DBLCLKS, LFGetApp()->LoadStandardCursor(IDC_ARROW));

	return CFrontstageWnd::CreateEx(WS_EX_CONTROLPARENT | WS_EX_NOACTIVATE, className, _T(""), WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE, CRect(0, 0, 0, 0), pParentWnd, nID);
}

void CFrontstagePane::AdjustLayout(CRect /*rectLayout*/)
{
}

INT CFrontstagePane::GetPreferredWidth() const
{
	return m_PreferredWidth;
}

void CFrontstagePane::SetMaxWidth(INT MaxWidth)
{
	m_MaxWidth = MaxWidth-GRIPPER;
}

void CFrontstagePane::GetLayoutRect(LPRECT lpRect) const
{
	GetClientRect(lpRect);

	if (m_IsLeft)
	{
		lpRect->right -= GRIPPER;
	}
	else
	{
		lpRect->left += GRIPPER;
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
	MemBitmap.CreateCompatibleBitmap(&pDC, GRIPPER, rect.Height());
	CBitmap* pOldBitmap = dc.SelectObject(&MemBitmap);

	if (IsCtrlThemed())
	{
		dc.FillSolidRect(0, 0, GRIPPER, 1, 0xE7E7E7);
		dc.FillSolidRect(0, 1, GRIPPER, 1, 0xF3F3F3);

		ASSERT(GRIPPER==4);
		BYTE Colors[] = { 0x88, 0xC8, 0xE8, 0xF8 };
		INT Line = rect.Height()*2/5;

		Graphics g(dc);
		g.SetPixelOffsetMode(PixelOffsetModeHalf);

		for (INT a=0; a<GRIPPER; a++)
		{
			const BYTE clr = Colors[m_IsLeft ? GRIPPER-1-a : a];

			LinearGradientBrush brush1(Point(0, 0), Point(0, Line), Color(0xFF, 0xFF, 0xFF), Color(clr, clr, clr));
			g.FillRectangle(&brush1, a, 2, 1, Line-2);

			LinearGradientBrush brush2(Point(0, Line), Point(0, rect.Height()), Color(clr, clr, clr), Color(0xFF, 0xFF, 0xFF));
			g.FillRectangle(&brush2, a, Line, 1, rect.Height()-Line);
		}
	}
	else
	{
		dc.FillSolidRect(rect.left, rect.top, GRIPPER, rect.Height(), GetSysColor(COLOR_3DFACE));
	}

	pDC.BitBlt(m_IsLeft ? rect.Width()-GRIPPER : 0, 0, GRIPPER, rect.Height(), &dc, 0, 0, SRCCOPY);
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
}

void CFrontstagePane::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	CFrontstageWnd::OnGetMinMaxInfo(lpMMI);

	lpMMI->ptMinTrackSize.x = 140+GRIPPER;
	lpMMI->ptMaxTrackSize.x = m_MaxWidth+GRIPPER;
}
