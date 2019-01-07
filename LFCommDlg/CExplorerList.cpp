
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
	m_ColumnCount = 1;
	m_hThemeButton = NULL;
}

void CExplorerList::PreSubclassWindow()
{
	CListCtrl::PreSubclassWindow();

	_AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	if (!pThreadState->m_pWndInit)
		Init();
}

void CExplorerList::Init()
{
	ModifyStyle(0, WS_CLIPCHILDREN | LVS_SHAREIMAGELISTS | LVS_SHOWSELALWAYS | LVS_AUTOARRANGE | LVS_SHAREIMAGELISTS | LVS_ALIGNTOP | LVS_SINGLESEL);
	SetExtendedStyle((GetExtendedStyle() & ~LVS_EX_AUTOCHECKSELECT) | LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_JUSTIFYCOLUMNS);

	CHeaderCtrl* pHeader = GetHeaderCtrl();
	if (pHeader)
		VERIFY(m_wndHeader.SubclassWindow(pHeader->m_hWnd));

	if (LFGetApp()->m_ThemeLibLoaded)
		m_hThemeButton = LFGetApp()->zOpenThemeData(GetSafeHwnd(), VSCLASS_BUTTON);

	SetWidgetSize();
}

void CExplorerList::SetWidgetSize()
{
	if (m_hThemeButton)
	{
		CDC dc;
		dc.CreateCompatibleDC(NULL);

		LFGetApp()->zGetThemePartSize(m_hThemeButton, dc, BP_CHECKBOX, CBS_UNCHECKEDDISABLED, NULL, TS_DRAW, &m_CheckboxSize);
	}
	else
	{
		m_CheckboxSize.cx = GetSystemMetrics(SM_CXMENUCHECK);
		m_CheckboxSize.cy = GetSystemMetrics(SM_CYMENUCHECK);
	}
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
	const BOOL Themed = IsCtrlThemed();

	dc.FillSolidRect(rect, GetSysColor(IsWindowEnabled() ? COLOR_WINDOW : COLOR_3DFACE));

	if (IsWindowEnabled())
	{
		DrawListItemBackground(dc, rect, Themed, GetFocus()==this, GetHotItem()==nID, State & LVIS_FOCUSED, State & LVIS_SELECTED);
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
	{
		CRect rectButton(rectIcon.TopLeft(), m_CheckboxSize);
		rectButton.OffsetRect(2*PADDING, (rectIcon.Height()-m_CheckboxSize.cy)/2);

		if (m_hThemeButton)
		{
			INT uiStyle = (GetHotItem()==nID) ? CBS_UNCHECKEDHOT : CBS_UNCHECKEDNORMAL;
			if (GetCheck(nID))
				uiStyle += 4;

			LFGetApp()->zDrawThemeBackground(m_hThemeButton, dc, BP_CHECKBOX, uiStyle, rectButton, rectButton);
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

	default:
		ASSERT(FALSE);
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
	ON_WM_THEMECHANGED()
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
	if (m_hThemeButton)
		LFGetApp()->zCloseThemeData(m_hThemeButton);

	CListCtrl::OnDestroy();
}

LRESULT CExplorerList::OnThemeChanged()
{
	if (LFGetApp()->m_ThemeLibLoaded)
	{
		if (m_hThemeButton)
			LFGetApp()->zCloseThemeData(m_hThemeButton);

		m_hThemeButton = LFGetApp()->zOpenThemeData(GetSafeHwnd(), VSCLASS_BUTTON);
	}

	SetWidgetSize();

	return NULL;
}

void CExplorerList::OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLVCUSTOMDRAW lpListViewCustomDraw = (LPNMLVCUSTOMDRAW)pNMHDR;

	CRect rect;
	GetClientRect(rect);

	switch (lpListViewCustomDraw->nmcd.dwDrawStage)
	{
	case CDDS_PREPAINT:
		m_View = GetView();

		if (m_View==LV_VIEW_DETAILS)
			for (INT a=0; a<=min(m_ColumnCount, 15); a++)
			{
				m_Columns[a].mask = LVCF_FMT | LVCF_WIDTH;
				GetColumn(a, &m_Columns[a]);
			}

		p_ImageList = GetImageList(LVSIL_SMALL);
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
