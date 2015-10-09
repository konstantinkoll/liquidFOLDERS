
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
		LFGetApp()->HideTooltip();
		break;
	}

	return CHeaderCtrl::PreTranslateMessage(pMsg);
}

void CTooltipHeader::Init()
{
	m_SortIndicators.Create(IDB_SORTINDICATORS, 7, 4);
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
	ON_MESSAGE(HDM_LAYOUT, OnLayout)
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

	CBitmap MemBitmap;
	MemBitmap.CreateCompatibleBitmap(&pDC, rect.Width(), rect.Height());
	CBitmap* pOldBitmap = dc.SelectObject(&MemBitmap);

	BOOL Themed = IsCtrlThemed();
	dc.FillSolidRect(rect, Themed ? 0xFFFFFF : GetSysColor(COLOR_3DFACE));

	CFont* pOldFont = dc.SelectObject(GetFont());
	Graphics g(dc);

	if (Themed)
	{
		dc.FillSolidRect(rect, 0xFFFFFF);

		CRect rectParent;
		GetParent()->GetClientRect(rectParent);

		CGdiPlusBitmap* pDivider = LFGetApp()->GetCachedResourceImage(IDB_DIVUP, _T("PNG"));
		g.DrawImage(pDivider->m_pBitmap, (rectParent.Width()-(INT)pDivider->m_pBitmap->GetWidth())/2+GetParent()->GetScrollPos(SB_HORZ), rect.Height()-(INT)pDivider->m_pBitmap->GetHeight());
	}
	else
	{
		dc.FillSolidRect(rect, GetSysColor(COLOR_3DFACE));
	}

	const INT Line = rect.Height()*2/5;
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
					if ((rectItem.left==0) && (lpBuffer[0]!=L'\0'))
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
					CRect rectBounds(rectItem);

					if (Themed)
					{
						rectBounds.InflateRect(1, 0);

						if (rectBounds.left<0)
							rectBounds.left = 0;
					}

					DrawSubitemBackground(dc, rectBounds, Themed, m_PressedItem==a, (m_PressedItem==-1) && ((m_TrackItem==a) || ((m_TrackItem==-1) && (m_HoverItem==a))));

					if (Themed)
					{
						if (hdi.fmt & (HDF_SORTDOWN | HDF_SORTUP))
							m_SortIndicators.Draw(&dc, (hdi.fmt & HDF_SORTUP) ? 0 : 1, CPoint(rectItem.left+(rectItem.Width()-7)/2, rectItem.top+2), ILD_TRANSPARENT);

						rectItem.bottom -= 3;
						rectItem.top = rectItem.bottom-dc.GetTextExtent(_T("Wy")).cy;
					}
					else
					{
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
	INT Index = HitTest(&htt);
	m_PressedItem = (htt.flags==HHT_ONHEADER) ? Index : -1;
	m_TrackItem = ((htt.flags==HHT_ONDIVIDER) || (htt.flags==HHT_ONDIVOPEN)) ? Index : -1;

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
		if ((LFGetApp()->IsTooltipVisible()) && (m_HoverItem!=m_TooltipItem))
			LFGetApp()->HideTooltip();

	if (m_PressedItem==-1)
		Invalidate();

	CHeaderCtrl::OnMouseMove(nFlags, point);
}

void CTooltipHeader::OnMouseLeave()
{
	LFGetApp()->HideTooltip();
	m_Hover = FALSE;
	m_HoverItem = -1;

	CHeaderCtrl::OnMouseLeave();
	Invalidate();
}

void CTooltipHeader::OnMouseHover(UINT nFlags, CPoint point)
{
	if ((nFlags & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON | MK_XBUTTON1 | MK_XBUTTON2))==0)
	{
		HDHITTESTINFO htt;
		htt.pt = point;

		m_TooltipItem = HitTest(&htt);
		if (m_TooltipItem!=-1)
			if (!LFGetApp()->IsTooltipVisible())
			{
				HDITEMW i;
				WCHAR TooltipTextBuffer[256];
				i.mask = HDI_TEXT;
				i.pszText = TooltipTextBuffer;
				i.cchTextMax = 256;

				if (GetItem(m_TooltipItem, &i))
					if (TooltipTextBuffer[0]!=L'\0')
						LFGetApp()->ShowTooltip(this, point, _T(""), TooltipTextBuffer);
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
	tme.dwHoverTime = LFHOVERTIME;
	tme.hwndTrack = GetSafeHwnd();
	TrackMouseEvent(&tme);
}

LRESULT CTooltipHeader::OnLayout(WPARAM wParam, LPARAM lParam)
{
	LRESULT Result = CHeaderCtrl::DefWindowProc(HDM_LAYOUT, wParam, lParam);

	LPHDLAYOUT pHL = (LPHDLAYOUT)lParam;
	if ((pHL->pwpos->cy) && (LFGetApp()->OSVersion==OS_XP))
		pHL->pwpos->cy += 4;

	pHL->prc->top = pHL->pwpos->cy;

	return Result;
}
