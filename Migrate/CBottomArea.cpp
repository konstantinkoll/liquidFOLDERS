
// CBottomArea.cpp: Implementierung der Klasse CBottomArea
//

#include "stdafx.h"
#include "CBottomArea.h"
#include "Resource.h"
#include "Migrate.h"
#include "LFCore.h"


// CBottomArea
//

CBottomArea::CBottomArea()
{
}


BEGIN_MESSAGE_MAP(CBottomArea, CDialogBar)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
END_MESSAGE_MAP()

BOOL CBottomArea::OnEraseBkgnd(CDC* pDC)
{
	CRect rect;
	GetClientRect(rect);

	if (((CGlasWindow*)GetParent())->GetDesign()>GWD_DEFAULT)
	{
		pDC->FillSolidRect(rect.top++, rect.left, rect.Width(), 1, 0xDFDFDF);
		pDC->FillSolidRect(rect, 0xF0F0F0);
	}
	else
	{
		pDC->FillSolidRect(rect, GetSysColor(COLOR_3DFACE));
	}

	return TRUE;
}

void CBottomArea::OnPaint()
{
	CPaintDC(this);
}
