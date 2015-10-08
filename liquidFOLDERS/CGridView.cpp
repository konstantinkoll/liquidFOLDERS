
// CGridView.cpp: Implementierung der Klasse CGridView
//

#include "stdafx.h"
#include "CGridView.h"
#include "liquidFOLDERS.h"


// CGridView
//

#define GetItemData(Index)     ((GridItemData*)(m_ItemData+(Index)*m_DataSize))

CGridView::CGridView(UINT DataSize, BOOL EnableLabelEdit)
	: CFileView(DataSize, TRUE, TRUE, TRUE, TRUE, EnableLabelEdit)
{
	for (UINT a=0; a<LFItemCategoryCount; a++)
		AddItemCategory(theApp.m_ItemCategories[a].Caption, theApp.m_ItemCategories[a].Hint);

	m_HasCategories = FALSE;
	m_GridArrange = GRIDARRANGE_CUSTOM;
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

	if (!p_CookedFiles)
	{
		m_ScrollWidth = m_ScrollHeight = 0;
		return;
	}

	CRect rectWindow;
	GetWindowRect(&rectWindow);

	if (!rectWindow.Width())
		return;

	const INT l = gva.cx+2*gva.padding;
	const INT h = gva.cy+2*gva.padding;
	ASSERT(l>0);
	ASSERT(h>0);
	m_RowHeight = h+gva.guttery;

	BOOL HasScrollbars = FALSE;

Restart:
	m_ScrollWidth = m_ScrollHeight = 0;

	INT col = 0;
	INT row = 0;
	INT x = gva.mx;
	INT y = gva.my;

	INT category = -1;

	INT gutter = 0;
	if ((Justify) && (!ForceBreak))
	{
		INT w = (rectWindow.Width()-gva.mx-gva.gutterx);
		INT c = w/(l+gva.gutterx);
		gutter = c>1 ? (w-l-gva.gutterx)/(c-1)-(l+gva.gutterx) : 0;
	}

	for (INT a=0; a<(INT)p_CookedFiles->m_ItemCount; a++)
	{
		if (m_HasCategories)
			if ((INT)p_CookedFiles->m_Items[a]->CategoryID!=category)
			{
				if (x>gva.mx)
				{
					col = 0;
					row++;
					x = gva.mx;
					y += h+gva.guttery;
				}
				if (y>gva.my)
					y += 8;

				category = p_CookedFiles->m_Items[a]->CategoryID;
				LPRECT rect = &m_Categories.m_Items[category].Rect;
				rect->left = rect->right = x;
				rect->top = y;
				rect->bottom = rect->top+2*LFCategoryPadding+m_FontHeight[1];
				if (m_Categories.m_Items[category].Hint[0]!=L'\0')
					rect->bottom += m_FontHeight[0];

				y = rect->bottom+4;
			}

		GridItemData* d = GetItemData(a);
		d->Column = col++;
		d->Row = row;
		d->Hdr.Rect.left = x;
		d->Hdr.Rect.top = y;
		d->Hdr.Rect.right = x+l;
		d->Hdr.Rect.bottom = y+h;

		x += l+gva.gutterx;
		if (x>m_ScrollWidth)
			m_ScrollWidth = x-1;

		if (y+h+gva.guttery-1>m_ScrollHeight)
		{
			m_ScrollHeight = y+h+max(gva.guttery, 0);
			if ((m_ScrollHeight>rectWindow.Height()) && (!HasScrollbars))
			{
				HasScrollbars = TRUE;
				rectWindow.right -= GetSystemMetrics(SM_CXVSCROLL);
				goto Restart;
			}
		}

		if ((x+l+gva.gutterx>rectWindow.Width()) || (ForceBreak))
		{
			if (MaxWidth)
				d->Hdr.Rect.right = rectWindow.Width()-gva.gutterx;

			col = 0;
			row++;
			x = gva.mx;
			y += h+gva.guttery;
		}
	}

	// Adjust categories to calculated width
	for (UINT a=0; a<m_Categories.m_ItemCount; a++)
		if (m_Categories.m_Items[a].Rect.right)
			m_Categories.m_Items[a].Rect.right = max(m_ScrollWidth, rectWindow.Width())-2;

	m_GridArrange = GRIDARRANGE_HORIZONTAL;
	CFileView::AdjustLayout();
}

void CGridView::ArrangeVertical(GVArrange& gva)
{
	ResetItemCategories();

	if (!p_CookedFiles)
		return;

	CRect rectWindow;
	GetWindowRect(&rectWindow);

	if (!rectWindow.Width())
		return;

	INT top = gva.my;
	if (m_HasCategories)
		top += 2*LFCategoryPadding+m_FontHeight[1]+4;

	const INT l = gva.cx+2*gva.padding;
	const INT h = gva.cy+2*gva.padding;
	ASSERT(l>0);
	ASSERT(h>0);
	m_RowHeight = h+gva.guttery;

	BOOL HasScrollbars = FALSE;

Restart:
	m_ScrollWidth = m_ScrollHeight = 0;

	INT col = 0;
	INT row = 0;
	INT x = gva.mx;
	INT y = top;

	INT category = -1;
	INT lastleft = x;
#define FinishCategory if (category!=-1) { m_Categories.m_Items[category].Rect.right = lastleft+l; }

	for (INT a=0; a<(INT)p_CookedFiles->m_ItemCount; a++)
	{
		if (m_HasCategories)
			if ((INT)p_CookedFiles->m_Items[a]->CategoryID!=category)
			{
				FinishCategory;

				if (y>top)
				{
					row = 0;
					col++;
					y = top;
					x += l+gva.gutterx;
				}
				if (x>gva.mx)
					x += 8;

				category = p_CookedFiles->m_Items[a]->CategoryID;
				LPRECT rect = &m_Categories.m_Items[category].Rect;
				rect->left = x;
				rect->top = gva.my;
				rect->bottom = rect->top+2*LFCategoryPadding+m_FontHeight[1];
			}

		GridItemData* d = GetItemData(a);
		d->Column = col;
		d->Row = row++;
		d->Hdr.Rect.left = lastleft = x;
		d->Hdr.Rect.top = y;
		d->Hdr.Rect.right = x+l;
		d->Hdr.Rect.bottom = y+h;

		if (a==(INT)p_CookedFiles->m_ItemCount-1)
			FinishCategory;

		y += h+gva.guttery;
		if (y>m_ScrollHeight)
			m_ScrollHeight = y-1;

		if (x+l+gva.gutterx-1>m_ScrollWidth)
		{
			m_ScrollWidth = x+l+max(gva.gutterx, 0);
			if ((m_ScrollWidth>rectWindow.Width()) && (!HasScrollbars))
			{
				HasScrollbars = TRUE;
				rectWindow.bottom -= GetSystemMetrics(SM_CXHSCROLL);
				goto Restart;
			}
		}

		if (y+h+gva.guttery>rectWindow.Height())
		{
			row = 0;
			col++;
			y = top;
			x += l+gva.gutterx;
		}
	}

	m_GridArrange = GRIDARRANGE_VERTICAL;
	CFileView::AdjustLayout();
}

void CGridView::HandleHorizontalKeys(UINT nChar, UINT /*nRepCnt*/, UINT /*nFlags*/)
{
	if (p_CookedFiles)
	{
		CRect rect;
		GetClientRect(&rect);

		INT Item = m_FocusItem;
		INT col = (Item==-1) ? 0 : GetItemData(Item)->Column;
		INT row = (Item==-1) ? 0 : GetItemData(Item)->Row;
		INT top = (Item==-1) ? 0 : GetItemData(Item)->Hdr.Rect.top;
		INT bottom = (Item==-1) ? 0 : GetItemData(Item)->Hdr.Rect.bottom;
		INT tmprow = -1;

		switch (nChar)
		{
		case VK_LEFT:
			for (INT a=Item-1; a>=0; a--)
			{
				GridItemData* d = GetItemData(a);
				if ((d->Row==row) && d->Hdr.Valid)
				{
					Item = a;
					break;
				}
			}

			break;

		case VK_RIGHT:
			for (INT a=Item+1; a<(INT)p_CookedFiles->m_ItemCount; a++)
			{
				GridItemData* d = GetItemData(a);
				if ((d->Row==row) && d->Hdr.Valid)
				{
					Item = a;
					break;
				}
			}

			break;

		case VK_UP:
			for (INT a=Item-1; a>=0; a--)
			{
				GridItemData* d = GetItemData(a);
				if ((d->Row<row) && d->Hdr.Valid)
				{
					Item = a;
					if (d->Column<=col)
						break;
				}
				if (d->Row<row-1)
					break;
			}

			break;

		case VK_PRIOR:
			for (INT a=Item-1; a>=0; a--)
			{
				GridItemData* d = GetItemData(a);
				if ((d->Row<=row) && (d->Column<=col) && d->Hdr.Valid)
				{
					Item = a;
					if (d->Hdr.Rect.top<=bottom-rect.Height()+(INT)m_HeaderHeight)
						break;
				}
			}

			break;

		case VK_DOWN:
			for (INT a=Item+1; a<(INT)p_CookedFiles->m_ItemCount; a++)
			{
				GridItemData* d = GetItemData(a);
				if ((d->Row>row) && d->Hdr.Valid)
				{
					Item = a;
					if (d->Column>=col)
						break;
				}
				if (d->Row>row+1)
					break;
			}

			break;

		case VK_NEXT:
			for (INT a=Item+1; a<(INT)p_CookedFiles->m_ItemCount; a++)
			{
				GridItemData* d = GetItemData(a);
				if (d->Row!=GetItemData(a-1)->Row)
					if (d->Hdr.Rect.bottom>=top+rect.Height()-(INT)m_HeaderHeight)
						if (tmprow==-1)
						{
							tmprow = d->Row;
						}
						else
						{
							if (d->Row>tmprow)
								break;
						}
				if (((tmprow==-1) || (d->Column<=col)) && d->Hdr.Valid)
					Item = a;
			}

			break;

		case VK_HOME:
			if (GetKeyState(VK_CONTROL)<0)
			{
				for (INT a=0; a<(INT)p_CookedFiles->m_ItemCount; a++)
					if (GetItemData(a)->Hdr.Valid)
					{
						Item = a;
						break;
					}
			}
			else
				for (INT a=Item-1; a>=0; a--)
				{
					GridItemData* d = GetItemData(a);
					if (d->Hdr.Valid)
						if (d->Row==row)
						{
							Item = a;
						}
						else
						{
							break;
						}
				}

			break;

		case VK_END:
			if (GetKeyState(VK_CONTROL)<0)
			{
				for (INT a=(INT)p_CookedFiles->m_ItemCount-1; a>=0; a--)
					if (GetItemData(a)->Hdr.Valid)
					{
						Item = a;
						break;
					}
			}
			else
				for (INT a=Item+1; a<(INT)p_CookedFiles->m_ItemCount; a++)
				{
					GridItemData* d = GetItemData(a);
					if (d->Hdr.Valid)
						if (d->Row==row)
						{
							Item = a;
						}
						else
						{
							break;
						}
				}

			break;
		}

		if (Item!=m_FocusItem)
		{
			m_ShowFocusRect = TRUE;
			SetFocusItem(Item, GetKeyState(VK_SHIFT)<0);

			CPoint pt;
			GetCursorPos(&pt);
			ScreenToClient(&pt);
			OnMouseMove(0, pt);
		}
	}
}

void CGridView::HandleVerticalKeys(UINT nChar, UINT /*nRepCnt*/, UINT /*nFlags*/)
{
	if (p_CookedFiles)
	{
		CRect rect;
		GetClientRect(&rect);

		INT Item = m_FocusItem;
		INT col = (Item==-1) ? 0 : GetItemData(Item)->Column;
		INT row = (Item==-1) ? 0 : GetItemData(Item)->Row;
		INT left = (Item==-1) ? 0 : GetItemData(Item)->Hdr.Rect.left;
		INT right = (Item==-1) ? 0 : GetItemData(Item)->Hdr.Rect.right;
		INT tmpcol = -1;

		switch (nChar)
		{
		case VK_LEFT:
			for (INT a=Item-1; a>=0; a--)
			{
				GridItemData* d = GetItemData(a);
				if ((d->Column<col) && d->Hdr.Valid)
				{
					Item = a;
					if (d->Row<=row)
						break;
				}
				if (d->Column<col-1)
					break;
			}

			break;

		case VK_PRIOR:
			for (INT a=Item-1; a>=0; a--)
			{
				GridItemData* d = GetItemData(a);
				if ((d->Column<=col) && (d->Row<=row) && d->Hdr.Valid)
				{
					Item = a;
					if (d->Hdr.Rect.left<=right-rect.Width())
						break;
				}
			}

			break;

		case VK_RIGHT:
			for (INT a=Item+1; a<(INT)p_CookedFiles->m_ItemCount; a++)
			{
				GridItemData* d = GetItemData(a);
				if ((d->Column>col) && d->Hdr.Valid)
				{
					Item = a;
					if (d->Row>=row)
						break;
				}
				if (d->Column>col+1)
					break;
			}

			break;

		case VK_NEXT:
			for (INT a=Item+1; a<(INT)p_CookedFiles->m_ItemCount; a++)
			{
				GridItemData* d = GetItemData(a);
				if (d->Column!=GetItemData(a-1)->Column)
					if (d->Hdr.Rect.right>=left+rect.Width())
						if (tmpcol==-1)
						{
							tmpcol = d->Column;
						}
						else
						{
							if (d->Column>tmpcol)
								break;
						}
				if (((tmpcol==-1) || (d->Row<=row)) && d->Hdr.Valid)
					Item = a;
			}

			break;

		case VK_UP:
			for (INT a=Item-1; a>=0; a--)
			{
				GridItemData* d = GetItemData(a);
				if ((d->Column==col) && d->Hdr.Valid)
				{
					Item = a;
					break;
				}
			}

			break;

		case VK_DOWN:
			for (INT a=Item+1; a<(INT)p_CookedFiles->m_ItemCount; a++)
			{
				GridItemData* d = GetItemData(a);
				if ((d->Column==col) && d->Hdr.Valid)
				{
					Item = a;
					break;
				}
			}

			break;

		case VK_HOME:
			if (GetKeyState(VK_CONTROL)<0)
			{
				for (INT a=0; a<(INT)p_CookedFiles->m_ItemCount; a++)
					if (GetItemData(a)->Hdr.Valid)
					{
						Item = a;
						break;
					}
			}
			else
				for (INT a=Item-1; a>=0; a--)
				{
					GridItemData* d = GetItemData(a);
					if (d->Hdr.Valid)
						if (d->Column==col)
						{
							Item = a;
						}
						else
						{
							break;
						}
				}

			break;

		case VK_END:
			if (GetKeyState(VK_CONTROL)<0)
			{
				for (INT a=(INT)p_CookedFiles->m_ItemCount-1; a>=0; a--)
					if (GetItemData(a)->Hdr.Valid)
					{
						Item = a;
						break;
					}
			}
			else
				for (INT a=Item+1; a<(INT)p_CookedFiles->m_ItemCount; a++)
				{
					GridItemData* d = GetItemData(a);
					if (d->Hdr.Valid)
						if (d->Column==col)
						{
							Item = a;
						}
						else
						{
							break;
						}
				}

			break;
		}

		if (Item!=m_FocusItem)
		{
			m_ShowFocusRect = TRUE;
			SetFocusItem(Item, GetKeyState(VK_SHIFT)<0);

			CPoint pt;
			GetCursorPos(&pt);
			ScreenToClient(&pt);
			OnMouseMove(0, pt);
		}
	}
}


BEGIN_MESSAGE_MAP(CGridView, CFileView)
	ON_WM_PAINT()
	ON_WM_KEYDOWN()
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

	CBitmap MemBitmap;
	MemBitmap.CreateCompatibleBitmap(&pDC, rect.Width(), rect.Height());
	CBitmap* pOldBitmap = dc.SelectObject(&MemBitmap);

	BOOL Themed = IsCtrlThemed();
	COLORREF bkCol = Themed ? 0xFFFFFF : GetSysColor(COLOR_WINDOW);
	dc.FillSolidRect(rect, bkCol);

	if (m_HeaderHeight>0)
		if (Themed)
		{
			CGdiPlusBitmap* pDivider = theApp.GetCachedResourceImage(IDB_DIVUP, _T("PNG"));

			Graphics g(dc);
			g.DrawImage(pDivider->m_pBitmap, (rect.Width()-(INT)pDivider->m_pBitmap->GetWidth())/2+GetScrollPos(SB_HORZ), m_HeaderHeight-(INT)pDivider->m_pBitmap->GetHeight());
		}
		else
		{
			dc.FillSolidRect(0, 0, rect.Width(), m_HeaderHeight, GetSysColor(COLOR_3DFACE));
		}

	CFont* pOldFont = dc.SelectObject(&theApp.m_DefaultFont);

	if (m_Nothing)
	{
		CRect rectText(rect);
		rectText.top += m_HeaderHeight+6;

		CString tmpStr((LPCSTR)IDS_NOTHINGTODISPLAY);

		dc.SetTextColor(Themed ? 0xBFB0A6 : GetSysColor(COLOR_3DFACE));
		dc.DrawText(tmpStr, rectText, DT_CENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
	}
	else
		if (p_CookedFiles)
			if (p_CookedFiles->m_ItemCount)
			{
				RECT rectIntersect;

				for (UINT a=0; a<p_CookedFiles->m_ItemCount; a++)
				{
					GridItemData* d = GetItemData(a);
					if (d->Hdr.Valid)
					{
						CRect rect(d->Hdr.Rect);
						rect.OffsetRect(-m_HScrollPos, -m_VScrollPos+(INT)m_HeaderHeight);
						if (IntersectRect(&rectIntersect, rect, rectUpdate))
						{
							DrawItemBackground(dc, rect, a, Themed);
							DrawItem(dc, rect, a, Themed);
							DrawItemForeground(dc, rect, a, Themed);
						}
					}
				}

				if (m_HasCategories)
					for (UINT a=0; a<m_Categories.m_ItemCount; a++)
					{
						CRect rect(m_Categories.m_Items[a].Rect);
						rect.OffsetRect(-m_HScrollPos, -m_VScrollPos+(INT)m_HeaderHeight);
						if (IntersectRect(&rectIntersect, rect, rectUpdate))
							DrawCategory(dc, rect, m_Categories.m_Items[a].Caption, m_Categories.m_Items[a].Hint, Themed);
					}
			}

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldFont);
	dc.SelectObject(pOldBitmap);
}

void CGridView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CFileView::OnKeyDown(nChar, nRepCnt, nFlags);

	switch (m_GridArrange)
	{
	case GRIDARRANGE_HORIZONTAL:
		HandleHorizontalKeys(nChar, nRepCnt, nFlags);
		break;

	case GRIDARRANGE_VERTICAL:
		HandleVerticalKeys(nChar, nRepCnt, nFlags);
		break;
	}
}
