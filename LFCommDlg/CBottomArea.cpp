
// CBottomArea.cpp: Implementierung der Klasse CBottomArea
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CBottomArea
//

extern AFX_EXTENSION_MODULE LFCommDlgDLL;

CBottomArea::CBottomArea()
	: CDialogBar()
{
	p_App = LFGetApp();
	hBackgroundBrush = NULL;
	m_BackBufferL = m_BackBufferH = 0;
}


BEGIN_MESSAGE_MAP(CBottomArea, CDialogBar)
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_THEMECHANGED()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

void CBottomArea::OnDestroy()
{
	if (hBackgroundBrush)
		DeleteObject(hBackgroundBrush);

	CDialogBar::OnDestroy();
}

BOOL CBottomArea::OnEraseBkgnd(CDC* pDC)
{
	CRect rect;
	GetClientRect(rect);

	CDC dc;
	dc.CreateCompatibleDC(pDC);
	dc.SetBkMode(TRANSPARENT);

	CBitmap* pOldBitmap;
	if ((m_BackBufferL!=rect.Width()) || (m_BackBufferH!=rect.Height()))
	{
		m_BackBufferL = rect.Width();
		m_BackBufferH = rect.Height();

		m_BackBuffer.DeleteObject();
		m_BackBuffer.CreateCompatibleBitmap(pDC, rect.Width(), rect.Height());
		pOldBitmap = dc.SelectObject(&m_BackBuffer);

		Graphics g(dc);
		g.SetCompositingMode(CompositingModeSourceOver);

		if (IsCtrlThemed())
		{
			dc.FillSolidRect(rect, 0xFEFEFE);

			CGdiPlusBitmap* pDivider = p_App->GetCachedResourceImage(IDB_DIVDOWN, _T("PNG"), LFCommDlgDLL.hResource);
			g.DrawImage(pDivider->m_pBitmap, (rect.Width()-(INT)pDivider->m_pBitmap->GetWidth())/2, 0);
		}
		else
		{
			dc.FillSolidRect(rect, GetSysColor(COLOR_3DFACE));
		}

		if (hBackgroundBrush)
			DeleteObject(hBackgroundBrush);
		hBackgroundBrush = CreatePatternBrush(m_BackBuffer);
	}
	else
	{
		pOldBitmap = dc.SelectObject(&m_BackBuffer);
	}

	pDC->BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldBitmap);

	return TRUE;
}

void CBottomArea::OnSize(UINT nType, INT cx, INT cy)
{
	CDialogBar::OnSize(nType, cx, cy);

	m_BackBufferL = m_BackBufferH = 0;
	Invalidate();
}

LRESULT CBottomArea::OnThemeChanged()
{
	m_BackBufferL = m_BackBufferH = 0;
	return TRUE;
}

void CBottomArea::OnSysColorChange()
{
	if (!IsCtrlThemed())
		m_BackBufferL = m_BackBufferH = 0;
}

HBRUSH CBottomArea::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	// Call base class version at first, else it will override changes
	HBRUSH hbr = CDialogBar::OnCtlColor(pDC, pWnd, nCtlColor);

	if ((nCtlColor==CTLCOLOR_BTN) || (nCtlColor==CTLCOLOR_STATIC))
	{
		CRect rc;
		pWnd->GetWindowRect(&rc);
		ScreenToClient(&rc);

		pDC->SetBkMode(TRANSPARENT);
		pDC->SetBrushOrg(-rc.left, -rc.top);

		hbr = hBackgroundBrush;
	}

	return hbr;
}
