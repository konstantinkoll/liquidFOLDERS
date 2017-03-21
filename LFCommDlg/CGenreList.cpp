
// CGenreList.cpp: Implementierung der Klasse CGenreList
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CGenreList
//

extern INT GetAttributeIconIndex(UINT Attr);

#define ICONPADDING     4
#define ICONSIZE        128
#define ITEMPADDING     2

CGenreList::CGenreList()
	: CFrontstageWnd()
{
	m_CategoriesHeight = m_VScrollMax = m_VScrollPos = 0;
	m_ItemsHeight = m_FocusItem = m_HotItem = -1;
	m_IsFirstItemInCategory = m_Hover = FALSE;
}

BOOL CGenreList::Create(CWnd* pParentWnd, UINT nID)
{
	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, LFGetApp()->LoadStandardCursor(IDC_ARROW));

	return CFrontstageWnd::Create(className, _T(""), WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_TABSTOP, CRect(0, 0, 0, 0), pParentWnd, nID);
}

BOOL CGenreList::PreTranslateMessage(MSG* pMsg)
{
	switch (pMsg->message)
	{
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
		LFGetApp()->HideTooltip();
		break;
	}

	return CFrontstageWnd::PreTranslateMessage(pMsg);
}

void CGenreList::AddCategory(LFMusicGenre* pMusicGenre)
{
	GenreCategoryData cd;

	wcscpy_s(cd.Caption, 256, pMusicGenre->Name);
	cd.IconID = pMusicGenre->IconID;
	cd.Rect.left = BACKSTAGEBORDER;
	cd.Rect.top = max(m_CategoriesHeight, m_ItemsHeight+1)+BACKSTAGEBORDER;
	cd.Rect.bottom = cd.Rect.top+LFGetApp()->m_LargeFont.GetFontHeight()+2*LFCATEGORYPADDING;

	m_Categories.AddItem(cd);

	m_ItemsHeight = cd.Rect.bottom+BACKSTAGEBORDER/2;
	m_CategoriesHeight = m_ItemsHeight+ICONSIZE-2*ICONPADDING;
	m_IsFirstItemInCategory = TRUE;
}

void CGenreList::AddItem(LFMusicGenre* pMusicGenre, INT Index, UINT FileCount, INT64 FileSize)
{
	// Hide some genres like "Negerpunk"
	if (!pMusicGenre->Show)
		return;

	GenreItemData id;

	id.pMusicGenre = pMusicGenre;
	id.Index = Index;
	id.FileCount = FileCount;
	id.FileSize = FileSize;
	id.Rect.left = 2*BACKSTAGEBORDER+ICONSIZE-2*ICONPADDING;
	id.Rect.top = m_ItemsHeight;
	id.Rect.bottom = id.Rect.top+m_ItemHeight;

	m_Items.AddItem(id);

	m_ItemsHeight = id.Rect.bottom-1;

	if (m_IsFirstItemInCategory)
	{
		m_ItemsHeight += BACKSTAGEBORDER*3/4;
		m_IsFirstItemInCategory = FALSE;
	}
}

void CGenreList::ResetScrollbars()
{
	ScrollWindow(0, m_VScrollPos);
	SetScrollPos(SB_VERT, m_VScrollPos=0);
}

void CGenreList::AdjustScrollbars()
{
	CRect rect;
	GetClientRect(rect);

	INT OldVScrollPos = m_VScrollPos;
	m_VScrollMax = max(0, m_ScrollHeight-rect.Height());
	m_VScrollPos = min(m_VScrollPos, m_VScrollMax);

	SCROLLINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
	si.nPage = rect.Height();
	si.nMin = 0;
	si.nMax = m_ScrollHeight-1;
	si.nPos = m_VScrollPos;
	SetScrollInfo(SB_VERT, &si);

	if (OldVScrollPos!=m_VScrollPos)
		Invalidate();
}

void CGenreList::AdjustLayout()
{
	// Adjust item width
	CRect rectClient;
	GetClientRect(rectClient);

	for (UINT a=0; a<m_Categories.m_ItemCount; a++)
		m_Categories[a].Rect.right = rectClient.right-BACKSTAGEBORDER;

	for (UINT a=0; a<m_Items.m_ItemCount; a++)
		m_Items[a].Rect.right = rectClient.right-BACKSTAGEBORDER;

	m_ScrollHeight = max(m_CategoriesHeight, m_ItemsHeight+1)+BACKSTAGEBORDER;

	AdjustScrollbars();
	EnsureVisible(m_FocusItem);
	Invalidate();
}

void CGenreList::EnsureVisible(INT Index)
{
	// Only make visible if item is focused
	if (Index<0)
		return;

	CRect rect;
	GetClientRect(rect);

	RECT rectItem = GetItemRect(Index);

	// Vertikal
	INT nInc = 0;

	if (rectItem.bottom>rect.Height())
		nInc = rectItem.bottom-rect.Height();

	if (rectItem.top<nInc)
		nInc = rectItem.top;

	nInc = max(-m_VScrollPos, min(nInc, m_VScrollMax-m_VScrollPos));
	if (nInc)
	{
		m_VScrollPos += nInc;
		ScrollWindow(0, -nInc);
		SetScrollPos(SB_VERT, m_VScrollPos);
	}
}

void CGenreList::SetFocusItem(INT FocusItem)
{
	if (FocusItem<0)
		FocusItem = 0;

	if (FocusItem>(INT)m_Items.m_ItemCount-1)
		FocusItem = (INT)m_Items.m_ItemCount-1;

	m_FocusItem = FocusItem;

	Invalidate();
	EnsureVisible(m_FocusItem);
}

RECT CGenreList::GetItemRect(INT Index) const
{
	RECT rect = { 0, 0, 0, 0 };

	if ((Index>=0) && (Index<(INT)m_Items.m_ItemCount))
	{
		rect = m_Items[Index].Rect;
		OffsetRect(&rect, 0, -m_VScrollPos);
	}

	return rect;
}

INT CGenreList::ItemAtPosition(CPoint point) const
{
	point.Offset(0, m_VScrollPos);

	for (UINT a=0; a<m_Items.m_ItemCount; a++)
		if (PtInRect(&m_Items[a].Rect, point))
			return a;

	return -1;
}

void CGenreList::InvalidateItem(INT Index)
{
	if ((Index>=0) && (Index<(INT)m_Items.m_ItemCount))
	{
		RECT rect = GetItemRect(Index);
		InvalidateRect(&rect);
	}
}

void CGenreList::DrawItem(CDC& dc, CRect& rectItem, INT Index, BOOL Themed) const
{
	DrawListItemBackground(dc, rectItem, Themed, GetFocus()==this, m_HotItem==Index, m_FocusItem==Index, m_FocusItem==Index);

	rectItem.DeflateRect(2*ITEMPADDING, ITEMPADDING);

	if (m_Items[Index].FileCount)
		rectItem.right -= m_CountWidth+ITEMPADDING;

	// Name
	dc.DrawText(m_Items[Index].pMusicGenre->Name, -1, rectItem, DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX | DT_LEFT | DT_VCENTER);

	// Count
	if (m_Items[Index].FileCount)
	{
		rectItem.left = rectItem.right+ITEMPADDING;
		rectItem.right += m_CountWidth;

		CFont* pOldFont = dc.SelectObject(&LFGetApp()->m_SmallFont);

		if (Themed && (m_FocusItem!=Index))
			dc.SetTextColor(0x808080);

		dc.DrawText(CBackstageSidebar::FormatCount(m_Items[Index].FileCount), rectItem, DT_SINGLELINE | DT_NOPREFIX | DT_RIGHT | DT_VCENTER);

		dc.SelectObject(pOldFont);
	}
}

void CGenreList::SelectGenre(UINT Genre)
{
	for (UINT a=0; a<m_Items.m_ItemCount; a++)
		if (m_Items[a].Index==Genre)
		{
			SetFocusItem(a);

			break;
		}
}


BEGIN_MESSAGE_MAP(CGenreList, CFrontstageWnd)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_VSCROLL()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_MOUSEHOVER()
	ON_WM_MOUSEWHEEL()
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_GETDLGCODE()
END_MESSAGE_MAP()

INT CGenreList::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrontstageWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	ResetScrollbars();

	m_ItemHeight = LFGetApp()->m_DefaultFont.GetFontHeight()+2*ITEMPADDING;
	m_CountWidth = LFGetApp()->m_SmallFont.GetTextExtent(_T("000W")).cx+2*ITEMPADDING;

	return 0;
}

BOOL CGenreList::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CGenreList::OnPaint()
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

	// Categories
	for (UINT a=0; a<m_Categories.m_ItemCount; a++)
	{
		CRect rect(m_Categories[a].Rect);
		rect.OffsetRect(0, -m_VScrollPos);

		DrawCategory(dc, rect, m_Categories[a].Caption, NULL, Themed);
		LFGetApp()->m_CoreImageListJumbo.Draw(&dc, m_Categories[a].IconID-1, CPoint(rect.left-ICONPADDING, rect.bottom+BACKSTAGEBORDER/2-ICONPADDING), ILD_NORMAL);
	}

	// Items
	CFont* pOldFont = dc.SelectObject(&LFGetApp()->m_DefaultFont);

	for (UINT a=0; a<m_Items.m_ItemCount; a++)
	{
		CRect rect(m_Items[a].Rect);
		rect.OffsetRect(0, -m_VScrollPos);

		RECT rectIntersect;
		if (IntersectRect(&rectIntersect, rect, rectUpdate))
			DrawItem(dc, rect, a, Themed);
	}

	dc.SelectObject(pOldFont);

	DrawWindowEdge(dc, Themed);

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);

	dc.SelectObject(pOldBitmap);
}

void CGenreList::OnSize(UINT nType, INT cx, INT cy)
{
	CFrontstageWnd::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CGenreList::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	CRect rect;
	GetClientRect(rect);

	SCROLLINFO si;

	INT nInc = 0;
	switch (nSBCode)
	{
	case SB_TOP:
		nInc = -m_VScrollPos;
		break;

	case SB_BOTTOM:
		nInc = m_VScrollMax-m_VScrollPos;
		break;

	case SB_LINEUP:
		nInc = -(INT)(m_ItemHeight-1);
		break;

	case SB_LINEDOWN:
		nInc = (INT)(m_ItemHeight-1);
		break;

	case SB_PAGEUP:
		nInc = min(-1, -rect.Height());
		break;

	case SB_PAGEDOWN:
		nInc = max(1, rect.Height());
		break;

	case SB_THUMBTRACK:
		ZeroMemory(&si, sizeof(si));
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_TRACKPOS;
		GetScrollInfo(SB_VERT, &si);

		nInc = si.nTrackPos-m_VScrollPos;
		break;
	}

	nInc = max(-m_VScrollPos, min(nInc, m_VScrollMax-m_VScrollPos));
	if (nInc)
	{
		m_VScrollPos += nInc;
		ScrollWindow(0, -nInc);
		SetScrollPos(SB_VERT, m_VScrollPos);
	}

	CFrontstageWnd::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CGenreList::OnMouseMove(UINT /*nFlags*/, CPoint point)
{
	INT Index = ItemAtPosition(point);

	if (!m_Hover)
	{
		m_Hover = TRUE;

		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_LEAVE | TME_HOVER;
		tme.dwHoverTime = HOVERTIME;
		tme.hwndTrack = m_hWnd;
		TrackMouseEvent(&tme);
	}
	else
		if ((LFGetApp()->IsTooltipVisible()) && (Index!=m_HotItem))
			LFGetApp()->HideTooltip();

	if (m_HotItem!=Index)
	{
		InvalidateItem(m_HotItem);

		m_HotItem = Index;

		InvalidateItem(m_HotItem);
	}
}

void CGenreList::OnMouseLeave()
{
	InvalidateItem(m_HotItem);
	LFGetApp()->HideTooltip();

	m_Hover = FALSE;
	m_HotItem = -1;
}

void CGenreList::OnMouseHover(UINT nFlags, CPoint point)
{
	if ((nFlags & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON | MK_XBUTTON1 | MK_XBUTTON2))==0)
	{
		if (m_HotItem!=-1)
			if (!LFGetApp()->IsTooltipVisible())
			{
				WCHAR Hint[256];
				LFCombineFileCountSize(m_Items[m_HotItem].FileCount, m_Items[m_HotItem].FileSize, Hint, 256);

				LFGetApp()->ShowTooltip(this, point, m_Items[m_HotItem].pMusicGenre->Name, Hint);
			}
	}
	else
	{
		LFGetApp()->HideTooltip();
	}

	TRACKMOUSEEVENT tme;
	tme.cbSize = sizeof(TRACKMOUSEEVENT);
	tme.dwFlags = TME_LEAVE | TME_HOVER;
	tme.dwHoverTime = HOVERTIME;
	tme.hwndTrack = m_hWnd;
	TrackMouseEvent(&tme);
}

BOOL CGenreList::OnMouseWheel(UINT nFlags, SHORT zDelta, CPoint pt)
{
	CRect rect;
	GetWindowRect(rect);

	if (!rect.PtInRect(pt))
		return FALSE;

	INT nScrollLines;
	SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &nScrollLines, 0);
	if (nScrollLines<1)
		nScrollLines = 1;

	INT nInc = max(-m_VScrollPos, min(-zDelta*(INT)(m_ItemHeight-1)*nScrollLines/WHEEL_DELTA, m_VScrollMax-m_VScrollPos));
	if (nInc)
	{
		m_VScrollPos += nInc;
		ScrollWindow(0, -nInc);
		SetScrollPos(SB_VERT, m_VScrollPos);

		ScreenToClient(&pt);
		OnMouseMove(nFlags, pt);
	}

	return TRUE;
}

void CGenreList::OnKeyDown(UINT nChar, UINT /*nRepCnt*/, UINT /*nFlags*/)
{
	CRect rectWindow;
	GetWindowRect(rectWindow);

	if (!rectWindow.Width())
		return;

	INT Item = m_FocusItem;

	switch (nChar)
	{
	case VK_PRIOR:
		Item -= max(1, rectWindow.Height()/(m_ItemHeight-1));
		break;

	case VK_NEXT:
		Item += max(1, rectWindow.Height()/(m_ItemHeight-1));
		break;

	case VK_UP:
		Item--;
		break;

	case VK_DOWN:
		Item++;
		break;

	case VK_HOME:
		if (GetKeyState(VK_CONTROL)<0)
			Item = 0;

		break;

	case VK_END:
		if (GetKeyState(VK_CONTROL)<0)
			Item = m_Items.m_ItemCount-1;

		break;
	}

	if (Item!=m_FocusItem)
	{
		SetFocusItem(Item);

		CPoint pt;
		GetCursorPos(&pt);
		ScreenToClient(&pt);
		OnMouseMove(0, pt);
	}
}

void CGenreList::OnLButtonDown(UINT /*nFlags*/, CPoint point)
{
	INT Index = ItemAtPosition(point);
	if (Index!=-1)
		SetFocusItem(Index);

	if (GetFocus()!=this)
		SetFocus();
}

void CGenreList::OnLButtonUp(UINT /*nFlags*/, CPoint /*point*/)
{
	if (GetFocus()!=this)
		SetFocus();
}

void CGenreList::OnLButtonDblClk(UINT /*nFlags*/, CPoint point)
{
	INT Index = ItemAtPosition(point);
	if (Index!=-1)
		GetOwner()->SendMessage(WM_COMMAND, IDOK);
}

void CGenreList::OnRButtonDown(UINT /*nFlags*/, CPoint point)
{
	INT Index = ItemAtPosition(point);
	if (Index!=-1)
	{
		if (m_FocusItem!=Index)
		{
			InvalidateItem(m_FocusItem);
			m_FocusItem = Index;
			InvalidateItem(m_FocusItem);
		}
	}
	else
	{
		if (GetFocus()!=this)
			SetFocus();
	}
}

void CGenreList::OnRButtonUp(UINT nFlags, CPoint point)
{
	INT Index = ItemAtPosition(point);
	if (Index!=-1)
	{
		if (GetFocus()!=this)
			SetFocus();

		if (m_FocusItem!=Index)
		{
			InvalidateItem(m_FocusItem);
			m_FocusItem = Index;
			InvalidateItem(m_FocusItem);
		}
	}

	CFrontstageWnd::OnRButtonUp(nFlags, point);
}

void CGenreList::OnSetFocus(CWnd* /*pOldWnd*/)
{
	Invalidate();
}

void CGenreList::OnKillFocus(CWnd* /*pNewWnd*/)
{
	Invalidate();
}

UINT CGenreList::OnGetDlgCode()
{
	return DLGC_WANTARROWS;
}
