
// CMaintenanceReport.cpp: Implementierung der Klasse CMaintenanceReport
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CMaintenanceReport
//

extern INT GetAttributeIconIndex(UINT Attr);

#define BORDERLEFT     15
#define BORDER         10

CMaintenanceReport::CMaintenanceReport()
	: CFrontstageWnd()
{
	p_MaintenanceList = NULL;
	m_ItemHeight = m_VScrollMax = m_VScrollPos = m_IconSize = 0;
	m_HotItem = -1;
	m_Hover = FALSE;
}

BOOL CMaintenanceReport::Create(CWnd* pParentWnd, UINT nID)
{
	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, LFGetApp()->LoadStandardCursor(IDC_ARROW));

	return CFrontstageWnd::Create(className, _T(""), WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL, CRect(0, 0, 0, 0), pParentWnd, nID);
}

BOOL CMaintenanceReport::PreTranslateMessage(MSG* pMsg)
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

void CMaintenanceReport::SetMaintenanceList(LFMaintenanceList* pMaintenanceList)
{
	p_MaintenanceList = pMaintenanceList;

	AdjustLayout();
}

void CMaintenanceReport::ResetScrollbars()
{
	ScrollWindow(0, m_VScrollPos);
	m_VScrollPos = 0;
	SetScrollPos(SB_VERT, m_VScrollPos, TRUE);
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
	si.nMin = 0;
	si.nMax = m_ScrollHeight-1;
	si.nPos = m_VScrollPos;
	SetScrollInfo(SB_VERT, &si);

	if (OldVScrollPos!=m_VScrollPos)
		Invalidate();
}

void CMaintenanceReport::AdjustLayout()
{
	m_ScrollHeight = p_MaintenanceList ? p_MaintenanceList->m_ItemCount*m_ItemHeight+BORDER : 0;

	AdjustScrollbars();
	Invalidate();
}

void CMaintenanceReport::DrawItem(CDC& dc, LPCRECT rectItem, INT Index, BOOL Themed) const
{
	LFMaintenanceListItem* pItem = &(*p_MaintenanceList)[Index];

	CRect rect(rectItem);

	// Icon
	p_StoreIcons->Draw(&dc, pItem->Icon-1, CPoint(rect.left, rect.top+(rect.Height()-m_IconSize)/2), ILD_TRANSPARENT);
	rect.left += m_IconSize+BORDER;

	// Badge
	HICON hIcon = hIconError;

	switch (pItem->Result)
	{
	case LFOk:
		hIcon = hIconReady;

		break;

	case LFCancel:
	case LFDriveWriteProtected:
	case LFIndexAccessError:
		hIcon = hIconWarning;

		break;
	}

	DrawIconEx(dc, rect.right-m_BadgeSize+2, rect.top+(rect.Height()-m_BadgeSize)/2, hIcon, m_BadgeSize, m_BadgeSize, 0, NULL, DI_NORMAL);
	rect.right -= m_BadgeSize+BORDER-2;

	// Text
	LPCWSTR pDescription = (pItem->Result==LFOk) ? pItem->Comments : m_ErrorText[pItem->Result];

	CRect rectText(rect);
	dc.DrawText(pDescription, -1, rectText, DT_CALCRECT | DT_END_ELLIPSIS | DT_NOPREFIX | DT_LEFT | DT_WORDBREAK);

	const INT FontHeight = LFGetApp()->m_DefaultFont.GetFontHeight();

	if (rectText.Height()>3*FontHeight)
		rectText.bottom = rectText.top+3*FontHeight;

	rect.top += (rect.Height()-rectText.Height()-FontHeight)/2-1;

	dc.SetTextColor(hIcon!=hIconReady ? 0x0000FF : Themed ? 0x000000 : GetSysColor(COLOR_WINDOWTEXT));
	dc.DrawText(pItem->Name, -1, rect, DT_END_ELLIPSIS | DT_NOPREFIX | DT_LEFT | DT_SINGLELINE);

	rect.top += FontHeight;

	dc.SetTextColor(hIcon!=hIconReady ? 0x0000FF : Themed ? 0x808080 : GetSysColor(COLOR_WINDOWTEXT));
	dc.DrawText(pDescription, -1, rect, DT_END_ELLIPSIS | DT_NOPREFIX | DT_LEFT | DT_WORDBREAK);
}


BEGIN_MESSAGE_MAP(CMaintenanceReport, CFrontstageWnd)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_VSCROLL()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_MOUSEHOVER()
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

	m_ItemHeight = max(48, 4*LFGetApp()->m_DefaultFont.GetFontHeight());

	IMAGEINFO ii;
	LFGetApp()->m_SystemImageListLarge.GetImageInfo(0, &ii);

	p_StoreIcons = (m_ItemHeight>=96) ? &LFGetApp()->m_CoreImageListHuge : &LFGetApp()->m_CoreImageListExtraLarge;
	m_IconSize = (m_ItemHeight>=96) ? 96 : 48;
	m_BadgeSize = ii.rcImage.bottom-ii.rcImage.top;

	hIconReady = (HICON)LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_READY), IMAGE_ICON, m_BadgeSize, m_BadgeSize, LR_SHARED);
	hIconWarning = (HICON)LoadImage(AfxGetResourceHandle(), IDI_WARNING, IMAGE_ICON, m_BadgeSize, m_BadgeSize, LR_SHARED);
	hIconError = (HICON)LoadImage(AfxGetResourceHandle(), IDI_ERROR, IMAGE_ICON, m_BadgeSize, m_BadgeSize, LR_SHARED);

	return 0;
}

BOOL CMaintenanceReport::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
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

	BOOL Themed = IsCtrlThemed();

	dc.FillSolidRect(rect, Themed ? 0xFFFFFF : GetSysColor(COLOR_WINDOW));

	if (p_MaintenanceList)
	{
		CFont* pOldFont = dc.SelectObject(&LFGetApp()->m_DefaultFont);

		CRect rectItem(BORDERLEFT, -m_VScrollPos+BORDER/2, rect.right-BORDER, -m_VScrollPos+BORDER/2+m_ItemHeight);
	
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
		nInc = -m_ItemHeight-BORDER-1;
		break;

	case SB_LINEDOWN:
		nInc = m_ItemHeight+BORDER+1;
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

		ZeroMemory(&si, sizeof(si));
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_POS;
		si.nPos = m_VScrollPos;
		SetScrollInfo(SB_VERT, &si);
	}

	CFrontstageWnd::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CMaintenanceReport::OnMouseMove(UINT /*nFlags*/, CPoint point)
{
	INT Item;

	if ((p_MaintenanceList) && (point.x>=BORDERLEFT))
	{
		Item = (point.y+m_VScrollPos-BORDER/2)/m_ItemHeight;

		if ((Item<0) || (Item>=(INT)p_MaintenanceList->m_ItemCount))
			Item = -1;
	}
	else
	{
		Item = -1;
	}

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
		if ((LFGetApp()->IsTooltipVisible()) && (Item!=m_HotItem))
			LFGetApp()->HideTooltip();

	m_HotItem = Item;
}

void CMaintenanceReport::OnMouseLeave()
{
	LFGetApp()->HideTooltip();

	m_Hover = FALSE;
	m_HotItem = -1;
}

void CMaintenanceReport::OnMouseHover(UINT nFlags, CPoint point)
{
	if ((nFlags & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON | MK_XBUTTON1 | MK_XBUTTON2))==0)
	{
		if (m_HotItem!=-1)
			if (!LFGetApp()->IsTooltipVisible())
			{
				LFStoreDescriptor Store;
				if (LFGetStoreSettings((*p_MaintenanceList)[m_HotItem].StoreID, &Store)==LFOk)
				{
					LFItemDescriptor* pItemDescriptor = LFAllocItemDescriptorEx(&Store);

					CString Hint;
					GetHintForStore(Hint, pItemDescriptor);

					LFGetApp()->ShowTooltip(this, point, pItemDescriptor->CoreAttributes.FileName, Hint, LFGetApp()->m_CoreImageListExtraLarge.ExtractIcon(pItemDescriptor->IconID-1));

					LFFreeItemDescriptor(pItemDescriptor);
				}
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
		LFGetApp()->HideTooltip();

		m_VScrollPos += nInc;
		ScrollWindow(0, -nInc);
		SetScrollPos(SB_VERT, m_VScrollPos, TRUE);

		ScreenToClient(&pt);
		OnMouseMove(nFlags, pt);
	}

	return TRUE;
}

void CMaintenanceReport::OnLButtonDown(UINT /*nFlags*/, CPoint /*point*/)
{
	SetFocus();
}
