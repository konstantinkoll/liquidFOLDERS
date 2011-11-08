
#include "stdafx.h"
#include "CHistoryBar.h"
#include "StoreManager.h"


// Breadcrumbs
//

#define NOPART     -3
#define VIEW       -2
#define RELOAD     -1

void AddBreadcrumbItem(BreadcrumbItem** bi, LFFilter* filter, FVPersistentData& data)
{
	BreadcrumbItem* add = new BreadcrumbItem;
	add->next = *bi;
	add->filter = filter;
	add->data = data;
	*bi = add;
}

void ConsumeBreadcrumbItem(BreadcrumbItem** bi, LFFilter** filter, FVPersistentData* data)
{
	*filter = NULL;
	ZeroMemory(data, sizeof(FVPersistentData));

	if (*bi)
	{
		*filter = (*bi)->filter;
		*data = (*bi)->data;
		BreadcrumbItem* victim = *bi;
		*bi = (*bi)->next;
		delete victim;
	}
}

void DeleteBreadcrumbs(BreadcrumbItem** bi)
{
	while (*bi)
	{
		BreadcrumbItem* victim = *bi;
		*bi = (*bi)->next;
		LFFreeFilter(victim->filter);
		delete victim;
	}
}


// CHistoryBar
//

#define BORDER          4
#define MARGIN          4

CHistoryBar::CHistoryBar()
	: CWnd()
{
	m_Hover = m_Pressed = NOPART;
	m_IsEmpty = TRUE;

	ENSURE(m_EmptyHint.LoadString(IDS_NONAVIGATION));
}

BOOL CHistoryBar::Create(CGlassWindow* pParentWnd, UINT nID)
{
	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW, LoadCursor(NULL, IDC_ARROW));

	const DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE;
	CRect rect;
	rect.SetRectEmpty();
	return CWnd::Create(className, _T(""), dwStyle, rect, pParentWnd, nID);
}

INT CHistoryBar::HitTest(CPoint point)
{
	CRect rect;
	GetClientRect(rect);

	if (!rect.PtInRect(point))
		return NOPART;

	if (((point.y>=2) && (point.y<rect.bottom-2)) || (!IsCtrlThemed() && (point.y>=1) && (point.y<rect.bottom-1)))
		for (UINT a=0; a<m_Breadcrumbs.m_ItemCount; a++)
			if ((point.x>=m_Breadcrumbs.m_Items[a].Left) && (point.x<m_Breadcrumbs.m_Items[a].Right))
				return a;

	return VIEW;
}

UINT CHistoryBar::GetPreferredHeight()
{
	LOGFONT lf;
	theApp.m_DefaultFont.GetLogFont(&lf);
	UINT h = max(abs(lf.lfHeight), GetSystemMetrics(SM_CYSMICON));

	return h+2*BORDER;
}

void CHistoryBar::AddFilter(LFFilter* Filter, CDC* pDC)
{
	HistoryItem item;
	ZeroMemory(&item, sizeof(item));

	wcscpy_s(item.Name, 256, Filter->Name);
	item.Width = pDC->GetTextExtent(item.Name, (INT)wcslen(item.Name)).cx+2*MARGIN;

	m_Breadcrumbs.AddItem(item);
}

void CHistoryBar::SetHistory(LFFilter* ActiveFilter, BreadcrumbItem* Breadcrumbs)
{
	m_IsEmpty = FALSE;
	m_Breadcrumbs.m_ItemCount = 0;

	CDC* dc = GetDC();
	CFont* pOldFont = dc->SelectObject(&theApp.m_DefaultFont);

	AddFilter(ActiveFilter, dc);
	while (Breadcrumbs)
	{
		AddFilter(Breadcrumbs->filter, dc);
		Breadcrumbs = Breadcrumbs->next;
	}

	dc->SelectObject(pOldFont);
	ReleaseDC(dc);

	AdjustLayout();
}

void CHistoryBar::AdjustLayout()
{
	CRect rect;
	GetClientRect(rect);

	INT Spacer = rect.Height()-2*BORDER;
	INT Width = 0;
	for (UINT a=0; a<m_Breadcrumbs.m_ItemCount; a++)
		Width += m_Breadcrumbs.m_Items[a].Width+((a>0) ? Spacer : 0);

	INT Right = min(rect.right-BORDER, rect.left+Width+BORDER);
	for (UINT a=0; a<m_Breadcrumbs.m_ItemCount; a++)
	{
		m_Breadcrumbs.m_Items[a].Right = Right;
		Right = m_Breadcrumbs.m_Items[a].Left = Right-m_Breadcrumbs.m_Items[a].Width;
		if (a<m_Breadcrumbs.m_ItemCount-1)
			Right -= Spacer;
	}

	Invalidate();
}


BEGIN_MESSAGE_MAP(CHistoryBar, CWnd)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONUP()
END_MESSAGE_MAP()

BOOL CHistoryBar::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CHistoryBar::OnPaint()
{
	CPaintDC pDC(this);

	CRect rectClient;
	GetClientRect(rectClient);

	CDC dc;
	dc.CreateCompatibleDC(&pDC);
	dc.SetBkMode(TRANSPARENT);

	BITMAPINFO dib = { 0 };
	dib.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	dib.bmiHeader.biWidth = rectClient.Width();
	dib.bmiHeader.biHeight = -rectClient.Height();
	dib.bmiHeader.biPlanes = 1;
	dib.bmiHeader.biBitCount = 32;
	dib.bmiHeader.biCompression = BI_RGB;

	HBITMAP hBmp = CreateDIBSection(dc, &dib, DIB_RGB_COLORS, NULL, NULL, 0);
	HBITMAP hOldBitmap = (HBITMAP)dc.SelectObject(hBmp);

	BOOL Themed = IsCtrlThemed();

	CGlassWindow* pCtrlSite = (CGlassWindow*)GetParent();
	pCtrlSite->DrawFrameBackground(&dc, rectClient);
	const BYTE Alpha = (((m_Hover!=NOPART) || (m_Pressed!=NOPART)) && !m_IsEmpty) ? 0xF0 : 0xD0;

	CRect rectContent(rectClient);
	if (Themed)
	{
		Graphics g(dc);

		CRect rectBounds(rectContent);
		rectBounds.right--;
		rectBounds.bottom--;

		rectContent.DeflateRect(2, 2);
		SolidBrush brush1(Color(Alpha, 0xFF, 0xFF, 0xFF));

		g.FillRectangle(&brush1, rectContent.left, rectContent.top, rectContent.Width(), rectContent.Height());
		g.SetSmoothingMode(SmoothingModeAntiAlias);

		GraphicsPath path;
		CreateRoundRectangle(rectBounds, 2, path);
		Pen pen(Color(0x40, 0xFF, 0xFF, 0xFF));
		g.DrawPath(&pen, &path);
		rectBounds.DeflateRect(1, 1);

		CreateRoundRectangle(rectBounds, 1, path);
		LinearGradientBrush brush2(Point(0, rectBounds.top), Point(0, rectBounds.bottom), Color(Alpha, 0x50, 0x50, 0x50), Color(Alpha, 0xB0, 0xB0, 0xB0));
		pen.SetBrush(&brush2);
		g.DrawPath(&pen, &path);
	}
	else
	{
		dc.Draw3dRect(rectContent, GetSysColor(COLOR_3DSHADOW), GetSysColor(COLOR_3DHIGHLIGHT));
		rectContent.DeflateRect(1, 1);
		if (GetFocus()==this)
		{
			dc.Draw3dRect(rectContent, 0x000000, GetSysColor(COLOR_3DFACE));
			rectContent.DeflateRect(1, 1);
			dc.FillSolidRect(rectContent, GetSysColor(COLOR_WINDOW));
		}
		else
		{
			rectContent.DeflateRect(1, 1);
		}
	}

	// Breadcrumbs
	CFont* pOldFont;
	COLORREF c1 = (pCtrlSite->GetDesign()==GWD_DEFAULT) ? GetSysColor(COLOR_WINDOWTEXT) : 0x000000;
	COLORREF c2 = (pCtrlSite->GetDesign()==GWD_DEFAULT) ? GetSysColor(COLOR_3DSHADOW) : 0x808080;

	if (m_IsEmpty)
	{
		CRect rectText(rectContent);
		rectText.DeflateRect(BORDER, 0);

		pOldFont = dc.SelectObject(&theApp.m_ItalicFont);
		dc.SetTextColor(c2);
		dc.DrawText(m_EmptyHint, rectText, DT_SINGLELINE | DT_END_ELLIPSIS | DT_VCENTER);
	}
	else
	{
		pOldFont = dc.SelectObject(&theApp.m_DefaultFont);
		dc.SetTextColor(c1);

		for (UINT a=0; a<m_Breadcrumbs.m_ItemCount; a++)
		{
			// Item
			HistoryItem* hi = &m_Breadcrumbs.m_Items[a];

			CRect rectItem(rectContent);
			rectItem.left = hi->Left;
			rectItem.right = hi->Right;
			CRect rectItemText(rectItem);

			if (rectItem.left<BORDER/2)
				rectItem.left = BORDER/2;

			BOOL Hover = (m_Hover==(INT)a) && (m_Pressed==NOPART);
			BOOL Pressed = (m_Hover==(INT)a) && (m_Pressed==(INT)a);
			if (Hover || Pressed)
				if (Themed)
				{
					COLORREF clr = Hover ? 0xB17F3C : 0x8B622C;
					dc.FillSolidRect(rectItem.left, rectItem.top, 1, rectItem.Height(), clr);
					dc.FillSolidRect(rectItem.right-1, rectItem.top, 1, rectItem.Height(), clr);

					Graphics g(dc);
					rectItem.DeflateRect(1, 0);

					if (Hover)
					{
						LinearGradientBrush brush1(Point(rectItem.left, rectItem.top), Point(rectItem.left, rectItem.bottom), Color(0xFA, 0xFD, 0xFE), Color(0xE8, 0xF5, 0xFC));
						g.FillRectangle(&brush1, rectItem.left, rectItem.top, rectItem.Width(), rectItem.Height());

						rectItem.DeflateRect(1, 1);
						INT y = (rectItem.top+rectItem.bottom)/2;

						LinearGradientBrush brush2(Point(rectItem.left, rectItem.top-1), Point(rectItem.left, y-1), Color(0xEA, 0xF6, 0xFD), Color(0xD7, 0xEF, 0xFC));
						g.FillRectangle(&brush2, rectItem.left, rectItem.top, rectItem.Width(), y-rectItem.top);

						LinearGradientBrush brush3(Point(rectItem.left, y-1), Point(rectItem.left, rectItem.bottom), Color(0xBD, 0xE6, 0xFD), Color(0xA6, 0xD9, 0xF4));
						g.FillRectangle(&brush3, rectItem.left, y, rectItem.Width(), rectItem.bottom-y);
					}
					else
					{
						dc.FillSolidRect(rectItem, 0xF6E4C2);

						INT y = (rectItem.top+rectItem.bottom)/2;

						LinearGradientBrush brush2(Point(rectItem.left, y-1), Point(rectItem.left, rectItem.bottom), Color(0xA9, 0xD9, 0xF2), Color(0x90, 0xCB, 0xEB));
						g.FillRectangle(&brush2, rectItem.left, y, rectItem.Width(), rectItem.bottom-y);

						LinearGradientBrush brush3(Point(rectItem.left, rectItem.top), Point(rectItem.left, rectItem.top+2), Color(0x80, 0x16, 0x31, 0x45), Color(0x00, 0x16, 0x31, 0x45));
						g.FillRectangle(&brush3, rectItem.left, rectItem.top, rectItem.Width(), 2);

						LinearGradientBrush brush4(Point(rectItem.left, rectItem.top), Point(rectItem.left+2, rectItem.top), Color(0x80, 0x16, 0x31, 0x45), Color(0x00, 0x16, 0x31, 0x45));
						g.FillRectangle(&brush4, rectItem.left, rectItem.top, 2, rectItem.Height());
					}
				}
				else
				{
					COLORREF c1 = GetSysColor(COLOR_3DHIGHLIGHT);
					COLORREF c2 = GetSysColor(COLOR_3DFACE);
					COLORREF c3 = GetSysColor(COLOR_3DSHADOW);
					COLORREF c4 = 0x000000;

					if (Pressed)
					{
						std::swap(c1, c4);
						std::swap(c2, c3);
					}

					dc.Draw3dRect(rectItem, c1, c4);
					rectItem.DeflateRect(1, 1);
					dc.Draw3dRect(rectItem, c2, c3);
				}

			rectItemText.DeflateRect(MARGIN, 0);
			if (rectItemText.left<BORDER/2)
				rectItemText.left = BORDER/2;
			if (Pressed)
				rectItemText.OffsetRect(1, 1);

			dc.DrawText(hi->Name, (INT)wcslen(hi->Name), rectItemText, DT_SINGLELINE | DT_VCENTER | DT_RIGHT);

			// Arrow
			if (a>0)
			{
				CRect rectArrow(rectContent);
				rectArrow.left = hi->Right;
				rectArrow.right = m_Breadcrumbs.m_Items[a-1].Left;

				INT cy = rectArrow.Height()/4;
				INT cx = (rectArrow.Width()-cy)/2;
				INT cc = (rectArrow.top+rectArrow.bottom)/2;
				for (INT y=0; y<cy; y++)
				{
					dc.MoveTo(rectArrow.left+cx, cc+y);
					dc.LineTo(rectArrow.left+cx+cy-y, cc+y);
					dc.MoveTo(rectArrow.left+cx, cc-y);
					dc.LineTo(rectArrow.left+cx+cy-y, cc-y);
				}
			}
		}
	}

	dc.SelectObject(pOldFont);

	// Set alpha
	BITMAP bmp;
	GetObject(hBmp, sizeof(BITMAP), &bmp);
	BYTE* pBits = ((BYTE*)bmp.bmBits)+4*(rectContent.top*rectClient.Width()+rectContent.left);
	for (INT row=rectContent.top; row<rectContent.bottom; row++)
	{
		for (INT col=rectContent.left; col<rectContent.right; col++)
		{
			*(pBits+3) = Alpha;
			pBits += 4;
		}
		pBits += 4*(rectClient.Width()-rectContent.Width());
	}

	pDC.BitBlt(0, 0, rectClient.Width(), rectClient.Height(), &dc, 0, 0, SRCCOPY);

	dc.SelectObject(hOldBitmap);
	DeleteObject(hBmp);
}

void CHistoryBar::OnSize(UINT nType, INT cx, INT cy)
{
	CWnd::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CHistoryBar::OnMouseMove(UINT /*nFlags*/, CPoint point)
{
	if (m_Hover==NOPART)
	{
		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_LEAVE;
		tme.dwHoverTime = LFHOVERTIME;
		tme.hwndTrack = m_hWnd;
		TrackMouseEvent(&tme);
	}

	INT Hover = HitTest(point);
	if (Hover!=m_Hover)
	{
		m_Hover = Hover;
		Invalidate();
	}
}

void CHistoryBar::OnMouseLeave()
{
	m_Hover = m_Pressed = NOPART;
	Invalidate();
}

void CHistoryBar::OnLButtonDown(UINT /*nFlags*/, CPoint point)
{
	m_Hover = m_Pressed = HitTest(point);
	Invalidate();

	if (m_Hover>=0)
		SetCapture();
}

void CHistoryBar::OnLButtonUp(UINT /*nFlags*/, CPoint point)
{
	INT ID = HitTest(point);
	if (ID==m_Pressed)
		switch (ID)
		{
		case NOPART:
		case VIEW:
			break;
		case RELOAD:
		case 0:
			GetOwner()->PostMessage(WM_RELOAD);
			break;
		default:
			GetOwner()->PostMessage(WM_NAVIGATEBACK, (WPARAM)ID);
			break;
		}

	m_Hover = ID;
	m_Pressed = NOPART;
	Invalidate();

	ReleaseCapture();
}

void CHistoryBar::OnRButtonUp(UINT nFlags, CPoint point)
{
	m_Hover = m_Pressed = NOPART;
	ReleaseCapture();

	Invalidate();
	UpdateWindow();

	CRect rect;
	GetWindowRect(rect);
	point += rect.TopLeft();
	GetParent()->ScreenToClient(&point);

	GetParent()->SendMessage(WM_RBUTTONUP, (WPARAM)nFlags, (LPARAM)((point.y<<16) | point.x));
}
