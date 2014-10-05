
// CTooltipHeader.cpp: Implementierung der Klasse CTooltipHeader
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CTooltipHeader
//

extern AFX_EXTENSION_MODULE LFCommDlgDLL;

CTooltipHeader::CTooltipHeader()
	: CHeaderCtrl()
{
	m_Hover = FALSE;
	m_HoverItem = m_PressedItem = m_TrackItem = m_TooltipItem = -1;
}

void CTooltipHeader::PreSubclassWindow()
{
	CHeaderCtrl::PreSubclassWindow();

	_AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	if (!pThreadState->m_pWndInit)
		Init();
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

void CTooltipHeader::Init()
{
	SetFont(&LFGetApp()->m_DefaultFont);

	m_SortIndicators.Create(IDB_SORTINDICATORS, 7, 4);

	// Tooltip
	m_TooltipCtrl.Create(this);
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

	Init();

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

	if (Themed)
	{
		dc.FillSolidRect(rect, 0xFFFFFF);

		CRect rectParent;
		GetParent()->GetClientRect(rectParent);

		CGdiPlusBitmap* pDivider = LFGetApp()->GetCachedResourceImage(IDB_DIVUP, _T("PNG"), LFCommDlgDLL.hResource);
		g.DrawImage(pDivider->m_pBitmap, (rectParent.Width()-(INT)pDivider->m_pBitmap->GetWidth())/2+GetParent()->GetScrollPos(SB_HORZ), rect.Height()-(INT)pDivider->m_pBitmap->GetHeight());
	}

	const UINT Line = rect.Height()*2/5;
	LinearGradientBrush brush1(Point(0, 0), Point(0, Line), Color(0x00, 0x00, 0x00, 0x00), Color(0x40, 0x00, 0x00, 0x00));
	LinearGradientBrush brush2(Point(0, Line-1), Point(0, rect.Height()), Color(0x40, 0x00, 0x00, 0x00), Color(0x00, 0x00, 0x00, 0x00));

	Pen pen(Color(0x80, 0xFF, 0xFF, 0xFF));

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
			{
				if (Themed)
				{
					if (rectItem.left==0)
					{
						g.DrawRectangle(&pen, rectItem.left-1, -1, 2, rectItem.Height()-1);
						g.FillRectangle(&brush1, rectItem.left, 0, 1, Line);
						g.FillRectangle(&brush2, rectItem.left, Line, 1, rectItem.Height()-Line-1);

						rectItem.left++;
					}

					g.DrawRectangle(&pen, rectItem.right-2, -1, 2, rectItem.Height()-1);
					g.FillRectangle(&brush1, rectItem.right-1, 0, 1, Line);
					g.FillRectangle(&brush2, rectItem.right-1, Line, 1, rectItem.Height()-Line-1);

					rectItem.right--;
				}

				if (lpBuffer[0]!=L'\0')
				{
					if (Themed)
					{
						const BOOL Hover = (m_PressedItem==-1) && ((m_TrackItem==a) || (m_HoverItem==a));

						if (Hover || (m_PressedItem==a))
						{
							CRect rectBounds(rectItem);
							rectBounds.InflateRect(1, 0);

							if (rectBounds.left<0)
								rectBounds.left = 0;

							COLORREF clr = LFGetApp()->OSVersion==OS_Eight ? Hover ? 0xEDC093 : 0xDAA026 : Hover ? 0xB17F3C : 0x8B622C;
							dc.Draw3dRect(rectBounds, clr, clr);

							rectBounds.DeflateRect(1, 1);

							if (LFGetApp()->OSVersion==OS_Eight)
							{
								dc.FillSolidRect(rectBounds, Hover ? 0xF8F0E1 : 0xF0E1C3);
							}
							else
								if (m_PressedItem==a)
								{
									dc.FillSolidRect(rectBounds, 0xF6E4C2);

									INT y = (rectBounds.top+rectBounds.bottom)/2;

									LinearGradientBrush brush2(Point(rectBounds.left, y-1), Point(rectBounds.left, rectBounds.bottom), Color(0xA9, 0xD9, 0xF2), Color(0x90, 0xCB, 0xEB));
									g.FillRectangle(&brush2, rectBounds.left, y, rectBounds.Width(), rectBounds.bottom-y);

									LinearGradientBrush brush3(Point(rectBounds.left, rectBounds.top), Point(rectBounds.left, rectBounds.top+2), Color(0x80, 0x16, 0x31, 0x45), Color(0x00, 0x16, 0x31, 0x45));
									g.FillRectangle(&brush3, rectBounds.left, rectBounds.top, rectBounds.Width(), 2);

									LinearGradientBrush brush4(Point(rectBounds.left, rectBounds.top), Point(rectBounds.left+2, rectBounds.top), Color(0x80, 0x16, 0x31, 0x45), Color(0x00, 0x16, 0x31, 0x45));
									g.FillRectangle(&brush4, rectBounds.left, rectBounds.top, 2, rectBounds.Height());
								}
								else
								{
									LinearGradientBrush brush1(Point(rectBounds.left, rectBounds.top), Point(rectBounds.left, rectBounds.bottom), Color(0xFA, 0xFD, 0xFE), Color(0xE8, 0xF5, 0xFC));
									g.FillRectangle(&brush1, rectBounds.left, rectBounds.top, rectBounds.Width(), rectBounds.Height());

									rectBounds.DeflateRect(1, 1);
									INT y = (rectBounds.top+rectBounds.bottom)/2;

									LinearGradientBrush brush2(Point(rectBounds.left, rectBounds.top-1), Point(rectBounds.left, y), Color(0xEA, 0xF6, 0xFD), Color(0xD7, 0xEF, 0xFC));
									g.FillRectangle(&brush2, rectBounds.left, rectBounds.top, rectBounds.Width(), y-rectBounds.top);

									LinearGradientBrush brush3(Point(rectBounds.left, y-1), Point(rectBounds.left, rectBounds.bottom), Color(0xBD, 0xE6, 0xFD), Color(0xA6, 0xD9, 0xF4));
									g.FillRectangle(&brush3, rectBounds.left, y, rectBounds.Width(), rectBounds.bottom-y);
								}
							}

						if (hdi.fmt & (HDF_SORTDOWN | HDF_SORTUP))
							m_SortIndicators.Draw(&dc, (hdi.fmt & HDF_SORTUP) ? 0 : 1, CPoint(rectItem.left+(rectItem.Width()-7)/2, rectItem.top+2), ILD_TRANSPARENT);

						rectItem.bottom -= 3;
						rectItem.top = rectItem.bottom-dc.GetTextExtent(_T("Wy")).cy;
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

						rectItem.right--;
					}

					rectItem.DeflateRect(4, 0);

					if (m_PressedItem==a)
						rectItem.OffsetRect(1, 1);

					UINT nFormat = DT_SINGLELINE | DT_END_ELLIPSIS | DT_VCENTER | DT_NOPREFIX;
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

					dc.SetTextColor(Themed ? 0x404040 : GetSysColor(COLOR_WINDOWTEXT));
					dc.DrawText(lpBuffer, rectItem, nFormat);

					if ((!Themed) && (hdi.fmt & (HDF_SORTDOWN | HDF_SORTUP)))
					{
						if ((hdi.fmt & HDF_JUSTIFYMASK)==HDF_RIGHT)
						{
							rectItem.left = rectItem.right-dc.GetTextExtent(lpBuffer, (INT)wcslen(lpBuffer)).cx-9;
						}
						else
						{
							rectItem.left += dc.GetTextExtent(lpBuffer, (INT)wcslen(lpBuffer)).cx+2;
						}

						if ((rectItem.left>1) && (rectItem.left+5<rectItem.right))
							m_SortIndicators.Draw(&dc, (hdi.fmt & HDF_SORTUP) ? 2 : 3, CPoint(rectItem.left, rectItem.top+(rectItem.Height()-3)/2), ILD_TRANSPARENT);
					}
				}
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
				WCHAR TooltipTextBuffer[256];
				i.mask = HDI_TEXT;
				i.pszText = TooltipTextBuffer;
				i.cchTextMax = 256;

				if (GetItem(m_TooltipItem, &i))
					if (TooltipTextBuffer[0]!=L'\0')
					{
						ClientToScreen(&point);
						m_TooltipCtrl.Track(point, NULL, NULL, _T(""), TooltipTextBuffer);
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
