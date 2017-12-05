
// CMaintenanceReport.cpp: Implementierung der Klasse CMaintenanceReport
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CMaintenanceReport
//

extern INT GetAttributeIconIndex(UINT Attr);

#define BORDER       5
#define MARGIN       BACKSTAGEBORDER-1
#define PADDINGX     BACKSTAGEBORDER-BORDER
#define PADDINGY     2

CMaintenanceReport::CMaintenanceReport()
	: CFrontstageWnd()
{
	p_MaintenanceList = NULL;

	m_BadgeSize = GetSystemMetrics(SM_CYICON);
	hIconReady = hIconWarning = hIconError = NULL;

	m_ItemHeight = max(LFGetApp()->m_ExtraLargeIconSize, 4*LFGetApp()->m_DefaultFont.GetFontHeight())+2*PADDINGY;
	m_IconSize = (m_ItemHeight>=128) ? 128 : LFGetApp()->m_ExtraLargeIconSize;
	p_StoreIcons = (m_IconSize==128) ? &LFGetApp()->m_CoreImageListJumbo : &LFGetApp()->m_CoreImageListExtraLarge;

	m_VScrollMax = m_VScrollPos = 0;
}

BOOL CMaintenanceReport::Create(CWnd* pParentWnd, UINT nID)
{
	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, LFGetApp()->LoadStandardCursor(IDC_ARROW));

	return CFrontstageWnd::Create(className, _T(""), WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_TABSTOP, CRect(0, 0, 0, 0), pParentWnd, nID);
}

void CMaintenanceReport::SetMaintenanceList(LFMaintenanceList* pMaintenanceList)
{
	p_MaintenanceList = pMaintenanceList;

	AdjustLayout();
}

void CMaintenanceReport::ResetScrollbars()
{
	ScrollWindow(0, m_VScrollPos);
	SetScrollPos(SB_VERT, m_VScrollPos=0);
}

void CMaintenanceReport::AdjustScrollbars()
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
	si.nMax = m_ScrollHeight-1;
	si.nPos = m_VScrollPos;
	SetScrollInfo(SB_VERT, &si);

	if (OldVScrollPos!=m_VScrollPos)
		Invalidate();
}

void CMaintenanceReport::AdjustLayout()
{
	m_ScrollHeight = p_MaintenanceList ? p_MaintenanceList->m_ItemCount*m_ItemHeight+2*BORDER : 0;

	AdjustScrollbars();
	Invalidate();
}

void CMaintenanceReport::DrawItem(CDC& dc, LPCRECT rectItem, INT Index, BOOL Themed) const
{
	DrawListItemBackground(dc, rectItem, Themed, GetFocus()==this, m_HoverItem==Index, FALSE, FALSE);

	LFMaintenanceListItem* pItem = &(*p_MaintenanceList)[Index];

	CRect rect(rectItem);
	rect.DeflateRect(PADDINGX, PADDINGY);

	// Icon
	p_StoreIcons->Draw(&dc, pItem->IconID-1, CPoint(rect.left, rect.top+(rect.Height()-m_IconSize)/2), ILD_TRANSPARENT);
	rect.left += m_IconSize+MARGIN;

	// Badge
	const HICON hIcon = (pItem->Result==LFOk) ? hIconReady : (pItem->Result<LFFirstFatalError) ? hIconWarning : hIconError;

	DrawIconEx(dc, rect.right-m_BadgeSize+2, rect.top+(rect.Height()-m_BadgeSize)/2, hIcon, m_BadgeSize, m_BadgeSize, 0, NULL, DI_NORMAL);
	rect.right -= m_BadgeSize+MARGIN-2;

	// Text
	LPCWSTR pDescription = (pItem->Result==LFOk) ? pItem->Comments : m_ErrorText[pItem->Result];

	CRect rectText(rect);
	dc.DrawText(pDescription, -1, rectText, DT_CALCRECT | DT_END_ELLIPSIS | DT_NOPREFIX | DT_LEFT | DT_WORDBREAK);

	const INT FontHeight = LFGetApp()->m_DefaultFont.GetFontHeight();

	if (rectText.Height()>3*FontHeight)
		rectText.bottom = rectText.top+3*FontHeight;

	rect.top += (rect.Height()-rectText.Height()-FontHeight)/2-1;

	dc.SetTextColor(Themed ? 0x000000 : GetSysColor(COLOR_WINDOWTEXT));
	dc.DrawText(pItem->Name, -1, rect, DT_END_ELLIPSIS | DT_NOPREFIX | DT_LEFT | DT_SINGLELINE);

	rect.top += FontHeight;

	dc.SetTextColor(hIcon!=hIconReady ? 0x0000FF : Themed ? 0x808080 : GetSysColor(COLOR_WINDOWTEXT));
	dc.DrawText(pDescription, -1, rect, DT_END_ELLIPSIS | DT_NOPREFIX | DT_LEFT | DT_WORDBREAK);
}

INT CMaintenanceReport::ItemAtPosition(CPoint point) const
{
	if (!p_MaintenanceList)
		return -1;

	CRect rectClient;
	GetClientRect(rectClient);

	if ((point.x<BORDER) || (point.x>=rectClient.right-BORDER))
		return -1;

	INT Index = (point.y+m_VScrollPos-BORDER)/m_ItemHeight;

	if ((Index<0) || (Index>=(INT)p_MaintenanceList->m_ItemCount))
		Index = -1;

	return Index;
}

void CMaintenanceReport::InvalidateItem(INT Index)
{
	const INT y = Index*m_ItemHeight-m_VScrollPos+BORDER;

	CRect rectClient;
	GetClientRect(rectClient);

	InvalidateRect(CRect(BORDER, y, rectClient.right-BORDER, y+m_ItemHeight));
}

void CMaintenanceReport::ShowTooltip(const CPoint& point)
{
	ASSERT(m_HoverItem>=0);

	LFStoreDescriptor Store;
	if (LFGetStoreSettings((*p_MaintenanceList)[m_HoverItem].StoreID, Store, TRUE)==LFOk)
		LFGetApp()->ShowTooltip(this, point, Store);
}


BEGIN_MESSAGE_MAP(CMaintenanceReport, CFrontstageWnd)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_VSCROLL()
	ON_WM_MOUSEWHEEL()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()

INT CMaintenanceReport::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrontstageWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	ResetScrollbars();

	for (UINT a=0; a<LFErrorCount; a++)
		LFGetErrorText(m_ErrorText[a], 256, a);

	hIconReady = (HICON)LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_READY), IMAGE_ICON, m_BadgeSize, m_BadgeSize, LR_SHARED);
	hIconWarning = (HICON)LoadImage(AfxGetResourceHandle(), IDI_WARNING, IMAGE_ICON, m_BadgeSize, m_BadgeSize, LR_SHARED);
	hIconError = (HICON)LoadImage(AfxGetResourceHandle(), IDI_ERROR, IMAGE_ICON, m_BadgeSize, m_BadgeSize, LR_SHARED);

	return 0;
}

void CMaintenanceReport::OnPaint()
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

	const BOOL Themed = IsCtrlThemed();

	dc.FillSolidRect(rect, Themed ? 0xFFFFFF : GetSysColor(COLOR_WINDOW));

	if (p_MaintenanceList)
	{
		CFont* pOldFont = dc.SelectObject(&LFGetApp()->m_DefaultFont);

		CRect rectItem(BORDER, -m_VScrollPos+BORDER, rect.right-BORDER, -m_VScrollPos+BORDER+m_ItemHeight);
	
		for (UINT a=0; a<p_MaintenanceList->m_ItemCount; a++)
		{
			RECT rectIntersect;
			if (IntersectRect(&rectIntersect, rectItem, rectUpdate))
				DrawItem(dc, rectItem, a, Themed);

			rectItem.OffsetRect(0, m_ItemHeight);
		}

		dc.SelectObject(pOldFont);
	}

	DrawWindowEdge(dc, Themed);

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);

	dc.SelectObject(pOldBitmap);
}

void CMaintenanceReport::OnSize(UINT nType, INT cx, INT cy)
{
	CFrontstageWnd::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CMaintenanceReport::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
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
		nInc = -(INT)m_ItemHeight;
		break;

	case SB_LINEDOWN:
		nInc = (INT)m_ItemHeight;
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

BOOL CMaintenanceReport::OnMouseWheel(UINT nFlags, SHORT zDelta, CPoint pt)
{
	CRect rect;
	GetWindowRect(rect);
	if (!rect.PtInRect(pt))
		return FALSE;

	INT nScrollLines;
	SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &nScrollLines, 0);
	if (nScrollLines<1)
		nScrollLines = 1;

	INT nInc = max(-m_VScrollPos, min(-zDelta*(INT)m_ItemHeight*nScrollLines/WHEEL_DELTA, m_VScrollMax-m_VScrollPos));
	if (nInc)
	{
		HideTooltip();

		m_VScrollPos += nInc;
		ScrollWindow(0, -nInc);
		SetScrollPos(SB_VERT, m_VScrollPos);

		ScreenToClient(&pt);
		OnMouseMove(nFlags, pt);
	}

	return TRUE;
}

void CMaintenanceReport::OnLButtonDown(UINT /*nFlags*/, CPoint /*point*/)
{
	SetFocus();
}
