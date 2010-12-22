
// CGridView.cpp: Implementierung der Klasse CGridView
//

#include "stdafx.h"
#include "CGridView.h"
#include "StoreManager.h"


// CGridView
//

#define GetItemData(idx)     ((FVItemData*)(m_ItemData+idx*m_DataSize))
#define CategoryPadding      3

CGridView::CGridView(UINT DataSize)
	: CFileView(DataSize)
{
	for (UINT a=0; a<LFItemCategoryCount; a++)
		AddItemCategory(theApp.m_ItemCategories[a]->Caption, theApp.m_ItemCategories[a]->Hint);
}

void CGridView::DrawItem(CDC& /*dc*/, LPRECT /*rectItem*/, INT /*idx*/, BOOL /*Themed*/)
{
}

void CGridView::DrawCategory(CDC& dc, LPRECT rectCategory, ItemCategory* ic, BOOL Themed)
{
	CRect rect(rectCategory);
	rect.DeflateRect(0, CategoryPadding);
	rect.left += CategoryPadding;

	CFont* pOldFont = dc.SelectObject(&theApp.m_LargeFont);
	dc.SetTextColor(Themed ? 0x993300 : GetSysColor(COLOR_WINDOWTEXT));
	dc.DrawText(ic->Caption, -1, rect, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_END_ELLIPSIS);

	CRect rectLine(rect);
	dc.DrawText(ic->Caption, -1, rectLine, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_END_ELLIPSIS | DT_CALCRECT);
	rectLine.right += 2*CategoryPadding;

	if (rectLine.right<=rect.right)
	{
		CPen pen(PS_SOLID, 1, 0xE2E2E2);

		CPen* pOldPen = dc.SelectObject(&pen);
		dc.MoveTo(rectLine.right, rect.top+(m_FontHeight[1]+1)/2);
		dc.LineTo(rect.right, rect.top+(m_FontHeight[1]+1)/2);
		dc.SelectObject(pOldPen);
	}

	if (ic->Hint[0]!=L'\0')
	{
		rect.top += m_FontHeight[1];
		dc.SelectObject(&theApp.m_DefaultFont);
		if (Themed)
			dc.SetTextColor(((dc.GetTextColor()>>1) & 0x7F7F7F) | 0x808080);
		dc.DrawText(ic->Hint, -1, rect, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_END_ELLIPSIS);
	}

	dc.SelectObject(pOldFont);
}

void CGridView::AddItemCategory(WCHAR* Caption, WCHAR* Hint)
{
	ItemCategory ic;
	ZeroMemory(&ic, sizeof(ic));
	wcscpy_s(ic.Caption, 256, Caption);
	wcscpy_s(ic.Hint, 256, Hint);

	m_Categories.AddItem(ic);
}

void CGridView::ResetItemCategories()
{
	for (UINT a=0; a<m_Categories.m_ItemCount; a++)
		ZeroMemory(&m_Categories.m_Items[a].Rect, sizeof(RECT));
}

void CGridView::ArrangeHorizontal(GVArrange& gva, BOOL Justify, BOOL ForceBreak, BOOL MaxWidth)
{
	ResetItemCategories();

	if (!p_Result)
		return;

	CClientDC dc(this);

	CRect rectWindow;
	GetWindowRect(rectWindow);
	if (!rectWindow.Width())
		return;

	m_ScrollWidth = m_ScrollHeight = 0;

	INT x = gva.mx;
	INT y = gva.my;
	const INT l = gva.cx+2*gva.padding;
	const INT h = gva.cy+2*gva.padding;
	ASSERT(l>0);
	ASSERT(h>0);
	m_RowHeight = h+gva.guttery;

	INT category = -1;

	INT gutter = 0;
	if ((Justify) && (!ForceBreak))
	{
		INT w = (rectWindow.Width()-gva.mx-gva.gutterx);
		INT c = w/(l+gva.gutterx);
		gutter = c>1 ? (w-l-gva.gutterx)/(c-1)-(l+gva.gutterx) : 0;
	}

	for (INT a=0; a<(INT)p_Result->m_ItemCount; a++)
	{
		if (m_HasCategories)
			if (((INT)p_Result->m_Items[a]->CategoryID!=category) && (p_Result->m_Items[a]->IconID!=IDI_FLD_Back))
			{
				if (x>gva.mx)
				{
					x = gva.mx;
					y += h+gva.guttery;
				}
				if (y>gva.my)
					y += 8;

				category = p_Result->m_Items[a]->CategoryID;
				LPRECT rect = &m_Categories.m_Items[category].Rect;
				rect->left = rect->right = x;
				rect->top = y;
				rect->bottom = rect->top+2*CategoryPadding+m_FontHeight[1];
				if (m_Categories.m_Items[category].Hint[0]!=L'\0')
					rect->bottom += m_FontHeight[0];

				y = rect->bottom+4;
				if (y+h+gva.guttery>m_ScrollHeight)
					m_ScrollHeight = y+h+gva.guttery;
			}

		FVItemData* d = GetItemData(a);
		d->Rect.left = x;
		d->Rect.top = y;
		d->Rect.right = x+l;
		d->Rect.bottom = y+h;

		x += l+gva.gutterx;
		if (x>m_ScrollWidth)
			m_ScrollWidth = x;

		if ((x+l>rectWindow.Width()) || (ForceBreak))
		{
			if (MaxWidth)
				d->Rect.right = rectWindow.Width()-gva.gutterx;

			x = gva.mx;
			y += h+gva.guttery;
			if (y>m_ScrollHeight)
				m_ScrollHeight = y;
				// TODO
		}
	}

	// Adjust categories to calculated width
	for (UINT a=0; a<m_Categories.m_ItemCount; a++)
		if (m_Categories.m_Items[a].Rect.right)
			m_Categories.m_Items[a].Rect.right = max(m_ScrollWidth, rectWindow.Width())-gva.gutterx;

	AdjustScrollbars();
	Invalidate();
}

void CGridView::ArrangeVertical(GVArrange& gva)
{
	ResetItemCategories();

	if (!p_Result)
		return;

	CClientDC dc(this);

	CRect rectWindow;
	GetWindowRect(rectWindow);
	if (!rectWindow.Width())
		return;

	INT top = gva.my;
	if (m_HasCategories)
		top += 2*CategoryPadding+m_FontHeight[1]+4;

	m_ScrollWidth = m_ScrollHeight = 0;

	INT x = gva.mx;
	INT y = top;
	const INT l = gva.cx+2*gva.padding;
	const INT h = gva.cy+2*gva.padding;
	ASSERT(l>0);
	ASSERT(h>0);
	m_RowHeight = h+gva.guttery;

	INT category = -1;
	INT lastleft = x;
#define FinishCategory if (category!=-1) { m_Categories.m_Items[category].Rect.right = lastleft+l; }

	for (INT a=0; a<(INT)p_Result->m_ItemCount; a++)
	{
		if (m_HasCategories)
			if (((INT)p_Result->m_Items[a]->CategoryID!=category) && (p_Result->m_Items[a]->IconID!=IDI_FLD_Back))
			{
				FinishCategory;

				if (y>top)
				{
					y = top;
					x += l+gva.gutterx;
				}
				if (x>gva.mx)
					x += 8;

				category = p_Result->m_Items[a]->CategoryID;
				LPRECT rect = &m_Categories.m_Items[category].Rect;
				rect->left = x;
				rect->top = gva.my;
				rect->bottom = rect->top+2*CategoryPadding+m_FontHeight[1];

				if (x+l+gva.gutterx>m_ScrollWidth)
					m_ScrollWidth = x+l+gva.gutterx;
			}

		FVItemData* d = GetItemData(a);
		d->Rect.left = lastleft = x;
		d->Rect.top = y;
		d->Rect.right = x+l;
		d->Rect.bottom = y+h;

		if (a==(INT)p_Result->m_ItemCount-1)
			FinishCategory;

		y += h+gva.guttery;
		if (y>m_ScrollHeight)
			m_ScrollHeight = y;

		if (y+h>rectWindow.Height())
		{
			y = top;
			x += l+gva.gutterx;
			if (x>m_ScrollWidth)
				m_ScrollWidth = x;
				// TODO
		}
	}

	AdjustScrollbars();
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
			RECT rectIntersect;

			for (UINT a=0; a<p_Result->m_ItemCount; a++)
			{
				CRect rect(GetItemRect(a));
				if (IntersectRect(&rectIntersect, rect, rectUpdate))
				{
					DrawItemBackground(dc, rect, a, Themed);
					DrawItem(dc, rect, a, Themed);
				}
			}

			for (UINT a=0; a<m_Categories.m_ItemCount; a++)
			{
				CRect rect(m_Categories.m_Items[a].Rect);
				rect.OffsetRect(-m_HScrollPos, -m_VScrollPos+(INT)m_HeaderHeight);
				if (IntersectRect(&rectIntersect, rect, rectUpdate))
					DrawCategory(dc, rect, &m_Categories.m_Items[a], Themed);
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
