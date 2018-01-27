
// CFrontstageItemView.cpp: Implementierung der Klasse CFrontstageItemView
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CFrontstageItemView
//

CFrontstageItemView::CFrontstageItemView(SIZE_T DataSize, UINT Flags, const CSize& szItemInflate)
	: CFrontstageScroller(Flags)
{
	ASSERT(DataSize>=sizeof(ItemData));
	ASSERT(((Flags & FRONTSTAGE_ENABLESELECTION)==0) || (Flags & FRONTSTAGE_ENABLEFOCUSITEM));
	ASSERT(((Flags & FRONTSTAGE_ENABLESHIFTSELECTION)==0) || (Flags & FRONTSTAGE_ENABLESELECTION));
	ASSERT(((Flags & FRONTSTAGE_ENABLEDRAGANDDROP)==0) || (Flags & FRONTSTAGE_ENABLEFOCUSITEM));
	ASSERT(((Flags & FRONTSTAGE_ENABLELABELEDIT)==0) || (Flags & FRONTSTAGE_ENABLEFOCUSITEM));

	m_DataSize = DataSize;
	m_pItemData = NULL;
	m_ItemDataAllocated = m_ItemCount = 0;
	m_Nothing = TRUE;

	m_szItemInflate = szItemInflate;

	m_FocusItem = m_EditItem = m_SelectionAnchor = -1;
	m_FocusItemSelected = m_MultipleSelected = m_ShowFocusRect = FALSE;

	ResetDragLocation();
	ZeroMemory(&m_Bitmaps, sizeof(m_Bitmaps));
}

BOOL CFrontstageItemView::Create(CWnd* pParentWnd, UINT nID, const CRect& rect, UINT nClassStyle)
{
	CString className = AfxRegisterWndClass(nClassStyle | CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, LFGetApp()->LoadStandardCursor(IDC_ARROW));

	return CFrontstageScroller::Create(className, _T(""), WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_TABSTOP, rect, pParentWnd, nID);
}

LRESULT CFrontstageItemView::SendNotifyMessage(UINT Code) const
{
	const NMHDR nmHdr = { m_hWnd, GetDlgCtrlID(), Code };

	return GetOwner()->SendMessage(WM_NOTIFY, nmHdr.idFrom, (LPARAM)&nmHdr);
}


// Item categories

void CFrontstageItemView::AddItemCategory(LPCWSTR Caption, LPCWSTR Hint)
{
	ASSERT(Caption);
	ASSERT(Caption[0]);
	ASSERT(Hint);

	ItemCategoryData Data;

	ZeroMemory(&Data.Rect, sizeof(RECT));
	wcscpy_s(Data.Caption, 256, Caption);
	wcscpy_s(Data.Hint, 256, Hint);

	m_ItemCategories.AddItem(Data);
}

INT CFrontstageItemView::GetItemCategory(INT /*Index*/) const
{
	return -1;
}


// Item data

void CFrontstageItemView::AllocItemData(UINT ItemCount)
{
	FreeItemData(TRUE);

	const SIZE_T Size = ((SIZE_T)(m_ItemDataAllocated=ItemCount))*m_DataSize;
	ZeroMemory(m_pItemData=(LPBYTE)malloc(Size), Size);
}

void CFrontstageItemView::SetItemCount(UINT ItemCount)
{
	AllocItemData(ItemCount);

	m_Nothing = ((m_ItemCount=(INT)ItemCount)==0);
	m_MultipleSelected = IsSelectionEnabled();

	// Focus item
	if (m_FocusItem>=m_ItemCount)
		m_FocusItem = m_ItemCount-1;
}

void CFrontstageItemView::AddItem(LPCVOID pData)
{
	ASSERT((UINT)m_ItemCount<m_ItemDataAllocated);
	ASSERT(offsetof(ItemData, Rect)==0);

	ItemData* pItemData = GetItemData(m_ItemCount++);

	ZeroMemory(pItemData, sizeof(ItemData));
	pItemData->Valid = TRUE;

	if (m_DataSize>sizeof(ItemData))
		memcpy_s(((LPBYTE)pItemData)+sizeof(ItemData), m_DataSize-sizeof(ItemData), ((LPBYTE)pData)+sizeof(ItemData), m_DataSize-sizeof(ItemData));

	m_Nothing = FALSE;
}

void CFrontstageItemView::FreeItemData(BOOL InternalCall)
{
	free(m_pItemData);

	m_pItemData = NULL;
	m_ShowFocusRect = FALSE;

	if (!InternalCall)
	{
		m_ItemDataAllocated = m_ItemCount = 0;
		m_Nothing = TRUE;
		m_FocusItem = -1;
	}
}

void CFrontstageItemView::ValidateAllItems()
{
	for (INT Index=0; Index<m_ItemCount; Index++)
		GetItemData(Index)->Valid = TRUE;
}


// Item sort

INT CFrontstageItemView::CompareItems(INT /*Index1*/, INT /*Index2*/) const
{
	return 0;
}

void CFrontstageItemView::Heap(INT Element, INT Count)
{
	while (Element<=Count/2-1)
	{
		INT Index = (Element+1)*2-1;
		if (Index+1<Count)
			if (CompareItems(Index, Index+1)<0)
				Index++;

		if (CompareItems(Element, Index)<0)
		{
			Swap(Element, Index);
			Element = Index;
		}
		else
		{
			break;
		}
	}
}

void CFrontstageItemView::SortItems()
{
	if (m_ItemCount>1)
	{
		for (INT a=m_ItemCount/2-1; a>=0; a--)
			Heap(a, m_ItemCount);

		for (INT a=m_ItemCount-1; a>0; a--)
		{
			Swap(0, a);
			Heap(0, a);
		}
	}
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

INT CFrontstageItemView::GetFocusItem() const
{
	if ((m_FocusItem>=0) && GetItemData(m_FocusItem)->Valid && m_FocusItemSelected)
	{
		ASSERT(IsFocusItemEnabled());

		return m_FocusItem;
	}

	return -1;
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
		case SETFOCUSITEM_MOVEDESELECT:
			if (IsSelectionEnabled() && m_MultipleSelected)
			{
				m_SelectionAnchor = -1;

				for (INT a=0; a<m_ItemCount; a++)
					SelectItem(a, a==Index);

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
				ItemSelectionChanged(Index);
			}

			m_FocusItemSelected = TRUE;
			m_MultipleSelected = FALSE;

			break;

		case SETFOCUSITEM_MOVETOGGLESELECT:
			m_SelectionAnchor = -1;
			m_MultipleSelected = TRUE;

			SelectItem(Index, m_FocusItemSelected=!IsItemSelected(Index));
			ItemSelectionChanged(Index);

			break;

		case SETFOCUSITEM_SHIFTSELECT:
			if (m_SelectionAnchor==-1)
				m_SelectionAnchor = m_FocusItem;

			m_MultipleSelected = TRUE;

			for (INT a=0; a<m_ItemCount; a++)
				SelectItem(a, ((a>=Index) && (a<=m_SelectionAnchor)) || ((a>=m_SelectionAnchor) && (a<=Index)));

			ItemSelectionChanged();

			break;
		}

		EnsureVisible(m_FocusItem=Index);
	}
}

COLORREF CFrontstageItemView::GetItemTextColor(INT /*Index*/) const
{
	return (COLORREF)-1;
}


// Item selection

BOOL CFrontstageItemView::IsItemSelected(INT Index) const
{
	return (Index==m_FocusItem);
}

void CFrontstageItemView::SelectItem(INT /*Index*/, BOOL /*Select*/)
{
}

INT CFrontstageItemView::GetSelectedItem() const
{
	const INT FocusItem = GetFocusItem();

	return ((FocusItem>=0) && IsItemSelected(FocusItem)) ? FocusItem : -1;
}

BOOL CFrontstageItemView::HasItemsSelected() const
{
	for (INT Index=0; Index<m_ItemCount; Index++)
		if (IsItemSelected(Index))
			return TRUE;

	return FALSE;
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
		for (INT a=Item-1; a>=0; a--)
		{
			const ItemData* pItemData = GetItemData(a);
			if ((pItemData->Row==Row) && pItemData->Valid)
			{
				Item = a;
				break;
			}
		}

		break;

	case VK_RIGHT:
		for (INT a=Item+1; a<m_ItemCount; a++)
		{
			const ItemData* pItemData = GetItemData(a);
			if ((pItemData->Row==Row) && pItemData->Valid)
			{
				Item = a;
				break;
			}
		}

		break;

	case VK_UP:
		for (INT a=Item-1; a>=0; a--)
		{
			const ItemData* pItemData = GetItemData(a);
			if ((pItemData->Row<Row) && pItemData->Valid)
			{
				Item = a;

				if (pItemData->Column<=Column)
					break;
			}

			if (pItemData->Row<Row-1)
				break;
		}

		break;

	case VK_PRIOR:
		for (INT a=Item-1; a>=0; a--)
		{
			const ItemData* pItemData = GetItemData(a);
			if ((pItemData->Row<=Row) && (pItemData->Column<=Column) && pItemData->Valid)
			{
				Item = a;

				if (pItemData->Rect.top<=Bottom-rect.Height()+(INT)m_HeaderHeight)
					break;
			}
		}

		break;

	case VK_DOWN:
		for (INT a=Item+1; a<m_ItemCount; a++)
		{
			const ItemData* pItemData = GetItemData(a);
			if ((pItemData->Row>Row) && pItemData->Valid)
			{
				Item = a;

				if (pItemData->Column>=Column)
					break;
			}

			if (pItemData->Row>Row+1)
				break;
		}

		break;

	case VK_NEXT:
		for (INT a=Item+1; a<m_ItemCount; a++)
		{
			const ItemData* pItemData = GetItemData(a);
			if (pItemData->Row!=GetItemData(a-1)->Row)
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
				Item = a;
		}

		break;

	case VK_HOME:
		if (Control)
		{
			for (INT a=0; a<m_ItemCount; a++)
				if (GetItemData(a)->Valid)
				{
					Item = a;
					break;
				}
		}
		else
			for (INT a=Item-1; a>=0; a--)
			{
				const ItemData* pItemData = GetItemData(a);
				if (pItemData->Valid)
					if (pItemData->Row==Row)
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
		if (Control)
		{
			for (INT a=m_ItemCount-1; a>=0; a--)
				if (GetItemData(a)->Valid)
				{
					Item = a;
					break;
				}
		}
		else
			for (INT a=Item+1; a<m_ItemCount; a++)
			{
				const ItemData* pItemData = GetItemData(a);
				if (pItemData->Valid)
					if (pItemData->Row==Row)
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

	return Item;
}


// Fire focus item

void CFrontstageItemView::FireFocusItem() const
{
	if (IsFocusItemEnabled())
		GetOwner()->PostMessage(WM_COMMAND, IDOK);
}


// Scroller

#define ENSUREVISIBLEEXTRAMARGIN     2

void CFrontstageItemView::EnsureVisible(INT Index)
{
	if (!IsScrollingEnabled())
		return;

	if ((Index<0) || (Index>=m_ItemCount))
		return;

	// Do NOT call GetLayoutRect to avoid resetting the item categories!
	CRect rect;
	GetClientRect(rect);

	if (!rect.Height())
		return;

	CSize nInc(0, 0);

	const RECT rectItem = GetItemRect(Index);

	// Vertical
	if (rectItem.bottom+ENSUREVISIBLEEXTRAMARGIN>rect.Height())
		nInc.cy = rectItem.bottom-rect.Height()+ENSUREVISIBLEEXTRAMARGIN;

	if (rectItem.top-ENSUREVISIBLEEXTRAMARGIN<nInc.cy+(INT)m_HeaderHeight)
		nInc.cy = rectItem.top-(INT)m_HeaderHeight-ENSUREVISIBLEEXTRAMARGIN;

	nInc.cy = max(-m_VScrollPos, min(nInc.cy, m_VScrollMax-m_VScrollPos));

	// Horizontal
	if ((rectItem.right-rectItem.left<rect.Width()) || (rectItem.right<rect.left) || (rectItem.left>=rect.right))
	{
		if (rectItem.right+ENSUREVISIBLEEXTRAMARGIN>rect.Width())
			nInc.cx = rectItem.right-rect.Width()+ENSUREVISIBLEEXTRAMARGIN;

		if (rectItem.left-ENSUREVISIBLEEXTRAMARGIN<nInc.cx)
			nInc.cx = rectItem.left-ENSUREVISIBLEEXTRAMARGIN;

		nInc.cx = max(-m_HScrollPos, min(nInc.cx, m_HScrollMax-m_HScrollPos));
	}

	// Scroll window
	if (nInc.cx || nInc.cy)
	{
		m_HScrollPos += nInc.cx;
		m_VScrollPos += nInc.cy;

		ScrollWindow(-nInc.cx, -nInc.cy);

		SetScrollPos(SB_VERT, m_VScrollPos);
		SetScrollPos(SB_HORZ, m_HScrollPos);
	}
}

void CFrontstageItemView::AdjustScrollbars()
{
	if (IsScrollingEnabled())
		CFrontstageScroller::AdjustScrollbars();
}

BOOL CFrontstageItemView::DrawNothing() const
{
	return !m_pItemData || !m_ItemCount || m_Nothing;
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
				DrawItemBackground(dc, rectItem, Index, Themed);
				DrawItem(dc, g, rectItem, Index, Themed);
				DrawItemForeground(dc, rectItem, Index, Themed);
			}
		}
	}

	// Categories
	for (UINT a=0; a<m_ItemCategories.m_ItemCount; a++)
	{
		CRect rect(m_ItemCategories[a].Rect);
		rect.OffsetRect(-m_HScrollPos, -m_VScrollPos+(INT)m_HeaderHeight);

		if (IntersectRect(&rectIntersect, rect, rectUpdate))
			DrawCategory(dc, rect, m_ItemCategories[a].Caption, m_ItemCategories[a].Hint, Themed);
	}
}


// Draw support

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
	const BOOL Selected = IsSelectionEnabled() ? IsItemSelected(Index) : (m_FocusItem==Index) && m_FocusItemSelected;

	if (Cached && Selected && Themed)
	{
		DRAWCACHED(BITMAP_SELECTION, DrawListItemBackground(MemDC, CRect(0, 0, Width, Height), Themed, GetFocus()==this, m_HoverItem==Index, m_FocusItem==Index, Selected));

		const COLORREF TextColor = GetItemTextColor(Index);
		dc.SetTextColor(Selected ? 0xFFFFFF : TextColor!=(COLORREF)-1 ? TextColor : 0x000000);
	}
	else
	{
		DrawListItemBackground(dc, rectItem, Themed, GetFocus()==this,
			m_HoverItem==Index, m_FocusItem==Index, Selected, GetItemTextColor(Index), m_ShowFocusRect);
	}
}

void CFrontstageItemView::DrawItemForeground(CDC& dc, LPCRECT rectItem, INT Index, BOOL Themed, BOOL Cached)
{
	const BOOL Selected = IsSelectionEnabled() ? IsItemSelected(Index) : (m_FocusItem==Index) && m_FocusItemSelected;

	if (((m_HoverItem!=Index) && !Selected) || !Themed)
		return;

	if (Cached)
	{
		DRAWCACHED(BITMAP_REFLECTION, DrawListItemForeground(MemDC, CRect(0, 0, Width, Height), Themed, GetFocus()==this, m_HoverItem==Index, m_FocusItem==Index, Selected));
	}
	else
	{
		DrawListItemForeground(dc, rectItem, Themed, GetFocus()==this, m_HoverItem==Index, m_FocusItem==Index, Selected);
	}
}

COLORREF CFrontstageItemView::SetLightTextColor(CDC& dc, INT Index, BOOL Themed) const
{
	return IsItemSelected(Index) ? dc.GetTextColor() : dc.SetTextColor(Themed ? 0xA39791 : GetSysColor(COLOR_3DSHADOW));
}

COLORREF CFrontstageItemView::SetDarkTextColor(CDC& dc, INT Index, BOOL Themed) const
{
	return IsItemSelected(Index) ? dc.GetTextColor() : dc.SetTextColor(Themed ? 0x4C4C4C : GetSysColor(COLOR_WINDOWTEXT));
}


// Layouts

void CFrontstageItemView::GetLayoutRect(CRect& rectLayout)
{
	// Reset item categories
	ResetItemCategories();

	// Client area for layout
	GetClientRect(rectLayout);
}

void CFrontstageItemView::AdjustLayoutGrid(const CSize& szItem, const CSize& szGutter, BOOL FullWidth, INT Margin)
{
	CRect rectLayout;
	GetLayoutRect(rectLayout);

	if (!rectLayout.Width())
		return;

	m_szScrollStep.cy = (m_ItemHeight=szItem.cy)+szGutter.cy;

	m_ScrollWidth = m_ScrollHeight = 0;

	INT Column = 0;
	INT Row = 0;
	INT x = Margin;
	INT y = m_HeaderHeight ? 1 : Margin;

	INT Category = -1;

	for (INT a=0; a<m_ItemCount; a++)
	{
			if (GetItemCategory(a)!=Category)
			{
				if (x>Margin)
				{
					Column = 0;
					Row++;

					x = Margin;
					y += szItem.cy+szGutter.cy;
				}

				if (y>Margin)
					y += 8;

				Category = GetItemCategory(a);
				ASSERT(Category<(INT)m_ItemCategories.m_ItemCount);

				const LPRECT lpRect = &m_ItemCategories[Category].Rect;
				lpRect->left = lpRect->right = x;
				lpRect->top = y;
				lpRect->bottom = lpRect->top+2*LFCATEGORYPADDING+m_LargeFontHeight;

				if (m_ItemCategories[Category].Hint[0]!=L'\0')
					lpRect->bottom += m_DefaultFontHeight;

				y = lpRect->bottom+4;
			}

		ItemData* pItemData = GetItemData(a);
		pItemData->Column = Column++;
		pItemData->Row = Row;
		pItemData->Rect.left = x;
		pItemData->Rect.top = y;
		pItemData->Rect.right = x+szItem.cx;
		pItemData->Rect.bottom = y+szItem.cy;

		if ((x+=szItem.cx+szGutter.cx)>m_ScrollWidth)
			m_ScrollWidth = x-1;

		if (y+szItem.cy+szGutter.cy-1>m_ScrollHeight)
			m_ScrollHeight = y+szItem.cy+Margin;

		if ((x+szItem.cx+szGutter.cx>rectLayout.Width()) || FullWidth)
		{
			if (FullWidth)
				pItemData->Rect.right = rectLayout.Width()-szGutter.cx;

			Column = 0;
			Row++;

			x = Margin;
			y += szItem.cy+szGutter.cy;
		}
	}

	// Adjust categories to calculated width
	for (UINT a=0; a<m_ItemCategories.m_ItemCount; a++)
		if (m_ItemCategories[a].Rect.right)
			m_ItemCategories[a].Rect.right = max(m_ScrollWidth, rectLayout.Width())-Margin;

	CFrontstageScroller::AdjustLayout();
}


BEGIN_MESSAGE_MAP(CFrontstageItemView, CFrontstageScroller)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_KEYDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()

	ON_COMMAND(IDM_ITEMVIEW_SELECTALL, OnSelectAll)
	ON_COMMAND(IDM_ITEMVIEW_SELECTNONE, OnSelectNone)
	ON_COMMAND(IDM_ITEMVIEW_SELECTTOGGLE, OnSelectToggle)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_ITEMVIEW_SELECTALL, IDM_ITEMVIEW_SELECTTOGGLE, OnUpdateCommands)
END_MESSAGE_MAP()

INT CFrontstageItemView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrontstageScroller::OnCreate(lpCreateStruct)==-1)
		return -1;

	// Font height
	m_LargeFontHeight = LFGetApp()->m_LargeFont.GetFontHeight();
	m_DefaultFontHeight = LFGetApp()->m_DefaultFont.GetFontHeight();
	m_SmallFontHeight = LFGetApp()->m_SmallFont.GetFontHeight();

	return 0;
}

void CFrontstageItemView::OnDestroy()
{
	// Item data
	free(m_pItemData);

	// Cached bitmaps
	DeleteObject(m_Bitmaps[BITMAP_SELECTION].hBitmap);
	DeleteObject(m_Bitmaps[BITMAP_REFLECTION].hBitmap);

	CFrontstageScroller::OnDestroy();
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
			const INT Item = HandleNavigationKeys(nChar, Control);

			if (Item!=m_FocusItem)
			{
				m_ShowFocusRect = TRUE;
				SetFocusItem(Item, GetKeyState(VK_SHIFT)<0 ? SETFOCUSITEM_SHIFTSELECT : SETFOCUSITEM_MOVEDESELECT);

				UpdateHoverItem();
			}
		}

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
		if (Plain && (m_FocusItem>=0))
			FireFocusItem();

		break;

	default:
		CFrontstageScroller::OnKeyDown(nChar, nRepCnt, nFlags);
	}
}

void CFrontstageItemView::OnMouseMove(UINT nFlags, CPoint point)
{
	if ((nFlags & MK_LBUTTON) && IsDragAndDropEnabled() && IsDragLocationValid())
		if ((abs(point.x-m_DragPos.x)>=GetSystemMetrics(SM_CXDRAG)) || (abs(point.y-m_DragPos.y)>=GetSystemMetrics(SM_CYDRAG)))
		{
			ResetDragLocation();

			if (!SendNotifyMessage(IVN_BEGINDRAGANDDROP))
				return;
		}

	CFrontstageScroller::OnMouseMove(nFlags, point);
}

void CFrontstageItemView::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (IsFocusItemEnabled())
	{
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
				if ((m_FocusItem==Index) && m_FocusItemSelected)
				{
					m_EditItem = Index;
				}
				else
				{
					SetFocusItem(Index, nFlags & MK_SHIFT ? SETFOCUSITEM_SHIFTSELECT : SETFOCUSITEM_MOVEDESELECT);
				}
		}
	}

	CFrontstageScroller::OnLButtonDown(nFlags, point);
}

void CFrontstageItemView::OnLButtonUp(UINT nFlags, CPoint point)
{
	ResetDragLocation();

	if (!(nFlags & MK_CONTROL) || !IsSelectionEnabled())
		if (ItemAtPosition(point)==-1)
			OnSelectNone();
}

void CFrontstageItemView::OnLButtonDblClk(UINT /*nFlags*/, CPoint point)
{
	if (IsFocusItemEnabled())
	{
		const INT Index = ItemAtPosition(point);
		if (Index!=-1)
		{
			SetFocusItem(Index);
			FireFocusItem();
		}
	}
}

void CFrontstageItemView::OnRButtonDown(UINT nFlags, CPoint point)
{
	ResetDragLocation();

	if (IsFocusItemEnabled())
	{
		const INT Index = ItemAtPosition(point);
		if (Index!=-1)
		{
			if (((nFlags & (MK_SHIFT | MK_CONTROL))!=MK_CONTROL) || !IsSelectionEnabled())
				SetFocusItem(Index);

			return;
		}
	}

	CFrontstageScroller::OnRButtonDown(nFlags, point);
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
