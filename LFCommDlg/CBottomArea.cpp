
// CBottomArea.cpp: Implementierung der Klasse CBottomArea
//

#include "stdafx.h"
#include "CBottomArea.h"
#include "CGlasWindow.h"


// CBottomArea
//

CBottomArea::CBottomArea()
{
	m_Design = GWD_DEFAULT;
}

void CBottomArea::SetDesign(UINT _Design)
{
	m_Design = _Design;
}


BEGIN_MESSAGE_MAP(CBottomArea, CDialogBar)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

int CBottomArea::OnCreate(LPCREATESTRUCT lpcs)
{
	if (CWnd::OnCreate(lpcs)==-1)
		return -1;

	hBackgroundBrush = CreateSolidBrush(0xF0F0F0);

	return 0;
}

void CBottomArea::OnDestroy()
{
	if (hBackgroundBrush)
		DeleteObject(hBackgroundBrush);

	CWnd::OnDestroy();
}

BOOL CBottomArea::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CBottomArea::OnPaint()
{
	CPaintDC pDC(this);

	CRect rect;
	GetClientRect(rect);

	if (m_Design>GWD_DEFAULT)
	{
		pDC.FillSolidRect(rect.top++, rect.left, rect.Width(), 1, 0xDFDFDF);
		pDC.FillSolidRect(rect, 0xF0F0F0);
	}
	else
	{
		pDC.FillSolidRect(rect, GetSysColor(COLOR_3DFACE));
	}
}

HBRUSH CBottomArea::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	// Call base class version at first, else it will override changes
	HBRUSH hbr = CWnd::OnCtlColor(pDC, pWnd, nCtlColor);

	if (m_Design>GWD_DEFAULT)
		if ((nCtlColor==CTLCOLOR_BTN) || (nCtlColor==CTLCOLOR_STATIC))
		{
			pDC->SetBkMode(TRANSPARENT);
			hbr = hBackgroundBrush;
		}

	return hbr;
}
