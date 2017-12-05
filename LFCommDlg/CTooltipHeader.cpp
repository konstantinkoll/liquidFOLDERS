
// CTooltipHeader.cpp: Implementierung der Klasse CTooltipHeader
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CTooltipHeader
//

CIcons CTooltipHeader::m_SortIndicators;

CTooltipHeader::CTooltipHeader()
	: CHeaderCtrl()
{
	CONSTRUCTOR_TOOLTIP()

	m_Shadow = FALSE;
	m_PressedItem = m_TrackItem = -1;
}

BOOL CTooltipHeader::Create(CWnd* pParentWnd, UINT nID, BOOL Shadow)
{
	m_Shadow = Shadow;

	return CHeaderCtrl::Create(WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | HDS_FLAT | HDS_HORZ | HDS_FULLDRAG | HDS_BUTTONS | CCS_TOP | CCS_NOMOVEY | CCS_NODIVIDER, CRect(0, 0, 0, 0), pParentWnd, nID);
}

void CTooltipHeader::PreSubclassWindow()
{
	CHeaderCtrl::PreSubclassWindow();

	_AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	if (!pThreadState->m_pWndInit)
		Init();
}

void CTooltipHeader::Init()
{
	m_SortIndicators.Load(IDB_SORTINDICATORS, CSize(7, 4));

	SetFont(&LFGetApp()->m_DefaultFont);
}

void CTooltipHeader::SetShadow(BOOL Shadow)
{
	m_Shadow = Shadow;

	Invalidate();
}

INT CTooltipHeader::ItemAtPosition(CPoint point) const
{
	HDHITTESTINFO htt;
	htt.pt = point;

	return ((CHeaderCtrl*)this)->HitTest(&htt);
}

CPoint CTooltipHeader::PointAtPosition(CPoint /*point*/) const
{
	return CPoint(-1, -1);
}

LPCVOID CTooltipHeader::PtrAtPosition(CPoint /*point*/) const
{
	return NULL;
}

void CTooltipHeader::InvalidateItem(INT /*Index*/)
{
	Invalidate();
}

void CTooltipHeader::InvalidatePoint(const CPoint& /*point*/)
{
	Invalidate();
}

void CTooltipHeader::InvalidatePtr(LPCVOID /*Ptr*/)
{
	Invalidate();
}

void CTooltipHeader::ShowTooltip(const CPoint& point)
{
	WCHAR TooltipTextBuffer[256];

	HDITEMW Item;
	Item.mask = HDI_TEXT;
	Item.pszText = TooltipTextBuffer;
	Item.cchTextMax = 256;

	if (GetItem(m_HoverItem, &Item) && (TooltipTextBuffer[0]!=L'\0'))
		LFGetApp()->ShowTooltip(this, point, _T(""), TooltipTextBuffer);
}


IMPLEMENT_TOOLTIP_WHEEL(CTooltipHeader, CHeaderCtrl)

BEGIN_TOOLTIP_MAP(CTooltipHeader, CHeaderCtrl)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_MESSAGE(HDM_LAYOUT, OnLayout)
END_TOOLTIP_MAP()

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

	Graphics g(dc);

	// Background
	const BOOL Themed = IsCtrlThemed();

	dc.FillSolidRect(rect, Themed ? 0xFFFFFF : GetSysColor(COLOR_3DFACE));

	CFont* pOldFont = dc.SelectObject(GetFont());

	if (Themed)
	{
		dc.FillSolidRect(rect, 0xFFFFFF);

		CRect rectParent;
		GetParent()->GetClientRect(rectParent);

		Bitmap* pDivider = LFGetApp()->GetCachedResourceImage(IDB_DIVUP);

		g.DrawImage(pDivider, (rectParent.Width()-(INT)pDivider->GetWidth())/2+GetParent()->GetScrollPos(SB_HORZ), rect.Height()-(INT)pDivider->GetHeight());
	}
	else
	{
		dc.FillSolidRect(rect, GetSysColor(COLOR_3DFACE));
	}

	// Header items
	const INT Line = rect.Height()*2/5;
	LinearGradientBrush brush1(Point(0, 0), Point(0, Line), Color(0x00000000), Color(0x40000000));
	LinearGradientBrush brush2(Point(0, Line-1), Point(0, rect.Height()), Color(0x40000000), Color(0x00000000));

	Pen pen(Color(0x80FFFFFF));

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

						if (m_Shadow)
							rectBounds.top--;
					}

					DrawSubitemBackground(dc, g, rectBounds, Themed, m_PressedItem==a, (m_PressedItem==-1) && ((m_TrackItem==a) || ((m_TrackItem==-1) && (m_HoverItem==a))));

					if (Themed)
					{
						if (hdi.fmt & (HDF_SORTDOWN | HDF_SORTUP))
							m_SortIndicators.Draw(dc, rectItem.left+(rectItem.Width()-7)/2, rectItem.top+2, (hdi.fmt & HDF_SORTUP) ? 0 : 1);

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

					if (!Themed && (hdi.fmt & (HDF_SORTDOWN | HDF_SORTUP)))
					{
						INT Left;

						if ((hdi.fmt & HDF_JUSTIFYMASK)==HDF_RIGHT)
						{
							Left = rectItem.right-dc.GetTextExtent(lpBuffer, (INT)wcslen(lpBuffer)).cx-9;
						}
						else
						{
							Left = rectItem.left+dc.GetTextExtent(lpBuffer, (INT)wcslen(lpBuffer)).cx+2;
						}

						if ((Left>rectItem.left) && (Left+5<rectItem.right))
							m_SortIndicators.Draw(dc, Left, rectItem.top+(rectItem.Height()-3)/2, (hdi.fmt & HDF_SORTUP) ? 2 : 3);
					}
				}
			}
		}
	}

	if (m_Shadow && Themed)
		CTaskbar::DrawTaskbarShadow(g, rect);

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);

	dc.SelectObject(pOldFont);
	dc.SelectObject(pOldBitmap);
}

void CTooltipHeader::OnLButtonDown(UINT nFlags, CPoint point)
{
	HDHITTESTINFO htt;
	htt.pt = point;
	const INT Index = HitTest(&htt);

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

LRESULT CTooltipHeader::OnLayout(WPARAM wParam, LPARAM lParam)
{
	LRESULT Result = CHeaderCtrl::DefWindowProc(HDM_LAYOUT, wParam, lParam);

	LPHDLAYOUT pHL = (LPHDLAYOUT)lParam;
	if ((pHL->pwpos->cy) && (LFGetApp()->OSVersion==OS_XP))
		pHL->pwpos->cy += 4;

	pHL->prc->top = pHL->pwpos->cy;

	return Result;
}
