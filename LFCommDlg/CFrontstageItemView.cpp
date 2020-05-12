
// CFrontstageItemView.cpp: Implementierung der Klasse CFrontstageItemView
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CFrontstageItemView
//

CFrontstageItemView::CFrontstageItemView(UINT Flags, SIZE_T szData, const CSize& szItemInflate)
	: CFrontstageScroller(Flags)
{
	ASSERT(((Flags & FRONTSTAGE_ENABLESELECTION)==0) || (Flags & FRONTSTAGE_ENABLEFOCUSITEM));
	ASSERT(((Flags & FRONTSTAGE_ENABLESHIFTSELECTION)==0) || (Flags & FRONTSTAGE_ENABLESELECTION));
	ASSERT(((Flags & FRONTSTAGE_ENABLEDRAGANDDROP)==0) || (Flags & FRONTSTAGE_ENABLEFOCUSITEM));
	ASSERT(((Flags & FRONTSTAGE_ENABLELABELEDIT)==0) || (Flags & FRONTSTAGE_ENABLEFOCUSITEM));
	ASSERT(((Flags & FRONTSTAGE_ENABLEEDITONHOVER)==0) || (Flags & FRONTSTAGE_ENABLELABELEDIT));
	ASSERT(((Flags & FRONTSTAGE_HIDESELECTIONONEDIT)==0) || (Flags & FRONTSTAGE_ENABLELABELEDIT));
	ASSERT(szData>=sizeof(ItemData));

	m_szData = szData;
	m_pItemData = NULL;
	m_szItemInflate = szItemInflate;

	// Font height
	m_LargeFontHeight = LFGetApp()->m_LargeFont.GetFontHeight();
	m_DefaultFontHeight = LFGetApp()->m_DefaultFont.GetFontHeight();
	m_SmallFontHeight = LFGetApp()->m_SmallFont.GetFontHeight();

	// Items
	m_ItemDataAllocated = m_ItemCount = m_IconSize = 0;
	m_Nothing = TRUE;

	m_FocusItem = m_EditItem = m_SelectionAnchor = -1;
	m_FocusItemSelected = m_MultipleSelected = m_ShowFocusRect = m_DrawItemSeparator = m_ButtonDownInWindow = FALSE;

	ResetDragLocation();
	ZeroMemory(&m_Bitmaps, sizeof(m_Bitmaps));
}

BOOL CFrontstageItemView::Create(CWnd* pParentWnd, UINT nID, const CRect& rect, UINT nClassStyle)
{
	const CString className = AfxRegisterWndClass(nClassStyle | CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, LFGetApp()->LoadStandardCursor(IDC_ARROW));

	return CFrontstageScroller::Create(className, _T(""), WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_TABSTOP, rect, pParentWnd, nID);
}

LRESULT CFrontstageItemView::SendNotifyMessage(UINT Code) const
{
	const NMHDR nmHdr = { m_hWnd, GetDlgCtrlID(), Code };

	return GetOwner()->SendMessage(WM_NOTIFY, nmHdr.idFrom, (LPARAM)&nmHdr);
}


// Scrolling

void CFrontstageItemView::SetItemHeight(INT IconSize, INT Rows, INT Padding)
{
	ASSERT(Rows>=1);
	ASSERT(Padding>=0);

	CFrontstageScroller::SetItemHeight(max(m_IconSize=IconSize, Rows*m_DefaultFontHeight)+2*Padding);
}

void CFrontstageItemView::SetItemHeight(const CImageList& ImageList, INT Rows, INT Padding)
{
	ASSERT(Rows>=1);
	ASSERT(Padding>=0);

	INT cx;
	INT cy;
	ImageList_GetIconSize(ImageList, &cx, &cy);

	SetItemHeight(cy, Rows, Padding);
}

void CFrontstageItemView::EnsureVisible(INT Index)
{
	if (IsScrollingEnabled() && (Index>=0) && (Index<m_ItemCount))
		CFrontstageScroller::EnsureVisible(GetItemRect(Index));
}

void CFrontstageItemView::AdjustScrollbars()
{
	if (IsScrollingEnabled())
		CFrontstageScroller::AdjustScrollbars();
}


// Layouts

void CFrontstageItemView::GetLayoutRect(CRect& rectLayout)
{
	// Reset item categories
	ResetItemCategories();

	CFrontstageScroller::GetLayoutRect(rectLayout);
}

void CFrontstageItemView::AdjustLayoutGrid(const CSize& szItem, BOOL FullWidth, INT Margin, INT TopOffset, BOOL AllowItemSeparator)
{
	// Item separator
	m_DrawItemSeparator = AllowItemSeparator && FullWidth;

	// Layout rect
	CRect rectLayout;
	GetLayoutRect(rectLayout);

	if (!rectLayout.Width())
		return;

	// Scroll area
	if (HasBorder())
		Margin = ITEMCELLPADDINGY;

	ASSERT(FullWidth || (szItem.cx>0));
	ASSERT(szItem.cy>0);

	CSize szGutter;
	szGutter.cx = GetGutterForMargin(Margin);
	szGutter.cy = FullWidth ? -1 : szGutter.cx;

	if (!TopOffset)
		TopOffset = m_HeaderHeight ? 1 : Margin;

	// Items
	m_szScrollStep.cy = (m_ItemHeight=szItem.cy)+szGutter.cy;

	BOOL HasScrollbars = FALSE;

Restart:
	m_ScrollWidth = m_ScrollHeight = 0;

	INT Column = 0;
	INT Row = 0;
	INT x = Margin;
	INT y = TopOffset;

	INT Category = -1;

	for (INT Index=0; Index<m_ItemCount; Index++)
	{
		ItemData* pData = GetItemData(Index);

		if (pData->Valid)
		{
			if (GetItemCategory(Index)!=Category)
			{
				if (x>Margin)
				{
					Column = 0;
					Row++;

					x = Margin;
					y += szItem.cy+szGutter.cy;
				}

				if (y>TopOffset)
					y += 8;

				Category = GetItemCategory(Index);
				ASSERT(Category<(INT)m_ItemCategories.m_ItemCount);

				const LPRECT lpRect = &m_ItemCategories[Category].Rect;
				lpRect->left = lpRect->right = x;
				lpRect->bottom = (lpRect->top=y)+2*LFCATEGORYPADDING+m_LargeFontHeight;

				if (m_ItemCategories[Category].Hint[0]!=L'\0')
					lpRect->bottom += m_DefaultFontHeight;

				y = lpRect->bottom+4;
			}

			pData->Column = Column++;
			pData->Row = Row;
			pData->Rect.right = (pData->Rect.left=x)+szItem.cx;
			pData->Rect.bottom = (pData->Rect.top=y)+szItem.cy;

			if ((x+=szItem.cx+szGutter.cx)>m_ScrollWidth)
				m_ScrollWidth = x-1;

			if (y+szItem.cy+szGutter.cy-1>m_ScrollHeight)
				if (((m_ScrollHeight=y+szItem.cy+Margin)>rectLayout.Height()) && !HasScrollbars)
				{
					rectLayout.right -= GetSystemMetrics(SM_CXVSCROLL);
					HasScrollbars = TRUE;

					goto Restart;
				}

			if ((x+szItem.cx+szGutter.cx>rectLayout.Width()) || FullWidth)
			{
				if (FullWidth)
					pData->Rect.right = rectLayout.Width()-Margin;

				Column = 0;
				Row++;

				x = Margin;
				y += szItem.cy+szGutter.cy;
			}
		}
	}

	// Adjust categories to calculated width
	for (UINT a=0; a<m_ItemCategories.m_ItemCount; a++)
		if (m_ItemCategories[a].Rect.right)
			m_ItemCategories[a].Rect.right = max(m_ScrollWidth, rectLayout.Width())-Margin;

	CFrontstageScroller::AdjustLayout();
}

void CFrontstageItemView::AdjustLayoutList()
{
	ASSERT(HasHeader());

	// Item separator
	m_DrawItemSeparator = TRUE;

	// Layout rect
	CRect rectLayout;
	GetLayoutRect(rectLayout);

	if (!rectLayout.Width())
		return;

	// Scroll area
	m_ScrollWidth = rectLayout.Width()-GetSystemMetrics(SM_CXVSCROLL);

	// Items
	m_szScrollStep.cy = m_ItemHeight-1;
	m_ScrollHeight = 2;

	for (INT Index=0; Index<m_ItemCount; Index++)
	{
		ItemData* pData = GetItemData(Index);

		if (pData->Valid)
		{
			pData->Row = Index;
			pData->Rect.left= GetHeaderIndent()+1;
			pData->Rect.right = m_ScrollWidth;
			pData->Rect.bottom = (pData->Rect.top=m_ScrollHeight-1)+m_ItemHeight;

			m_ScrollHeight = pData->Rect.bottom;
		}
	}

	CFrontstageScroller::AdjustLayout();
}

void CFrontstageItemView::AdjustLayoutColumns(INT Columns, INT Margin)
{
	ASSERT(Columns>=1);

	// Layout rect
	CRect rectLayout;
	GetLayoutRect(rectLayout);

	if (!rectLayout.Width())
		return;

	// Scroll area
	if (HasBorder())
		Margin = ITEMCELLPADDINGY;

	const INT Width = (rectLayout.Width()-GetSystemMetrics(SM_CXVSCROLL)-Margin)/Columns-GetGutterForMargin(Margin);

	AdjustLayoutGrid(CSize(max(Width, ITEMVIEWMINWIDTH), m_ItemHeight), Columns==1, Margin);
}

void CFrontstageItemView::AdjustLayoutSingleRow(INT Columns, INT Margin)
{
	ASSERT(Columns>=1);

	// Item separator
	m_DrawItemSeparator = FALSE;

	// Layout rect
	CRect rectLayout;
	GetLayoutRect(rectLayout);

	if (!rectLayout.Width())
		return;

	// Scroll area
	if (HasBorder())
		Margin = ITEMCELLPADDINGY;

	INT Width = (rectLayout.Width()-Margin)/Columns-Margin;
	if (Width<ITEMVIEWMINWIDTH)
		Width = ITEMVIEWMINWIDTH;

	// Items
	m_ItemHeight = (m_ScrollHeight=rectLayout.Height()-GetSystemMetrics(SM_CYHSCROLL))-2*Margin;
	m_ScrollWidth = Margin;

	INT Column = 0;
	INT x = Margin;

	for (INT Index=0; Index<m_ItemCount; Index++)
	{
		ItemData* pItemData = GetItemData(Index);

		if (pItemData->Valid)
		{
			pItemData->Column = Column++;
			pItemData->Rect.right = (pItemData->Rect.left=x)+Width;
			pItemData->Rect.bottom = (pItemData->Rect.top=Margin)+m_ItemHeight;

			m_ScrollWidth = x+=(Width+Margin);
		}
	}

	CFrontstageScroller::AdjustLayout();
}


// Item categories

void CFrontstageItemView::AddItemCategory(LPCWSTR Caption, LPCWSTR Hint, INT IconID)
{
	ASSERT(Caption);
	ASSERT(Caption[0]);
	ASSERT(Hint);
	ASSERT(IconID>=0);

	ItemCategoryData Data;

	ZeroMemory(&Data.Rect, sizeof(RECT));
	wcscpy_s(Data.Caption, 256, Caption);
	wcscpy_s(Data.Hint, 256, Hint);
	Data.IconID = IconID;

	m_ItemCategories.AddItem(Data);
}

INT CFrontstageItemView::GetItemCategory(INT /*Index*/) const
{
	return -1;
}


// Item data

void CFrontstageItemView::SetItemCount(UINT ItemCount, BOOL Virtual)
{
	// Free old item data
	FreeItemData(TRUE);

	// Allocate new item data
	const SIZE_T Size = ((SIZE_T)(m_ItemDataAllocated=ItemCount))*m_szData;
	ZeroMemory(m_pItemData=(LPBYTE)malloc(Size), Size);

	if (Virtual)
	{
		// Virtual data, so set m_ItemCount to ItemCount paramter
		// Otherwise, items will be added using AddItem() calls
		m_ItemCount = ItemCount;

		// Focus item
		LastItem();
	}

	m_MultipleSelected = IsSelectionEnabled();
	m_Nothing = TRUE;
}

void CFrontstageItemView::AddItem(LPCVOID pData)
{
	ASSERT((UINT)m_ItemCount<m_ItemDataAllocated);
	ASSERT(offsetof(ItemData, Rect)==0);

	ItemData* pItemData = GetItemData(m_ItemCount++);

	ZeroMemory(pItemData, sizeof(ItemData));
	pItemData->Valid = TRUE;

	if (m_szData>sizeof(ItemData))
		memcpy(((LPBYTE)pItemData)+sizeof(ItemData), ((LPBYTE)pData)+sizeof(ItemData), m_szData-sizeof(ItemData));

	m_Nothing = FALSE;
}

void CFrontstageItemView::LastItem()
{
	if (m_FocusItem>=m_ItemCount)
		m_FocusItem = m_ItemCount-1;
}

void CFrontstageItemView::ValidateAllItems()
{
	for (INT Index=0; Index<m_ItemCount; Index++)
		GetItemData(Index)->Valid = TRUE;

	m_Nothing = (m_ItemCount<=0);
}

void CFrontstageItemView::FreeItemData(BOOL InternalCall)
{
	free(m_pItemData);

	m_pItemData = NULL;
	m_ItemDataAllocated = m_ItemCount = 0;
	m_ShowFocusRect = FALSE;

	if (!InternalCall)
	{
		m_Nothing = TRUE;
		m_FocusItem = -1;
	}
}


// Item sort

void CFrontstageItemView::SortItems(PFNCOMPARE zCompare, UINT Attr, BOOL Descending, BOOL Parameter1, BOOL Parameter2)
{
	LFSortMemory(m_pItemData, m_ItemCount, m_szData, zCompare, Attr, Descending, Parameter1, Parameter2);
}


// Item handling

RECT CFrontstageItemView::GetItemRect(INT Index) const
{
	ASSERT(Index>=0);
	ASSERT(Index<m_ItemCount);

	RECT rect = GetItemData(Index)->Rect;
	OffsetRect(&rect, -m_HScrollPos, -m_VScrollPos+(INT)m_HeaderHeight);

	return rect;
}

INT CFrontstageItemView::ItemAtPosition(CPoint point) const
{
	point.Offset(m_HScrollPos, m_VScrollPos-m_HeaderHeight);

	for (INT Index=0; Index<m_ItemCount; Index++)
	{
		const ItemData* pData = GetItemData(Index);

		if (pData->Valid && PtInRect(&pData->Rect, point))
			return Index;
	}

	return -1;
}

void CFrontstageItemView::InvalidateItem(INT Index)
{
	if ((Index>=0) && (Index<m_ItemCount))
	{
		RECT rect = GetItemRect(Index);
		InflateRect(&rect, m_szItemInflate.cx, m_szItemInflate.cy);

		InvalidateRect(&rect);
	}
}

COLORREF CFrontstageItemView::GetItemTextColor(INT /*Index*/, BOOL /*Themed*/) const
{
	return (COLORREF)-1;
}

void CFrontstageItemView::SetFocusItem(INT Index, UINT Mode)
{
	m_EditItem = -1;

	if (IsFocusItemEnabled())
	{
		if (!IsSelectionEnabled())
			Mode = SETFOCUSITEM_MOVEDESELECT;
		
		switch (Mode)
		{
		case SETFOCUSITEM_MOVEONLY:
			m_FocusItem = Index;

			break;

		case SETFOCUSITEM_MOVEDESELECT:
			m_FocusItemSelected = TRUE;

			if (IsSelectionEnabled() && m_MultipleSelected)
			{
				m_SelectionAnchor = -1;

				for (INT a=0; a<m_ItemCount; a++)
					SelectItem(a, a==Index);

				m_FocusItem = Index;
				ItemSelectionChanged();
			}
			else
			{
				if (m_FocusItem>=0)
				{
					SelectItem(m_FocusItem, FALSE);
					InvalidateItem(m_FocusItem);
				}

				SelectItem(Index);
				ItemSelectionChanged(m_FocusItem=Index);
			}

			m_MultipleSelected = FALSE;

			break;

		case SETFOCUSITEM_MOVETOGGLESELECT:
			m_SelectionAnchor = -1;
			m_MultipleSelected = TRUE;

			SelectItem(Index, m_FocusItemSelected=!IsItemSelected(Index));
			ItemSelectionChanged(m_FocusItem=Index);

			break;

		case SETFOCUSITEM_SHIFTSELECT:
			if (m_SelectionAnchor==-1)
				m_SelectionAnchor = m_FocusItem;

			m_FocusItemSelected = m_MultipleSelected = TRUE;

			for (INT a=0; a<m_ItemCount; a++)
				SelectItem(a, ((a>=Index) && (a<=m_SelectionAnchor)) || ((a>=m_SelectionAnchor) && (a<=Index)));

			m_FocusItem = Index;
			ItemSelectionChanged();

			break;
		}

		ASSERT(m_FocusItem==Index);
		EnsureVisible(m_FocusItem);
	}
}

INT CFrontstageItemView::HandleNavigationKeys(UINT nChar, BOOL Control) const
{
	CRect rect;
	GetClientRect(rect);

	INT Item = m_FocusItem;
	const ItemData* pData = (Item==-1) ? NULL : GetItemData(Item);
	const INT Top = pData ? pData->Rect.top : 0;
	const INT Bottom = pData ? pData->Rect.bottom : 0;
	const INT Column = pData ? pData->Column : 0;
	const INT Row = pData ? pData->Row : 0;
	INT TmpRow = -1;

	switch (nChar)
	{
	case VK_LEFT:
		for (INT Index=Item-1; Index>=0; Index--)
		{
			const ItemData* pItemData = GetItemData(Index);
			if ((pItemData->Row==Row) && pItemData->Valid)
			{
				Item = Index;
				break;
			}
		}

		break;

	case VK_RIGHT:
		for (INT Index=Item+1; Index<m_ItemCount; Index++)
		{
			const ItemData* pItemData = GetItemData(Index);
			if ((pItemData->Row==Row) && pItemData->Valid)
			{
				Item = Index;
				break;
			}
		}

		break;

	case VK_UP:
		for (INT Index=Item-1; Index>=0; Index--)
		{
			const ItemData* pItemData = GetItemData(Index);
			if (pItemData->Valid)
			{
				if ((Item!=m_FocusItem) && (pItemData->Row<Row-1))
					break;

				if (pItemData->Row<Row)
				{
					Item = Index;

					if (pItemData->Column<=Column)
						break;
				}
			}
		}

		break;

	case VK_PRIOR:
		for (INT Index=Item-1; Index>=0; Index--)
		{
			const ItemData* pItemData = GetItemData(Index);
			if ((pItemData->Row<=Row) && (pItemData->Column<=Column) && pItemData->Valid)
			{
				Item = Index;

				if (pItemData->Rect.top<=Bottom-rect.Height()+(INT)m_HeaderHeight)
					break;
			}
		}

		break;

	case VK_DOWN:
		for (INT Index=Item+1; Index<m_ItemCount; Index++)
		{
			const ItemData* pItemData = GetItemData(Index);
			if (pItemData->Valid)
			{
				if ((Item!=m_FocusItem) && (pItemData->Row>Row+1))
					break;

				if (pItemData->Row>Row)
				{
					Item = Index;

					if (pItemData->Column>=Column)
						break;
				}

			}
		}

		break;

	case VK_NEXT:
		for (INT Index=Item+1; Index<m_ItemCount; Index++)
		{
			const ItemData* pItemData = GetItemData(Index);
			if (pItemData->Row!=GetItemData(Index-1)->Row)
				if (pItemData->Rect.bottom>=Top+rect.Height()-(INT)m_HeaderHeight)
					if (TmpRow==-1)
					{
						TmpRow = pItemData->Row;
					}
					else
					{
						if (pItemData->Row>TmpRow)
							break;
					}

			if (((TmpRow==-1) || (pItemData->Column<=Column)) && pItemData->Valid)
				Item = Index;
		}

		break;

	case VK_HOME:
		if (Control)
		{
			for (INT Index=0; Index<m_ItemCount; Index++)
				if (GetItemData(Index)->Valid)
				{
					Item = Index;
					break;
				}
		}
		else
			for (INT Index=Item-1; Index>=0; Index--)
			{
				const ItemData* pItemData = GetItemData(Index);
				if (pItemData->Valid)
					if (pItemData->Row==Row)
					{
						Item = Index;
					}
					else
					{
						break;
					}
			}

		break;

	case VK_END:
		if (Control)
		{
			for (INT Index=m_ItemCount-1; Index>=0; Index--)
				if (GetItemData(Index)->Valid)
				{
					Item = Index;
					break;
				}
		}
		else
			for (INT Index=Item+1; Index<m_ItemCount; Index++)
			{
				const ItemData* pItemData = GetItemData(Index);
				if (pItemData->Valid)
					if (pItemData->Row==Row)
					{
						Item = Index;
					}
					else
					{
						break;
					}
			}

		break;
	}

	return Item;
}


// Item selection

BOOL CFrontstageItemView::IsItemSelected(INT Index) const
{
	return (Index==m_FocusItem) && m_FocusItemSelected;
}

void CFrontstageItemView::SelectItem(INT /*Index*/, BOOL /*Select*/)
{
}

BOOL CFrontstageItemView::HasItemsSelected() const
{
	for (INT Index=0; Index<m_ItemCount; Index++)
		if (IsItemSelected(Index))
			return TRUE;

	return FALSE;
}

INT CFrontstageItemView::GetSelectedItem() const
{
	ASSERT(IsFocusItemEnabled());

	return ((m_FocusItem>=0) && GetItemData(m_FocusItem)->Valid && IsItemSelected(m_FocusItem)) ? m_FocusItem : -1;
}


// Selected item commands

void CFrontstageItemView::FireSelectedItem()
{
	ASSERT(IsFocusItemEnabled());
	ASSERT(GetSelectedItem()>=0);

	// Only fire focus item if there is no disabled OK button
	CWnd* pWnd = GetOwner()->GetDlgItem(IDOK);
	if (pWnd && !pWnd->IsWindowEnabled())
		return;

	GetOwner()->PostMessage(WM_COMMAND, IDOK);
}

void CFrontstageItemView::DeleteSelectedItem()
{
	ASSERT(IsFocusItemEnabled());
	ASSERT(GetSelectedItem()>=0);
}


// Draw support

void CFrontstageItemView::DrawItemSeparator(CDC& dc, LPCRECT rectItem, BOOL Themed)
{
	ASSERT(rectItem);
	ASSERT(m_DrawItemSeparator);

	if (Themed)
		dc.FillSolidRect(rectItem->left+ITEMCELLPADDINGY, rectItem->bottom-1, rectItem->right-rectItem->left-2*ITEMCELLPADDINGY, 1, 0xF8F6F6);
}

#define DRAWCACHED(nID, DrawOps) \
	CDC MemDC; \
	MemDC.CreateCompatibleDC(&dc); \
	HBITMAP hOldBitmap; \
	const INT Width = rectItem->right-rectItem->left; \
	const INT Height = rectItem->bottom-rectItem->top; \
	if ((m_Bitmaps[nID].Width!=Width) || (m_Bitmaps[nID].Height!=Height)) \
	{ \
		DeleteObject(m_Bitmaps[nID].hBitmap); \
		m_Bitmaps[nID].hBitmap = CreateTransparentBitmap(m_Bitmaps[nID].Width=Width, m_Bitmaps[nID].Height=Height); \
		hOldBitmap = (HBITMAP)MemDC.SelectObject(m_Bitmaps[nID].hBitmap); \
		DrawOps; \
	} \
	else \
	{ \
		hOldBitmap = (HBITMAP)MemDC.SelectObject(m_Bitmaps[nID].hBitmap); \
	} \
	AlphaBlend(dc, rectItem->left, rectItem->top, Width, Height, MemDC, 0, 0, Width, Height, BF); \
	MemDC.SelectObject(hOldBitmap);

void CFrontstageItemView::DrawItemBackground(CDC& dc, LPCRECT rectItem, INT Index, BOOL Themed, BOOL Cached)
{
	ASSERT(rectItem);

	if (IsWindowEnabled())
	{
		const BOOL bDrawHover = DrawHover(Index);
		const BOOL bDrawSelection = DrawSelection(Index);

		if (Cached && bDrawSelection && Themed)
		{
			DRAWCACHED(BITMAP_SELECTION, DrawListItemBackground(MemDC, CRect(0, 0, Width, Height), Themed, GetFocus()==this, bDrawHover, m_FocusItem==Index, bDrawSelection));

			const COLORREF TextColor = GetItemTextColor(Index, Themed);
			dc.SetTextColor(bDrawSelection ? 0xFFFFFF : TextColor!=(COLORREF)-1 ? TextColor : 0x000000);
		}
		else
		{
			DrawListItemBackground(dc, rectItem, Themed, GetFocus()==this,
				bDrawHover, m_FocusItem==Index, bDrawSelection, IsWindowEnabled() ? GetItemTextColor(Index, Themed) : GetSysColor(COLOR_GRAYTEXT), m_ShowFocusRect);
		}
	}
	else
	{
		dc.SetTextColor(GetSysColor(COLOR_GRAYTEXT));
	}
}

void CFrontstageItemView::DrawItemForeground(CDC& dc, LPCRECT rectItem, INT Index, BOOL Themed, BOOL Cached)
{
	ASSERT(rectItem);

	if (((m_HoverItem!=Index) && !DrawSelection(Index)) || !Themed)
		return;

	if (Cached)
	{
		DRAWCACHED(BITMAP_REFLECTION, DrawListItemForeground(MemDC, CRect(0, 0, Width, Height), Themed, GetFocus()==this, m_HoverItem==Index, m_FocusItem==Index, TRUE));
	}
	else
	{
		DrawListItemForeground(dc, rectItem, Themed, GetFocus()==this, m_HoverItem==Index, m_FocusItem==Index, TRUE);
	}
}

COLORREF CFrontstageItemView::GetLightTextColor(CDC& dc, INT Index, BOOL Themed) const
{
	return DrawSelection(Index) ? dc.GetTextColor() : Themed ? 0xA39791 : GetSysColor(COLOR_3DSHADOW);
}

COLORREF CFrontstageItemView::GetDarkTextColor(CDC& dc, INT Index, BOOL Themed) const
{
	return DrawSelection(Index) ? dc.GetTextColor() : Themed ? 0x4C4C4C : GetSysColor(COLOR_WINDOWTEXT);
}


// Drawing

BOOL CFrontstageItemView::DrawNothing() const
{
	return !m_pItemData || !m_ItemCount || m_Nothing;
}

UINT CFrontstageItemView::GetTileRows(UINT Rows, va_list vl)
{
	UINT TileRows = 0;

	for (UINT a=0; a<Rows; a++)
	{
		LPCWSTR pStr = va_arg(vl, LPCWSTR);

		if (pStr && pStr[0])
			TileRows++;
	}

	return TileRows;
}

void CFrontstageItemView::DrawListItem(CDC& dc, CRect rect, INT Index, BOOL Themed, INT* pColumnOrder, INT* pColumnWidth, INT PreviewAttribute)
{
	ASSERT(HasHeader());
	ASSERT(pColumnOrder);
	ASSERT(pColumnWidth);

	rect.right = rect.left-ITEMCELLSPACER+ITEMCELLPADDINGY+1;

	// Columns
	const UINT ColumnCount = GetColumnCount();

	for (UINT a=0; a<ColumnCount; a++)
	{
		const UINT Attr = pColumnOrder[a];

		if (pColumnWidth[Attr] && ((INT)Attr!=PreviewAttribute))
		{
			rect.right = (rect.left=rect.right+ITEMCELLSPACER)+pColumnWidth[Attr]-ITEMCELLSPACER;

			DrawItemCell(dc, rect, Index, Attr, Themed);
		}
	}
}

void CFrontstageItemView::DrawTile(CDC& dc, CRect& rect, COLORREF TextColor, UINT Rows, va_list& vl) const
{
	rect.left += m_IconSize+ITEMVIEWPADDING;
	rect.top += (rect.Height()-GetTileRows(Rows, vl)*m_DefaultFontHeight)/2;

	for (UINT a=0; a<Rows; a++)
	{
		LPCWSTR pStr = va_arg(vl, LPCWSTR);

		if (pStr && pStr[0])
		{
			dc.DrawText(pStr, -1, rect, DT_END_ELLIPSIS | DT_NOPREFIX | DT_LEFT | DT_SINGLELINE);
			rect.top += m_DefaultFontHeight;
		}

		if (a==0)
			dc.SetTextColor(TextColor);
	}
}

void CFrontstageItemView::DrawTile(CDC& dc, CRect rect, CIcons& Icons, INT IconID, COLORREF TextColor, UINT Rows, ...) const
{
	rect.DeflateRect(ITEMVIEWPADDING, ITEMVIEWPADDING);

	// Icon
	Icons.Draw(dc, rect.left, rect.top+(rect.Height()-m_IconSize)/2, IconID);

	// Text rows
	va_list vl;
	va_start(vl, Rows);

	DrawTile(dc, rect, TextColor, Rows, vl);
}

void CFrontstageItemView::DrawTile(CDC& dc, CRect rect, CImageList& ImageList, INT IconID, UINT nStyle, COLORREF TextColor, UINT Rows, ...) const
{
	rect.DeflateRect(ITEMVIEWPADDING, ITEMVIEWPADDING);

	// Icon
	ImageList.DrawEx(&dc, IconID, CPoint(rect.left, rect.top+(rect.Height()-m_IconSize)/2), CSize(m_IconSize, m_IconSize), CLR_NONE, CLR_NONE, nStyle);

	// Text rows
	va_list vl;
	va_start(vl, Rows);

	DrawTile(dc, rect, TextColor, Rows, vl);
}

void CFrontstageItemView::DrawItemCell(CDC& /*dc*/, CRect& /*rectCell*/, INT /*Index*/, UINT /*Attr*/, BOOL /*Themed*/)
{
}

void CFrontstageItemView::DrawItem(CDC& /*dc*/, Graphics& /*g*/, LPCRECT /*rectItem*/, INT /*Index*/, BOOL /*Themed*/)
{
}

void CFrontstageItemView::DrawStage(CDC& dc, Graphics& g, const CRect& /*rect*/, const CRect& rectUpdate, BOOL Themed)
{
	RECT rectIntersect;

	// Items
	for (INT Index=0; Index<m_ItemCount; Index++)
	{
		const ItemData* pData = GetItemData(Index);

		if (pData->Valid)
		{
			CRect rectItem(pData->Rect);
			rectItem.OffsetRect(-m_HScrollPos, -m_VScrollPos+(INT)m_HeaderHeight);

			if (IntersectRect(&rectIntersect, rectItem, rectUpdate))
			{
				if (m_DrawItemSeparator && ((Index<m_ItemCount-1) || (m_VScrollMax==0)))
					DrawItemSeparator(dc, rectItem, Themed);

				DrawItemBackground(dc, rectItem, Index, Themed);
				DrawItem(dc, g, rectItem, Index, Themed);

				if (!HasHeader())
					DrawItemForeground(dc, rectItem, Index, Themed);
			}
		}
	}

	// Categories
	for (UINT a=0; a<m_ItemCategories.m_ItemCount; a++)
	{
		CRect rectCategory(m_ItemCategories[a].Rect);
		rectCategory.OffsetRect(-m_HScrollPos, -m_VScrollPos+(INT)m_HeaderHeight);

		if (IntersectRect(&rectIntersect, rectCategory, rectUpdate))
			DrawCategory(dc, rectCategory, m_ItemCategories[a].Caption, m_ItemCategories[a].Hint, Themed);

		if (m_ItemCategories[a].IconID)
		{
			CRect rectIcon(rectCategory);
			rectIcon.right = (rectIcon.left-=ITEMVIEWICONPADDING)+128;
			rectIcon.bottom = (rectIcon.top=rectIcon.bottom+BACKSTAGEBORDER/2-ITEMVIEWICONPADDING)+128;

			if (IntersectRect(&rectIntersect, rectIcon, rectUpdate))
				LFGetApp()->m_CoreImageListJumbo.Draw(&dc, m_ItemCategories[a].IconID-1, rectIcon.TopLeft(), ILD_NORMAL);
		}
	}
}


// Label edit

BOOL CFrontstageItemView::AllowItemEditLabel(INT /*Index*/) const
{
	return TRUE;
}

RECT CFrontstageItemView::GetLabelRect() const
{
	ASSERT(m_EditItem>=0);
	ASSERT(m_EditItem<m_ItemCount);

	return GetItemRect(m_EditItem);
}

CEdit* CFrontstageItemView::CreateLabelEditControl()
{
	CEdit* pWndEdit = new CEdit();
	pWndEdit->Create(WS_CHILD | WS_VISIBLE | WS_BORDER | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | ES_AUTOHSCROLL, GetLabelRect(), this, 2);

	return pWndEdit;
}

void CFrontstageItemView::EditLabel(INT Index)
{
	ASSERT(!m_pWndEdit);

	if (IsLabelEditEnabled() && !IsEditing() && (Index>=0) && (Index<m_ItemCount) && AllowItemEditLabel(Index))
	{
		HideTooltip();

		EnsureVisible(m_EditItem=Index);
		InvalidateItem(Index);

		ASSERT(CRect(GetLabelRect()).Height()<=GetLabelFont()->GetFontHeight()+4);
		m_pWndEdit = CreateLabelEditControl();

		m_pWndEdit->SetFont(GetLabelFont());
		m_pWndEdit->SetFocus();
		m_pWndEdit->SetSel(0, -1);
	}
	else
	{
		m_EditItem = -1;
	}
}

void CFrontstageItemView::EndLabelEdit(INT /*Index*/, CString& /*Value*/)
{
}

void CFrontstageItemView::DestroyEdit(BOOL Accept)
{
	if (IsEditing())
	{
		const INT EditItem = m_EditItem;

		// Set m_pWndEdit to NULL to avoid recursive calls when the edit window loses focus
		CEdit* pVictim = m_pWndEdit;
		m_pWndEdit = NULL;

		// Get value
		CString Value;
		pVictim->GetWindowText(Value);

		// Destroy window; this will trigger another DestroyEdit() call!
		pVictim->DestroyWindow();
		delete pVictim;

		if (Accept && (EditItem!=-1))
			EndLabelEdit(EditItem, Value);

		if (IsHideSelectionOnEditEnabled())
			InvalidateItem(EditItem);
	}

	if (m_EditItem!=-1)
	{
		m_HoverItem = m_EditItem = -1;
		UpdateHoverItem();
	}

	CFrontstageScroller::DestroyEdit(Accept);
}


BEGIN_MESSAGE_MAP(CFrontstageItemView, CFrontstageScroller)
	ON_WM_DESTROY()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEHOVER()
	ON_WM_MOUSELEAVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_GETDLGCODE()
	ON_WM_KEYDOWN()

	ON_COMMAND(IDM_ITEMVIEW_SELECTALL, OnSelectAll)
	ON_COMMAND(IDM_ITEMVIEW_SELECTNONE, OnSelectNone)
	ON_COMMAND(IDM_ITEMVIEW_SELECTTOGGLE, OnSelectToggle)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_ITEMVIEW_SELECTALL, IDM_ITEMVIEW_SELECTTOGGLE, OnUpdateCommands)
END_MESSAGE_MAP()

void CFrontstageItemView::OnDestroy()
{
	// Item data
	free(m_pItemData);

	// Cached bitmaps
	DeleteObject(m_Bitmaps[BITMAP_SELECTION].hBitmap);
	DeleteObject(m_Bitmaps[BITMAP_REFLECTION].hBitmap);

	CFrontstageScroller::OnDestroy();
}

void CFrontstageItemView::OnMouseMove(UINT nFlags, CPoint point)
{
	if ((nFlags & MK_LBUTTON) && IsDragAndDropEnabled() && IsDragLocationValid())
		if ((abs(point.x-m_DragPos.x)>=GetSystemMetrics(SM_CXDRAG)) || (abs(point.y-m_DragPos.y)>=GetSystemMetrics(SM_CYDRAG)))
		{
			ResetDragLocation();
			m_EditItem = -1;

			if (!SendNotifyMessage(IVN_BEGINDRAGANDDROP))
				return;
		}

	CFrontstageScroller::OnMouseMove(nFlags, point);
}

void CFrontstageItemView::OnMouseHover(UINT nFlags, CPoint point)
{
	if (IsEditOnHoverEnabled() && !IsEditing() && (m_HoverItem==m_EditItem))
	{
		EditLabel(m_EditItem);
	}
	else
	{
		CFrontstageScroller::OnMouseHover(nFlags, point);
	}
}

void CFrontstageItemView::OnMouseLeave()
{
	m_ButtonDownInWindow = FALSE;

	CFrontstageScroller::OnMouseLeave();
}

void CFrontstageItemView::OnLButtonDown(UINT nFlags, CPoint point)
{
	CFrontstageScroller::OnLButtonDown(nFlags, point);

	if (IsFocusItemEnabled())
	{
		m_ButtonDownInWindow = TRUE;

		const INT Index = ItemAtPosition(point);
		if (Index!=-1)
		{
			if (!IsDragLocationValid())
				m_DragPos = point;

			if (nFlags & MK_CONTROL)
			{
				SetFocusItem(Index, SETFOCUSITEM_MOVETOGGLESELECT);
			}
			else
				if ((m_FocusItem==Index) && m_FocusItemSelected && (m_EditItem!=Index))
				{
					m_EditItem = Index;
				}
				else
				{
					SetFocusItem(Index, nFlags & MK_SHIFT ? SETFOCUSITEM_SHIFTSELECT : SETFOCUSITEM_MOVEDESELECT);
				}
		}
	}
}

void CFrontstageItemView::OnLButtonUp(UINT nFlags, CPoint point)
{
	ResetDragLocation();

	if ((!(nFlags & MK_CONTROL) || !IsSelectionEnabled()) && (ItemAtPosition(point)==-1) && m_ButtonDownInWindow)
		OnSelectNone();

	m_ButtonDownInWindow = FALSE;
}

void CFrontstageItemView::OnLButtonDblClk(UINT /*nFlags*/, CPoint point)
{
	if (IsFocusItemEnabled())
	{
		const INT Index = ItemAtPosition(point);
		if (Index!=-1)
		{
			SetFocusItem(Index);
			FireSelectedItem();
		}
	}
}

void CFrontstageItemView::OnRButtonDown(UINT nFlags, CPoint point)
{
	CFrontstageScroller::OnRButtonDown(nFlags, point);

	ResetDragLocation();

	if (IsFocusItemEnabled())
	{
		const INT Index = ItemAtPosition(point);
		if (Index!=-1)
		{
			if (((nFlags & (MK_SHIFT | MK_CONTROL))!=MK_CONTROL) || !IsSelectionEnabled())
				SetFocusItem(Index, IsItemSelected(Index) ? SETFOCUSITEM_MOVEONLY : SETFOCUSITEM_MOVEDESELECT);

			return;
		}
	}
}

void CFrontstageItemView::OnRButtonUp(UINT nFlags, CPoint point)
{
	if (IsFocusItemEnabled())
	{
		const INT Index = ItemAtPosition(point);
		if (Index!=-1)
		{
			if (GetFocus()!=this)
				SetFocus();

			SetFocusItem(Index, IsItemSelected(Index) ? SETFOCUSITEM_MOVEONLY : SETFOCUSITEM_MOVEDESELECT);
		}
		else
		{
			if (!(nFlags & MK_CONTROL) && IsSelectionEnabled())
				OnSelectNone();
		}
	}

	CFrontstageScroller::OnRButtonUp(nFlags, point);
}

UINT CFrontstageItemView::OnGetDlgCode()
{
	return DLGC_WANTARROWS;
}

void CFrontstageItemView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	const BOOL Control = (GetKeyState(VK_CONTROL)<0) && (GetKeyState(VK_SHIFT)>=0);
	const BOOL Plain = (GetKeyState(VK_CONTROL)>=0) && (GetKeyState(VK_SHIFT)>=0);

	// Item selection
	switch (nChar)
	{
	case 'A':
		if (Control)
			OnSelectAll();

		break;

	case 'D':
		if (Control && (GetSelectedItem()>=0))
			DeleteSelectedItem();

		break;

	case 'I':
	case 'T':
		if (Control)
			OnSelectToggle();

		break;

	case 'N':
		if (Control)
			OnSelectNone();

		break;

	case VK_LEFT:
	case VK_RIGHT:
	case VK_UP:
	case VK_PRIOR:
	case VK_DOWN:
	case VK_NEXT:
	case VK_HOME:
	case VK_END:
		if (IsFocusItemEnabled() && m_ItemCount)
		{
			const INT Item = HandleNavigationKeys(nChar, GetKeyState(VK_CONTROL)<0);

			if (Item!=m_FocusItem)
			{
				m_ShowFocusRect = TRUE;
				SetFocusItem(Item, GetKeyState(VK_SHIFT)<0 ? SETFOCUSITEM_SHIFTSELECT : SETFOCUSITEM_MOVEDESELECT);

				UpdateHoverItem();
			}
		}

		break;

	case VK_F2:
		if (Plain && (m_FocusItem>=0) && IsItemSelected(m_FocusItem))
			EditLabel(m_FocusItem);

		break;

	case VK_SPACE:
		if (IsSelectionEnabled() && (m_FocusItem>=0))
		{
			SelectItem(m_FocusItem, !Control || !IsItemSelected(m_FocusItem));
			ItemSelectionChanged(m_FocusItem);
		}

		break;

	case VK_EXECUTE:
	case VK_RETURN:
		if (Plain && (GetSelectedItem()>=0))
			FireSelectedItem();

		break;

	case VK_DELETE:
		if (Plain && (GetSelectedItem()>=0))
			DeleteSelectedItem();

		break;

	default:
		CFrontstageScroller::OnKeyDown(nChar, nRepCnt, nFlags);
	}
}


// Menu commands

void CFrontstageItemView::OnSelectAll()
{
	if (m_ItemCount && IsSelectionEnabled())
	{
		m_MultipleSelected = TRUE;

		for (INT Index=0; Index<m_ItemCount; Index++)
			if (GetItemData(Index)->Valid)
				SelectItem(Index);

		ItemSelectionChanged();
	}
}

void CFrontstageItemView::OnSelectNone()
{
	if (m_ItemCount)
	{
		m_FocusItemSelected = m_MultipleSelected = FALSE;

		if (IsSelectionEnabled())
		{
			for (INT Index=0; Index<m_ItemCount; Index++)
				SelectItem(Index, FALSE);
		}
		else
		{
			if (m_FocusItem>=0)
				SelectItem(m_FocusItem, FALSE);
		}

		ItemSelectionChanged();
	}
}

void CFrontstageItemView::OnSelectToggle()
{
	if (m_ItemCount && IsSelectionEnabled())
	{
		m_MultipleSelected = TRUE;

		for (INT Index=0; Index<m_ItemCount; Index++)
			SelectItem(Index, GetItemData(Index)->Valid && !IsItemSelected(Index));

		ItemSelectionChanged();
	}
}

void CFrontstageItemView::OnUpdateCommands(CCmdUI* pCmdUI)
{
	BOOL bEnable = TRUE;

	switch (pCmdUI->m_nID)
	{
	case IDM_ITEMVIEW_SELECTALL:
	case IDM_ITEMVIEW_SELECTTOGGLE:
		if (!IsSelectionEnabled())
			bEnable = FALSE;

	case IDM_ITEMVIEW_SELECTNONE:
		bEnable &= (m_ItemCount!=0);
		break;
	}

	pCmdUI->Enable(bEnable);
}
