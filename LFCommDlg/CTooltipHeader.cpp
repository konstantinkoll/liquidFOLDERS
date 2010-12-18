
// CTooltipHeader.cpp: Implementierung der Klasse CTooltipHeader
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CTooltipHeader
//

CTooltipHeader::CTooltipHeader()
	: CHeaderCtrl()
{
	m_Hover = FALSE;
	m_HoverItem = m_PressedItem = m_TrackItem = m_TooltipItem = -1;
}

BOOL CTooltipHeader::PreTranslateMessage(MSG* pMsg)
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
		m_TooltipCtrl.Deactivate();
		break;
	}

	return CHeaderCtrl::PreTranslateMessage(pMsg);
}


BEGIN_MESSAGE_MAP(CTooltipHeader, CHeaderCtrl)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_MOUSEHOVER()
END_MESSAGE_MAP()

INT CTooltipHeader::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CHeaderCtrl::OnCreate(lpCreateStruct)==-1)
		return -1;

	SetFont(&((LFApplication*)AfxGetApp())->m_DefaultFont);

	// Tooltip
	m_TooltipCtrl.Create(this);

	return 0;
}

BOOL CTooltipHeader::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CTooltipHeader::OnPaint()
{
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
	dc.FillSolidRect(rect, Themed ? 0xFFFFFF : GetSysColor(COLOR_3DFACE));

	CFont* pOldFont = dc.SelectObject(GetFont());
	Graphics g(dc);

	for (INT a=0; a<GetItemCount(); a++)
	{
		CRect rectItem;
		if (GetItemRect(a, rectItem))
		{
			HDITEM hdi;
			WCHAR lpBuffer[256];
			hdi.mask = HDI_TEXT | HDI_STATE | HDI_FORMAT | HDI_WIDTH;
			hdi.pszText = lpBuffer;
			hdi.cchTextMax = 256;

			if (GetItem(a, &hdi) && (hdi.cxy))
				if (lpBuffer[0]!=L'\0')
				{
					if (Themed)
					{
						if (m_PressedItem==a)
						{
							dc.Draw3dRect(rectItem, 0xD9CBC0, 0xD9CBC0);
							rectItem.DeflateRect(1, 1);

							dc.FillSolidRect(rectItem.left, rectItem.top++, rectItem.Width(), 1, 0xE7DED7);
							dc.FillSolidRect(rectItem.left, rectItem.top++, rectItem.Width(), 1, 0xF2EEEB);
							dc.FillSolidRect(rectItem, 0xF8F7F6);

							rectItem.InflateRect(1, 1);
							rectItem.top -= 2;
						}
						else
							if ((m_PressedItem==-1) && (m_TrackItem==a) || ((m_TrackItem==-1) && (m_HoverItem==a)))
							{
								LinearGradientBrush brush1(Point(0, 0), Point(0, rect.bottom), Color(0xDF, 0xEA, 0xF7), Color(0xE3, 0xE8, 0xEE));
								g.FillRectangle(&brush1, rectItem.left, rectItem.top, 1, rectItem.Height());
								g.FillRectangle(&brush1, rectItem.right-1, rectItem.top, 1, rectItem.Height());
								dc.FillSolidRect(rectItem.left, rectItem.bottom-1, rectItem.Width(), 1, 0xEEE8E3);

								LinearGradientBrush brush2(Point(0, 0), Point(0, rect.bottom-2), Color(0xFD, 0xFE, 0xFF), Color(0xEF, 0xF3, 0xF9));
								g.FillRectangle(&brush2, rectItem.left+2, 0, rectItem.Width()-4, rectItem.Height()-2);
							}
							else
							{
								LinearGradientBrush brush1(Point(0, 0), Point(0, rect.bottom), Color(0xDF, 0xEA, 0xF7), Color(0xFF, 0xFF, 0xFF));
								g.FillRectangle(&brush1, rectItem.right-1, rectItem.top, 1, rectItem.Height());
							}

					}
					else
					{
						COLORREF c1 = GetSysColor(COLOR_3DHIGHLIGHT);
						COLORREF c2 = GetSysColor(COLOR_3DFACE);
						COLORREF c3 = GetSysColor(COLOR_3DSHADOW);
						COLORREF c4 = 0x000000;

						if (m_PressedItem==a)
						{
							std::swap(c1, c4);
							std::swap(c2, c3);
						}

						dc.Draw3dRect(rectItem, c1, c4);
						rectItem.DeflateRect(1, 1);
						dc.Draw3dRect(rectItem, c2, c3);
						rectItem.InflateRect(1, 1);
					}

					rectItem.DeflateRect(5, 1);
					rectItem.top--;

					UINT nFormat = DT_SINGLELINE | DT_END_ELLIPSIS | DT_VCENTER;
					switch (hdi.fmt & HDF_JUSTIFYMASK)
					{
					case HDF_LEFT:
						nFormat |= DT_LEFT;
						break;
					case HDF_CENTER:
						nFormat |= DT_CENTER;
						break;
					case HDF_RIGHT:
						nFormat |= DT_RIGHT;
						break;
					}

					dc.SetTextColor(Themed ? 0x7A604C : GetSysColor(COLOR_WINDOWTEXT));
					dc.DrawText(lpBuffer, -1, rectItem, nFormat);
				}
				else
					if (IsCtrlThemed())
					{
						LinearGradientBrush brush1(Point(0, 0), Point(0, rect.bottom), Color(0xDF, 0xEA, 0xF7), Color(0xFF, 0xFF, 0xFF));
						g.FillRectangle(&brush1, rectItem.right-1, rectItem.top, 1, rectItem.Height());
					}
		}
	}

	dc.SelectObject(pOldFont);

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldBitmap);
}

void CTooltipHeader::OnLButtonDown(UINT nFlags, CPoint point)
{
	HDHITTESTINFO htt;
	htt.pt = point;
	INT idx = HitTest(&htt);
	m_PressedItem = (htt.flags==HHT_ONHEADER) ? idx : -1;
	m_TrackItem = ((htt.flags==HHT_ONDIVIDER) || (htt.flags==HHT_ONDIVOPEN)) ? idx : -1;

	CHeaderCtrl::OnLButtonDown(nFlags, point);
	Invalidate();
}

void CTooltipHeader::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_PressedItem = m_TrackItem = -1;

	CHeaderCtrl::OnLButtonUp(nFlags, point);
	Invalidate();
}

void CTooltipHeader::OnMouseMove(UINT nFlags, CPoint point)
{
	HDHITTESTINFO htt;
	htt.pt = point;
	m_HoverItem = HitTest(&htt);

	if (!m_Hover)
	{
		m_Hover = TRUE;

		TRACKMOUSEEVENT tme;
		ZeroMemory(&tme, sizeof(tme));
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_LEAVE | TME_HOVER;
		tme.dwHoverTime = LFHOVERTIME;
		tme.hwndTrack = GetSafeHwnd();
		TrackMouseEvent(&tme);
	}
	else
		if ((m_TooltipCtrl.IsWindowVisible()) && (m_HoverItem!=m_TooltipItem))
			m_TooltipCtrl.Deactivate();

	CHeaderCtrl::OnMouseMove(nFlags, point);
}

void CTooltipHeader::OnMouseLeave()
{
	m_TooltipCtrl.Deactivate();
	m_Hover = FALSE;
	m_HoverItem = -1;

	CHeaderCtrl::OnMouseLeave();
}

void CTooltipHeader::OnMouseHover(UINT nFlags, CPoint point)
{
	if ((nFlags & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON | MK_XBUTTON1 | MK_XBUTTON2))==0)
	{
		HDHITTESTINFO htt;
		htt.pt = point;

		m_TooltipItem = HitTest(&htt);
		if (m_TooltipItem!=-1)
			if (!m_TooltipCtrl.IsWindowVisible())
			{
				HDITEMW i;
				i.mask = HDI_TEXT;
				i.pszText = m_TooltipTextBuffer;
				i.cchTextMax = 256;

				if (GetItem(m_TooltipItem, &i))
					if (m_TooltipTextBuffer[0]!=L'\0')
					{
						ClientToScreen(&point);
						m_TooltipCtrl.Track(point, NULL, NULL, _T(""), m_TooltipTextBuffer);
					}
			}
	}
	else
	{
		m_TooltipCtrl.Deactivate();
	}

	TRACKMOUSEEVENT tme;
	ZeroMemory(&tme, sizeof(tme));
	tme.cbSize = sizeof(TRACKMOUSEEVENT);
	tme.dwFlags = TME_LEAVE | TME_HOVER;
	tme.dwHoverTime = LFHOVERTIME;
	tme.hwndTrack = GetSafeHwnd();
	TrackMouseEvent(&tme);
}
