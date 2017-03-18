
// CExplorerList.cpp: Implementierung der Klasse CExplorerList
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CExplorerList
//

#define PADDING                               1
#define DrawLabel(dc, rect, text, format)     dc.DrawText(text, rect, DT_END_ELLIPSIS | format);

CExplorerList::CExplorerList()
	: CListCtrl()
{
	p_ImageList = NULL;
	m_ItemMenuID = m_BackgroundMenuID = 0;
	m_ItemsPerRow = m_ColumnsPerTile = 3;
	m_ColumnCount = 1;
	m_Hover = FALSE;
	m_HoverItem = m_TooltipItem = -1;
	hThemeButton = NULL;
}

void CExplorerList::PreSubclassWindow()
{
	CListCtrl::PreSubclassWindow();

	_AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	if (!pThreadState->m_pWndInit)
		Init();
}

BOOL CExplorerList::PreTranslateMessage(MSG* pMsg)
{
	switch (pMsg->message)
	{
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	case WM_NCLBUTTONDOWN:
	case WM_NCRBUTTONDOWN:
	case WM_NCMBUTTONDOWN:
	case WM_NCLBUTTONUP:
	case WM_NCRBUTTONUP:
	case WM_NCMBUTTONUP:
	case WM_MOUSEWHEEL:
	case WM_MOUSEHWHEEL:
		LFGetApp()->HideTooltip();
		break;
	}

	return CListCtrl::PreTranslateMessage(pMsg);
}

void CExplorerList::Init()
{
	ModifyStyle(0, WS_CLIPCHILDREN | LVS_SHAREIMAGELISTS | LVS_SHOWSELALWAYS | LVS_AUTOARRANGE | LVS_SHAREIMAGELISTS | LVS_ALIGNTOP | LVS_SINGLESEL);
	SetExtendedStyle((GetExtendedStyle() & ~LVS_EX_AUTOCHECKSELECT) | LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_JUSTIFYCOLUMNS);

	CHeaderCtrl* pHeader = GetHeaderCtrl();
	if (pHeader)
		VERIFY(m_wndHeader.SubclassWindow(pHeader->m_hWnd));

	if (LFGetApp()->m_ThemeLibLoaded)
		hThemeButton = LFGetApp()->zOpenThemeData(GetSafeHwnd(), VSCLASS_BUTTON);

	SetWidgetSize();

	CRect rect;
	GetWindowRect(rect);

	AdjustLayout(rect.Width());
}

void CExplorerList::SetWidgetSize()
{
	if (hThemeButton)
	{
		CDC dc;
		dc.CreateCompatibleDC(NULL);

		LFGetApp()->zGetThemePartSize(hThemeButton, dc, BP_CHECKBOX, CBS_UNCHECKEDDISABLED, NULL, TS_DRAW, &m_CheckboxSize);
	}
	else
	{
		m_CheckboxSize.cx = GetSystemMetrics(SM_CXMENUCHECK);
		m_CheckboxSize.cy = GetSystemMetrics(SM_CYMENUCHECK);
	}
}

void CExplorerList::AddCategory(INT ID, LPCWSTR Name, LPCWSTR Hint, BOOL Collapsible)
{
	LVGROUP lvg;
	ZeroMemory(&lvg, sizeof(lvg));

	lvg.cbSize = sizeof(lvg);
	lvg.mask = LVGF_HEADER | LVGF_GROUPID | LVGF_ALIGN;
	lvg.uAlign = LVGA_HEADER_LEFT;
	lvg.iGroupId = ID;
	lvg.pszHeader = (LPWSTR)Name;

	if (LFGetApp()->OSVersion>=OS_Vista)
	{
		if (Hint[0]!=L'\0')
		{
			lvg.pszSubtitle = (LPWSTR)Hint;
			lvg.mask |= LVGF_SUBTITLE;
		}

		if (Collapsible)
		{
			lvg.stateMask = LVGS_COLLAPSIBLE;
			lvg.state = LVGS_COLLAPSIBLE;
			lvg.mask |= LVGF_STATE;
		}
	}

	InsertGroup(ID, &lvg);
}

void CExplorerList::AddColumn(INT ID, LPCWSTR Name, INT Width, BOOL Right)
{
	LV_COLUMN lvc;
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.pszText = (LPWSTR)Name;
	lvc.cx = Width;
	lvc.fmt = Right ? LVCFMT_RIGHT : LVCFMT_LEFT;
	lvc.iSubItem = ID;

	InsertColumn(ID, &lvc);

	if (ID+1>m_ColumnCount)
		m_ColumnCount = ID+1;
}

void CExplorerList::SetMenus(UINT ItemMenuID, BOOL HighlightFirst, UINT BackgroundMenuID)
{
	m_ItemMenuID = ItemMenuID;
	m_HighlightFirst = HighlightFirst;
	m_BackgroundMenuID = BackgroundMenuID;
}

void CExplorerList::SetItemsPerRow(INT ItemsPerRow, INT ColumnsPerTile)
{
	ASSERT(ItemsPerRow>0);

	m_ItemsPerRow = ItemsPerRow;
	m_ColumnsPerTile = ColumnsPerTile;

	CRect rect;
	GetClientRect(rect);

	AdjustLayout(rect.Width());
}

void CExplorerList::AdjustLayout(INT ListWidth)
{
	ASSERT(m_ItemsPerRow>0);

	if ((IsGroupViewEnabled()) && (LFGetApp()->OSVersion==OS_XP))
		ListWidth -= 14;

	if (GetStyle() & WS_BORDER)
		ListWidth -= 4;

	if (!(GetStyle() & LVS_ALIGNLEFT))
		ListWidth -= GetSystemMetrics(SM_CXVSCROLL);

	if (ListWidth<16)
		return;

	INT Width = ListWidth/m_ItemsPerRow;

	// Tile view
	LVTILEVIEWINFO tvi;
	ZeroMemory(&tvi, sizeof(tvi));
	tvi.cbSize = sizeof(LVTILEVIEWINFO);
	tvi.cLines = m_ColumnsPerTile;
	tvi.dwFlags = LVTVIF_FIXEDWIDTH;
	tvi.dwMask = LVTVIM_COLUMNS | LVTVIM_TILESIZE;
	tvi.sizeTile.cx = Width;
	SetTileViewInfo(&tvi);

	// Icon view
	IMAGEINFO ii;
	LFGetApp()->m_SystemImageListExtraLarge.GetImageInfo(0, &ii);
	SetIconSpacing(Width, GetSystemMetrics(SM_CYICONSPACING));
}

__forceinline void CExplorerList::DrawIcon(CDC* pDC, CRect& rect, LVITEM& Item, UINT State)
{
	rect.OffsetRect((rect.Width()-m_IconSize)/2, (rect.Height()-m_IconSize)/2);

	p_ImageList->DrawEx(pDC, Item.iImage, rect.TopLeft(), CSize(m_IconSize, m_IconSize), CLR_NONE, 0xFFFFFF, ((State & LVIS_CUT) || !IsWindowEnabled() ? ILD_BLEND50 : ILD_TRANSPARENT) | (State & LVIS_OVERLAYMASK));
}

void CExplorerList::DrawItem(INT nID, CDC* pDC)
{
	UINT State = GetItemState(nID, LVIS_SELECTED | LVIS_FOCUSED | LVIS_CUT | LVIS_OVERLAYMASK);

	CRect rectItem;
	GetItemRect(nID, rectItem, LVIR_BOUNDS);

	CRect rect(0, 0, rectItem.Width(), rectItem.Height());

	CDC dc;
	dc.CreateCompatibleDC(pDC);
	dc.SetBkMode(TRANSPARENT);

	CBitmap MemBitmap;
	MemBitmap.CreateCompatibleBitmap(pDC, rect.Width(), rect.Height());
	CBitmap* pOldBitmap = dc.SelectObject(&MemBitmap);

	// Hintergrund
	BOOL Themed = IsCtrlThemed();

	dc.FillSolidRect(rect, GetSysColor(IsWindowEnabled() ? COLOR_WINDOW : COLOR_3DFACE));

	NM_TEXTCOLOR tag;
	ZeroMemory(&tag, sizeof(tag));

	tag.hdr.code = REQUEST_TEXTCOLOR;
	tag.hdr.hwndFrom = m_hWnd;
	tag.hdr.idFrom = GetDlgCtrlID();
	tag.Item = nID;
	tag.Color = (COLORREF)-1;

	GetOwner()->SendMessage(WM_NOTIFY, tag.hdr.idFrom, LPARAM(&tag));

	if (IsWindowEnabled())
	{
		DrawListItemBackground(dc, rect, Themed, GetFocus()==this, GetHotItem()==nID, State & LVIS_FOCUSED, State & LVIS_SELECTED, tag.Color);
	}
	else
	{
		dc.SetTextColor(GetSysColor(COLOR_GRAYTEXT));
	}

	// Item
	WCHAR Text[256];
	UINT Columns[4];

	LVITEM Item;
	ZeroMemory(&Item, sizeof(Item));

	Item.iItem = nID;
	Item.pszText = Text;
	Item.cchTextMax = sizeof(Text)/sizeof(WCHAR);
	Item.puColumns = Columns;
	Item.cColumns = 4;
	Item.mask = LVIF_IMAGE | LVIF_TEXT | LVIF_COLUMNS;
	GetItem(&Item);

	// Paint
	CRect rectIcon(rect);
	CRect rectLabel(rect);

	// Checkbox
	if (GetExtendedStyle() & LVS_EX_CHECKBOXES)
		if ((m_View==LV_VIEW_SMALLICON) || (m_View==LV_VIEW_LIST) || (m_View==LV_VIEW_DETAILS))
		{
			CRect rectButton(rectIcon.TopLeft(), m_CheckboxSize);
			rectButton.OffsetRect(2*PADDING, (rectIcon.Height()-m_CheckboxSize.cy)/2);

			if (hThemeButton)
			{
				INT uiStyle = (GetHotItem()==nID) ? CBS_UNCHECKEDHOT : CBS_UNCHECKEDNORMAL;
				if (GetCheck(nID))
					uiStyle += 4;

				LFGetApp()->zDrawThemeBackground(hThemeButton, dc, BP_CHECKBOX, uiStyle, rectButton, rectButton);
			}
			else
			{
				UINT uiStyle = GetCheck(nID) ? DFCS_CHECKED : DFCS_BUTTONCHECK;
				dc.DrawFrameControl(rectButton, DFC_BUTTON, uiStyle);
			}

			rectIcon.left += m_CheckboxSize.cx+3*PADDING;
			rectLabel.left += m_CheckboxSize.cx+3*PADDING;
		}

	// Item
	CFont* pOldFont = dc.SelectObject(GetFont());

	switch (m_View)
	{
	case LV_VIEW_ICON:
		rectIcon.top += PADDING;
		rectIcon.bottom = rectIcon.top+m_IconSize;

		if ((this->GetEditControl()) && (State & LVIS_FOCUSED))
			break;

		rectLabel.top += m_IconSize+PADDING;
		rectLabel.bottom -= PADDING;
		DrawLabel(dc, rectLabel, Item.pszText, DT_VCENTER | DT_CENTER | DT_SINGLELINE | DT_NOPREFIX);

		break;

	case LV_VIEW_SMALLICON:
	case LV_VIEW_LIST:
		rectIcon.left += 2*PADDING;
		rectIcon.right = rectIcon.left+m_IconSize;

		if ((this->GetEditControl()) && (State & LVIS_FOCUSED))
			break;

		rectLabel.left += m_IconSize+4*PADDING;
		DrawLabel(dc, rectLabel, Item.pszText, DT_VCENTER | DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);

		break;

	case LV_VIEW_DETAILS:
		rectIcon.left++;
		rectIcon.right = rectIcon.left+m_IconSize;

		if ((this->GetEditControl()) && (State & LVIS_FOCUSED))
			break;

		rectLabel.right = rectLabel.left+m_Columns[0].cx-5*PADDING+1;
		rectLabel.left = rectIcon.right+5*PADDING-3;
		DrawLabel(dc, rectLabel, Text, DT_VCENTER | DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);

		Item.mask = LVIF_TEXT;

		for (INT a=1; a<min(m_ColumnCount, 16); a++)
		{
			Item.iSubItem = a;
			Item.pszText = Text;
			GetItem(&Item);

			rectLabel.right += m_Columns[a].cx;
			rectLabel.left = rectLabel.right-m_Columns[a].cx+11*PADDING-2;

			dc.DrawText(Item.pszText, rectLabel, DT_END_ELLIPSIS | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | ((m_Columns[a].fmt & LVCFMT_JUSTIFYMASK)==LVCFMT_RIGHT ? DT_RIGHT : DT_LEFT));
		}

		break;

	case LV_VIEW_TILE:
		rectIcon.left += 3*PADDING;
		rectIcon.right = rectIcon.left+m_IconSize;

		if ((this->GetEditControl()) && (State & LVIS_FOCUSED))
			break;

		if (Item.cColumns>15)
			Item.cColumns = 15;

		Item.mask = LVIF_TEXT;
		INT cCount = 1;

		for (UINT a=1; a<=Item.cColumns; a++)
		{
			Item.iSubItem = Item.puColumns[a-1];
			Item.pszText = Text;
			GetItem(&Item);

			if (*Item.pszText)
				cCount++;
		}

		const INT FontHeight = LFGetApp()->m_DefaultFont.GetFontHeight();

		rectLabel.left += m_IconSize+7*PADDING;
		rectLabel.right -= 2*PADDING;
		rectLabel.top += (rect.Height()-cCount*FontHeight)/2;
		rectLabel.bottom = rectLabel.top+FontHeight;

		Item.iSubItem = 0;
		Item.pszText = Text;
		GetItem(&Item);

		DrawLabel(dc, rectLabel, Item.pszText, DT_VCENTER | DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);

		if (IsWindowEnabled() && (tag.Color==(COLORREF)-1) && Themed && !(State & LVIS_SELECTED))
			dc.SetTextColor(0x808080);

		for (UINT a=1; a<=Item.cColumns; a++)
		{
			Item.iSubItem = Item.puColumns[a-1];
			Item.pszText = Text;
			GetItem(&Item);

			if (*Item.pszText)
			{
				rectLabel.OffsetRect(0, FontHeight);
				dc.DrawText(Item.pszText, rectLabel, DT_VCENTER | DT_END_ELLIPSIS | DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);
			}
		}

		break;
	}

	if (p_ImageList)
		DrawIcon(&dc, rectIcon, Item, State);

	if (IsWindowEnabled())
		DrawListItemForeground(dc, rect, Themed, GetFocus()==this,
			GetHotItem()==nID, State & LVIS_FOCUSED, State & LVIS_SELECTED);

	pDC->BitBlt(rectItem.left, rectItem.top, rectItem.Width(), rectItem.Height(), &dc, 0, 0, SRCCOPY);

	dc.SelectObject(pOldFont);
	dc.SelectObject(pOldBitmap);
}


BEGIN_MESSAGE_MAP(CExplorerList, CListCtrl)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_NCHITTEST()
	ON_WM_THEMECHANGED()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_MOUSEHOVER()
	ON_WM_WINDOWPOSCHANGING()
	ON_WM_WINDOWPOSCHANGED()
	ON_WM_CONTEXTMENU()
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
END_MESSAGE_MAP()

INT CExplorerList::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CListCtrl::OnCreate(lpCreateStruct)==-1)
		return -1;

	Init();

	return 0;
}

void CExplorerList::OnDestroy()
{
	if (hThemeButton)
		LFGetApp()->zCloseThemeData(hThemeButton);

	CListCtrl::OnDestroy();
}

LRESULT CExplorerList::OnNcHitTest(CPoint point)
{
	CRect rectWindow;
	GetAncestor(GA_ROOT)->GetWindowRect(rectWindow);

	if (!rectWindow.PtInRect(point))
		return HTNOWHERE;

	if ((point.x<rectWindow.left+BACKSTAGEGRIPPER) || (point.x>=rectWindow.right-BACKSTAGEGRIPPER) ||
		(point.y<rectWindow.top+BACKSTAGEGRIPPER) || (point.y>=rectWindow.bottom-BACKSTAGEGRIPPER))
		return HTTRANSPARENT;

	return CListCtrl::OnNcHitTest(point);
}

LRESULT CExplorerList::OnThemeChanged()
{
	if (LFGetApp()->m_ThemeLibLoaded)
	{
		if (hThemeButton)
			LFGetApp()->zCloseThemeData(hThemeButton);

		hThemeButton = LFGetApp()->zOpenThemeData(GetSafeHwnd(), VSCLASS_BUTTON);
	}

	SetWidgetSize();

	return NULL;
}

void CExplorerList::OnMouseMove(UINT nFlags, CPoint point)
{
	LVHITTESTINFO htt;
	htt.pt = point;
	INT HoverItem = HitTest(&htt);

	if (HoverItem!=m_HoverItem)
	{
		m_HoverItem = HoverItem;
		Invalidate();
	}

	if (!m_Hover)
	{
		m_Hover = TRUE;

		TRACKMOUSEEVENT tme;
		ZeroMemory(&tme, sizeof(tme));
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_LEAVE | TME_HOVER;
		tme.dwHoverTime = HOVERTIME;
		tme.hwndTrack = GetSafeHwnd();
		TrackMouseEvent(&tme);
	}
	else
		if ((LFGetApp()->IsTooltipVisible()) && (m_HoverItem!=m_TooltipItem))
			LFGetApp()->HideTooltip();

	CListCtrl::OnMouseMove(nFlags, point);
}

void CExplorerList::OnMouseLeave()
{
	LFGetApp()->HideTooltip();
	m_Hover = FALSE;
	m_HoverItem = -1;
	Invalidate();

	CListCtrl::OnMouseLeave();
}

void CExplorerList::OnMouseHover(UINT nFlags, CPoint point)
{
	if ((nFlags & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON | MK_XBUTTON1 | MK_XBUTTON2))==0)
	{
		LVHITTESTINFO htt;
		htt.pt = point;

		m_TooltipItem = HitTest(&htt);
		if (m_TooltipItem!=-1)
			if (!LFGetApp()->IsTooltipVisible())
			{
				NM_TOOLTIPDATA tag;
				ZeroMemory(&tag, sizeof(tag));

				tag.hdr.code = REQUEST_TOOLTIP_DATA;
				tag.hdr.hwndFrom = m_hWnd;
				tag.hdr.idFrom = GetDlgCtrlID();
				tag.Item = m_TooltipItem;

				if (GetOwner()->SendMessage(WM_NOTIFY, tag.hdr.idFrom, LPARAM(&tag)))
					LFGetApp()->ShowTooltip(this, point, tag.Caption[0] ? tag.Caption : GetItemText(m_TooltipItem, 0), tag.Hint, tag.hIcon, tag.hBitmap);
			}
	}
	else
	{
		LFGetApp()->HideTooltip();
	}

	TRACKMOUSEEVENT tme;
	ZeroMemory(&tme, sizeof(tme));
	tme.cbSize = sizeof(TRACKMOUSEEVENT);
	tme.dwFlags = TME_LEAVE | TME_HOVER;
	tme.dwHoverTime = HOVERTIME;
	tme.hwndTrack = GetSafeHwnd();
	TrackMouseEvent(&tme);
}

void CExplorerList::OnWindowPosChanging(WINDOWPOS* lpwndpos)
{
	CRect rectWindow;
	GetWindowRect(rectWindow);

	if (lpwndpos->cx<rectWindow.Width())
		AdjustLayout(lpwndpos->cx);

	CListCtrl::OnWindowPosChanging(lpwndpos);
}

void CExplorerList::OnWindowPosChanged(WINDOWPOS* lpwndpos)
{
	CListCtrl::OnWindowPosChanged(lpwndpos);

	AdjustLayout(lpwndpos->cx);
}

void CExplorerList::OnContextMenu(CWnd* pWnd, CPoint pos)
{
	if (pWnd!=this)
		return;

	LVHITTESTINFO pInfo;
	if ((pos.x<0) || (pos.y<0))
	{
		CRect r;
		GetItemRect(GetNextItem(-1, LVNI_FOCUSED), r, LVIR_ICON);
		pInfo.pt.x = pos.x = r.left;
		pInfo.pt.y = r.top;

		GetItemRect(GetNextItem(-1, LVNI_FOCUSED), r, LVIR_LABEL);
		pos.y = r.bottom;
	}
	else
	{
		ScreenToClient(&pos);
		pInfo.pt = pos;
	}

	SubItemHitTest(&pInfo);

	UINT MenuID = m_BackgroundMenuID;
	if (pInfo.iItem!=-1)
		if (GetNextItem(pInfo.iItem-1, LVNI_FOCUSED | LVNI_SELECTED)==pInfo.iItem)
			MenuID = m_ItemMenuID;

	if (MenuID)
	{
		ClientToScreen(&pos);

		CMenu Menu;
		Menu.LoadMenu(MenuID);
		ASSERT_VALID(&Menu);

		CMenu* pPopup = Menu.GetSubMenu(0);
		ASSERT_VALID(pPopup);

		if ((pInfo.iItem!=-1) && (m_HighlightFirst))
			pPopup->SetDefaultItem(0, TRUE);

		pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pos.x, pos.y, GetOwner());
	}
}

void CExplorerList::OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLVCUSTOMDRAW lpListViewCustomDraw = (LPNMLVCUSTOMDRAW)pNMHDR;

	CRect rect;
	GetClientRect(rect);

	switch(lpListViewCustomDraw->nmcd.dwDrawStage)
	{
	case CDDS_PREPAINT:
		m_View = GetView();

		if (m_View==LV_VIEW_DETAILS)
			for (INT a=0; a<=min(m_ColumnCount, 15); a++)
			{
				m_Columns[a].mask = LVCF_FMT | LVCF_WIDTH;
				GetColumn(a, &m_Columns[a]);
			}

		p_ImageList = GetImageList((m_View==LV_VIEW_ICON) || (m_View==LV_VIEW_TILE) ? LVSIL_NORMAL : LVSIL_SMALL);
		if (p_ImageList)
		{
			IMAGEINFO ii;
			p_ImageList->GetImageInfo(0, &ii);

			m_IconSize = ii.rcImage.bottom-ii.rcImage.top;
		}
		else
		{
			m_IconSize = 0;
		}

		*pResult = CDRF_NOTIFYITEMDRAW;
		break;

	case CDDS_ITEMPREPAINT:
		DrawItem((INT)lpListViewCustomDraw->nmcd.dwItemSpec, CDC::FromHandle(lpListViewCustomDraw->nmcd.hdc));

		*pResult = CDRF_SKIPDEFAULT;
		break;

	default:
		*pResult = CDRF_DODEFAULT;
	}
}
