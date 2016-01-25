
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
	GetWindowRect(rectWindow);

	if (!rectWindow.Width())
		return;

	const INT cx = gva.cx+2*gva.padding;
	const INT cy = gva.cy+2*gva.padding;
	ASSERT(cx>0);
	ASSERT(cy>0);
	m_RowHeight = cy+gva.guttery;

	BOOL HasScrollbars = FALSE;

Restart:
	m_ScrollWidth = m_ScrollHeight = 0;

	INT Column = 0;
	INT Row = 0;
	INT x = gva.mx;
	INT y = gva.my;

	INT Category = -1;

	INT gutter = 0;
	if ((Justify) && (!ForceBreak))
	{
		INT Width = (rectWindow.Width()-gva.mx-gva.gutterx);
		INT cColumns = Width/(cx+gva.gutterx);
		gutter = cColumns>1 ? (Width-cx-gva.gutterx)/(cColumns-1)-(cx+gva.gutterx) : 0;
	}

	for (INT a=0; a<(INT)p_CookedFiles->m_ItemCount; a++)
	{
		if (m_HasCategories)
			if ((INT)p_CookedFiles->m_Items[a]->CategoryID!=Category)
			{
				if (x>gva.mx)
				{
					Column = 0;
					Row++;
					x = gva.mx;
					y += cy+gva.guttery;
				}
				if (y>gva.my)
					y += 8;

				Category = p_CookedFiles->m_Items[a]->CategoryID;

				const LPRECT lpRect = &m_Categories.m_Items[Category].Rect;

				lpRect->left = lpRect->right = x;
				lpRect->top = y;
				lpRect->bottom = lpRect->top+2*LFCategoryPadding+theApp.m_LargeFont.GetFontHeight();

				if (m_Categories.m_Items[Category].Hint[0]!=L'\0')
					lpRect->bottom += theApp.m_DefaultFont.GetFontHeight();;

				y = lpRect->bottom+4;
			}

		GridItemData* pData = GetItemData(a);
		pData->Column = Column++;
		pData->Row = Row;
		pData->Hdr.Rect.left = x;
		pData->Hdr.Rect.top = y;
		pData->Hdr.Rect.right = x+cx;
		pData->Hdr.Rect.bottom = y+cy;

		x += cx+gva.gutterx;
		if (x>m_ScrollWidth)
			m_ScrollWidth = x-1;

		if (y+cy+gva.guttery-1>m_ScrollHeight)
		{
			m_ScrollHeight = y+cy+max(gva.guttery, 0);
			if ((m_ScrollHeight>rectWindow.Height()) && (!HasScrollbars))
			{
				HasScrollbars = TRUE;
				rectWindow.right -= GetSystemMetrics(SM_CXVSCROLL);
				goto Restart;
			}
		}

		if ((x+cx+gva.gutterx>rectWindow.Width()) || (ForceBreak))
		{
			if (MaxWidth)
				pData->Hdr.Rect.right = rectWindow.Width()-gva.gutterx;

			Column = 0;
			Row++;
			x = gva.mx;
			y += cy+gva.guttery;
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
	GetWindowRect(rectWindow);

	if (!rectWindow.Width())
		return;

	INT Top = gva.my;
	if (m_HasCategories)
		Top += 2*LFCategoryPadding+theApp.m_LargeFont.GetFontHeight();+4;

	const INT cx = gva.cx+2*gva.padding;
	const INT cy = gva.cy+2*gva.padding;
	ASSERT(cx>0);
	ASSERT(cy>0);
	m_RowHeight = cy+gva.guttery;

	BOOL HasScrollbars = FALSE;

Restart:
	m_ScrollWidth = m_ScrollHeight = 0;

	INT Column = 0;
	INT Row = 0;
	INT x = gva.mx;
	INT y = Top;

	INT Category = -1;
	INT LastLeft = x;
#define FINISHCATEGORY if (Category!=-1) { m_Categories.m_Items[Category].Rect.right = LastLeft+cx; }

	for (INT a=0; a<(INT)p_CookedFiles->m_ItemCount; a++)
	{
		if (m_HasCategories)
			if ((INT)p_CookedFiles->m_Items[a]->CategoryID!=Category)
			{
				FINISHCATEGORY;

				if (y>Top)
				{
					Row = 0;
					Column++;
					y = Top;
					x += cx+gva.gutterx;
				}
				if (x>gva.mx)
					x += 8;

				Category = p_CookedFiles->m_Items[a]->CategoryID;

				const LPRECT lpRect = &m_Categories.m_Items[Category].Rect;
				lpRect->left = x;
				lpRect->top = gva.my;
				lpRect->bottom = lpRect->top+2*LFCategoryPadding+theApp.m_LargeFont.GetFontHeight();
			}

		GridItemData* pData = GetItemData(a);
		pData->Column = Column;
		pData->Row = Row++;
		pData->Hdr.Rect.left = LastLeft = x;
		pData->Hdr.Rect.top = y;
		pData->Hdr.Rect.right = x+cx;
		pData->Hdr.Rect.bottom = y+cy;

		if (a==(INT)p_CookedFiles->m_ItemCount-1)
			FINISHCATEGORY;

		y += cy+gva.guttery;
		if (y>m_ScrollHeight)
			m_ScrollHeight = y-1;

		if (x+cx+gva.gutterx-1>m_ScrollWidth)
		{
			m_ScrollWidth = x+cx+max(gva.gutterx, 0);
			if ((m_ScrollWidth>rectWindow.Width()) && (!HasScrollbars))
			{
				HasScrollbars = TRUE;
				rectWindow.bottom -= GetSystemMetrics(SM_CXHSCROLL);
				goto Restart;
			}
		}

		if (y+cy+gva.guttery>rectWindow.Height())
		{
			Row = 0;
			Column++;
			y = Top;
			x += cx+gva.gutterx;
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
		GetClientRect(rect);

		INT Item = m_FocusItem;
		INT Column = (Item==-1) ? 0 : GetItemData(Item)->Column;
		INT Row = (Item==-1) ? 0 : GetItemData(Item)->Row;
		INT top = (Item==-1) ? 0 : GetItemData(Item)->Hdr.Rect.top;
		INT bottom = (Item==-1) ? 0 : GetItemData(Item)->Hdr.Rect.bottom;
		INT tmprow = -1;

		switch (nChar)
		{
		case VK_LEFT:
			for (INT a=Item-1; a>=0; a--)
			{
				GridItemData* pData = GetItemData(a);
				if ((pData->Row==Row) && pData->Hdr.Valid)
				{
					Item = a;
					break;
				}
			}

			break;

		case VK_RIGHT:
			for (INT a=Item+1; a<(INT)p_CookedFiles->m_ItemCount; a++)
			{
				GridItemData* pData = GetItemData(a);
				if ((pData->Row==Row) && pData->Hdr.Valid)
				{
					Item = a;
					break;
				}
			}

			break;

		case VK_UP:
			for (INT a=Item-1; a>=0; a--)
			{
				GridItemData* pData = GetItemData(a);
				if ((pData->Row<Row) && pData->Hdr.Valid)
				{
					Item = a;
					if (pData->Column<=Column)
						break;
				}
				if (pData->Row<Row-1)
					break;
			}

			break;

		case VK_PRIOR:
			for (INT a=Item-1; a>=0; a--)
			{
				GridItemData* pData = GetItemData(a);
				if ((pData->Row<=Row) && (pData->Column<=Column) && pData->Hdr.Valid)
				{
					Item = a;
					if (pData->Hdr.Rect.top<=bottom-rect.Height()+(INT)m_HeaderHeight)
						break;
				}
			}

			break;

		case VK_DOWN:
			for (INT a=Item+1; a<(INT)p_CookedFiles->m_ItemCount; a++)
			{
				GridItemData* pData = GetItemData(a);
				if ((pData->Row>Row) && pData->Hdr.Valid)
				{
					Item = a;
					if (pData->Column>=Column)
						break;
				}
				if (pData->Row>Row+1)
					break;
			}

			break;

		case VK_NEXT:
			for (INT a=Item+1; a<(INT)p_CookedFiles->m_ItemCount; a++)
			{
				GridItemData* pData = GetItemData(a);
				if (pData->Row!=GetItemData(a-1)->Row)
					if (pData->Hdr.Rect.bottom>=top+rect.Height()-(INT)m_HeaderHeight)
						if (tmprow==-1)
						{
							tmprow = pData->Row;
						}
						else
						{
							if (pData->Row>tmprow)
								break;
						}
				if (((tmprow==-1) || (pData->Column<=Column)) && pData->Hdr.Valid)
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
					GridItemData* pData = GetItemData(a);
					if (pData->Hdr.Valid)
						if (pData->Row==Row)
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
					GridItemData* pData = GetItemData(a);
					if (pData->Hdr.Valid)
						if (pData->Row==Row)
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
		GetClientRect(rect);

		INT Item = m_FocusItem;
		INT Column = (Item==-1) ? 0 : GetItemData(Item)->Column;
		INT Row = (Item==-1) ? 0 : GetItemData(Item)->Row;
		INT left = (Item==-1) ? 0 : GetItemData(Item)->Hdr.Rect.left;
		INT right = (Item==-1) ? 0 : GetItemData(Item)->Hdr.Rect.right;
		INT tmpcol = -1;

		switch (nChar)
		{
		case VK_LEFT:
			for (INT a=Item-1; a>=0; a--)
			{
				GridItemData* pData = GetItemData(a);
				if ((pData->Column<Column) && pData->Hdr.Valid)
				{
					Item = a;
					if (pData->Row<=Row)
						break;
				}
				if (pData->Column<Column-1)
					break;
			}

			break;

		case VK_PRIOR:
			for (INT a=Item-1; a>=0; a--)
			{
				GridItemData* pData = GetItemData(a);
				if ((pData->Column<=Column) && (pData->Row<=Row) && pData->Hdr.Valid)
				{
					Item = a;
					if (pData->Hdr.Rect.left<=right-rect.Width())
						break;
				}
			}

			break;

		case VK_RIGHT:
			for (INT a=Item+1; a<(INT)p_CookedFiles->m_ItemCount; a++)
			{
				GridItemData* pData = GetItemData(a);
				if ((pData->Column>Column) && pData->Hdr.Valid)
				{
					Item = a;
					if (pData->Row>=Row)
						break;
				}
				if (pData->Column>Column+1)
					break;
			}

			break;

		case VK_NEXT:
			for (INT a=Item+1; a<(INT)p_CookedFiles->m_ItemCount; a++)
			{
				GridItemData* pData = GetItemData(a);
				if (pData->Column!=GetItemData(a-1)->Column)
					if (pData->Hdr.Rect.right>=left+rect.Width())
						if (tmpcol==-1)
						{
							tmpcol = pData->Column;
						}
						else
						{
							if (pData->Column>tmpcol)
								break;
						}
				if (((tmpcol==-1) || (pData->Row<=Row)) && pData->Hdr.Valid)
					Item = a;
			}

			break;

		case VK_UP:
			for (INT a=Item-1; a>=0; a--)
			{
				GridItemData* pData = GetItemData(a);
				if ((pData->Column==Column) && pData->Hdr.Valid)
				{
					Item = a;
					break;
				}
			}

			break;

		case VK_DOWN:
			for (INT a=Item+1; a<(INT)p_CookedFiles->m_ItemCount; a++)
			{
				GridItemData* pData = GetItemData(a);
				if ((pData->Column==Column) && pData->Hdr.Valid)
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
					GridItemData* pData = GetItemData(a);
					if (pData->Hdr.Valid)
						if (pData->Column==Column)
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
					GridItemData* pData = GetItemData(a);
					if (pData->Hdr.Valid)
						if (pData->Column==Column)
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

	dc.FillSolidRect(rect, Themed ? 0xFFFFFF : GetSysColor(COLOR_WINDOW));

	if (m_HeaderHeight>0)
		if (Themed)
		{
			Bitmap* pDivider = theApp.GetCachedResourceImage(IDB_DIVUP);

			Graphics g(dc);
			g.DrawImage(pDivider, (rect.Width()-(INT)pDivider->GetWidth())/2+GetScrollPos(SB_HORZ), m_HeaderHeight-(INT)pDivider->GetHeight());
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

		dc.SetTextColor(Themed ? 0xBFB0A6 : GetSysColor(COLOR_3DSHADOW));
		dc.DrawText(tmpStr, rectText, DT_CENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
	}
	else
		if (p_CookedFiles)
			if (p_CookedFiles->m_ItemCount)
			{
				RECT rectIntersect;

				for (UINT a=0; a<p_CookedFiles->m_ItemCount; a++)
				{
					GridItemData* pData = GetItemData(a);
					if (pData->Hdr.Valid)
					{
						CRect rect(pData->Hdr.Rect);
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

	DrawWindowEdge(dc, Themed);

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
