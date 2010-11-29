
// CGridView.cpp: Implementierung der Klasse CGridView
//

#include "stdafx.h"
#include "CGridView.h"
#include "StoreManager.h"


// CGridView
//

#define GetItemData(idx)     ((FVItemData*)(m_ItemData+idx*m_DataSize))

CGridView::CGridView(UINT DataSize)
	: CFileView(DataSize)
{
}

void CGridView::EditLabel(INT idx)
{
	m_EditLabel = -1;
}

BOOL CGridView::IsEditing()
{
	return FALSE;
}

void CGridView::DrawItem(CDC& /*dc*/, LPRECT /*rectItem*/, INT /*idx*/, BOOL /*Themed*/)
{
}

void CGridView::ArrangeHorizontal(GVArrange& gva, BOOL Justify, BOOL ForceBreak, BOOL MaxWidth)
{
	if (!p_Result)
		return;

	CClientDC dc(this);

	CRect rectClient;
	GetClientRect(rectClient);
	if (!rectClient.Width())
		return;

	INT x = gva.mx;
	INT y = gva.my;
	const INT l = gva.cx+2*gva.padding;
	const INT h = gva.cy+2*gva.padding;
	ASSERT(l>0);
	ASSERT(h>0);

	INT gutter = 0;
	if ((Justify) && (!ForceBreak))
	{
		INT w = (rectClient.Width()-gva.mx-gva.gutterx);
		INT c = w/(l+gva.gutterx);
		gutter = c>1 ? (w-l-gva.gutterx)/(c-1)-(l+gva.gutterx) : 0;
	}

	for (INT a=0; a<(INT)p_Result->m_ItemCount; a++)
	{
		FVItemData* d = GetItemData(a);
		d->Rect.left = x;
		d->Rect.top = y;
		d->Rect.right = x+l;
		d->Rect.bottom = y+h;

		x += l+gva.gutterx;
		if ((x+l>rectClient.Width()) || (ForceBreak))
		{
			if (MaxWidth)
				d->Rect.right = rectClient.right-gva.gutterx;

			x = gva.mx;
			y += h+gva.guttery;
		}
	}

	Invalidate();
}

void CGridView::ArrangeVertical(GVArrange& gva)
{
	if (!p_Result)
		return;

	CClientDC dc(this);

	CRect rectClient;
	GetClientRect(rectClient);
	if (!rectClient.Width())
		return;

	INT x = gva.mx;
	INT y = gva.my;
	const INT l = gva.cx+2*gva.padding;
	const INT h = gva.cy+2*gva.padding;
	ASSERT(l>0);
	ASSERT(h>0);

	for (INT a=0; a<(INT)p_Result->m_ItemCount; a++)
	{
		FVItemData* d = GetItemData(a);
		d->Rect.left = x;
		d->Rect.top = y;
		d->Rect.right = x+l;
		d->Rect.bottom = y+h;

		y += h+gva.guttery;
		if (y+h>rectClient.Height())
		{
			y = gva.my;
			x += l+gva.gutterx;
		}
	}

	Invalidate();
}


BEGIN_MESSAGE_MAP(CGridView, CFileView)
	ON_WM_PAINT()
END_MESSAGE_MAP()

void CGridView::OnPaint()
{
	CRect rectUpdate;
	GetUpdateRect(rectUpdate);

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
	COLORREF bkCol = Themed ? 0xFFFFFF : GetSysColor(COLOR_WINDOW);
	dc.FillSolidRect(rect, bkCol);

	CFont* pOldFont = dc.SelectObject(&theApp.m_DefaultFont);

	BOOL Nothing = TRUE;

	if (p_Result)
		if (p_Result->m_ItemCount)
		{
			Nothing = FALSE;

			for (UINT a=0; a<p_Result->m_ItemCount; a++)
			{
				FVItemData* d = GetItemData(a);
				RECT rectIntersect;
				if (IntersectRect(&rectIntersect, &d->Rect, rectUpdate))
				{
					DrawItemBackground(dc, &d->Rect, a, Themed);
					DrawItem(dc, &d->Rect, a, Themed);
				}
			}
		}

	if (Nothing)
	{
		CRect rectText(rect);
		rectText.top += m_HeaderHeight+6;

		CString tmpStr;
		ENSURE(tmpStr.LoadString(IDS_NOTHINGTODISPLAY));

		dc.SetTextColor(Themed ? 0x6D6D6D : GetSysColor(COLOR_3DFACE));
		dc.DrawText(tmpStr, -1, rectText, DT_CENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
	}

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldFont);
	dc.SelectObject(pOldBitmap);
}
