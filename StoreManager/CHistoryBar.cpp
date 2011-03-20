
#include "stdafx.h"
#include "CHistoryBar.h"
#include "StoreManager.h"


// Breadcrumbs
//

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
	hTheme = NULL;
	m_Hover = FALSE;
	m_IsEmpty = TRUE;

	ENSURE(m_EmptyHint.LoadString(IDS_NONAVIGATION));
}

BOOL CHistoryBar::Create(CGlasWindow* pParentWnd, UINT nID)
{
	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW, LoadCursor(NULL, IDC_ARROW));

	const DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE;// | WS_TABSTOP;
	CRect rect;
	rect.SetRectEmpty();
	return CWnd::Create(className, _T(""), dwStyle, rect, pParentWnd, nID);
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
	item.Width = pDC->GetTextExtent(item.Name, wcslen(item.Name)).cx+2*MARGIN;

	m_Breadcrumbs.AddItem(item);
}

void CHistoryBar::SetHistory(LFFilter* ActiveFilter, BreadcrumbItem* BreadcrumbBack)
{
	m_IsEmpty = FALSE;
	m_Breadcrumbs.m_ItemCount = 0;

	CDC* dc = GetDC();
	CFont* pOldFont = dc->SelectObject(&theApp.m_DefaultFont);

	AddFilter(ActiveFilter, dc);
	while (BreadcrumbBack)
	{
		AddFilter(BreadcrumbBack->filter, dc);
		BreadcrumbBack = BreadcrumbBack->next;
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
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_THEMECHANGED()
	ON_WM_SIZE()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONUP()
	//ON_WM_SETFOCUS()
	//ON_WM_KILLFOCUS()
END_MESSAGE_MAP()

INT CHistoryBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	OnThemeChanged();

	return 0;
}

void CHistoryBar::OnDestroy()
{
	if (hTheme)
		theApp.zCloseThemeData(hTheme);

	CWnd::OnDestroy();
}

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

	Graphics g(dc);

	CGlasWindow* pCtrlSite = (CGlasWindow*)GetParent();
	pCtrlSite->DrawFrameBackground(&dc, rectClient);
	const BYTE Alpha = /*m_Dropped ? 0xFF : */(m_Hover && !m_IsEmpty) ? 0xF0 : 0xD0;

	CRect rectContent(rectClient);
	if (hTheme)
	{
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
		if (/*m_Dropped || */(GetFocus()==this))
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

	// Reload button

	/*CRect rectClip(rectContent);
	rectClip.left = rectClip.right-GetSystemMetrics(SM_CXHSCROLL);
	CRect rectArrow(rectClip);

	if (hTheme)
	{
		// Hack to achieve the same style as Windows Explorer
		if (p_App->OSVersion>OS_XP)
		{
			rectArrow.InflateRect(1, 1);
			if (m_Hover)
			{
				rectClip.left--;
			}
			else
			{
				rectArrow.InflateRect(1, 1);
			}
		}

		p_App->zDrawThemeBackground(hTheme, dc, CP_DROPDOWNBUTTON, m_Pressed ? CBXS_PRESSED : (m_Hover && !m_Dropped) ? CBXS_HOT : CBXS_NORMAL, rectArrow, rectClip);
	}
	else
	{
		if (m_Hover || m_Pressed || m_Dropped)
			dc.DrawFrameControl(rectArrow, DFC_BUTTON, DFCS_TRANSPARENT | 16 | DFCS_HOT | (m_Pressed ? DFCS_PUSHED : 0));

		dc.DrawFrameControl(rectArrow, DFC_MENU, DFCS_TRANSPARENT | 16 | (m_Pressed ? DFCS_PUSHED : (m_Hover && !m_Dropped) ? DFCS_HOT : DFCS_FLAT));
		rectClip.left--;

		if (m_Hover || m_Pressed || m_Dropped)
			dc.FillSolidRect(rectClip.left, rectClip.top, 1, rectClip.Height(), GetSysColor(COLOR_3DFACE));
	}*/

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
			//TODO

			rectItemText.DeflateRect(MARGIN, 0);
			if (rectItemText.left<BORDER/2)
				rectItemText.left = BORDER/2;

			dc.DrawText(hi->Name, wcslen(hi->Name), rectItemText, DT_SINGLELINE | DT_VCENTER | DT_RIGHT);

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

LRESULT CHistoryBar::OnThemeChanged()
{
	if (theApp.m_ThemeLibLoaded)
	{
		if (hTheme)
			theApp.zCloseThemeData(hTheme);

		hTheme = theApp.zOpenThemeData(m_hWnd, VSCLASS_COMBOBOX);
	}

	return TRUE;
}

void CHistoryBar::OnSize(UINT nType, INT cx, INT cy)
{
	CWnd::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CHistoryBar::OnMouseMove(UINT /*nFlags*/, CPoint /*point*/)
{
	if (!m_Hover)
	{
		m_Hover = TRUE;
		Invalidate();

		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_LEAVE;
		tme.dwHoverTime = LFHOVERTIME;
		tme.hwndTrack = m_hWnd;
		TrackMouseEvent(&tme);
	}
}

void CHistoryBar::OnMouseLeave()
{
	m_Hover = FALSE;
	Invalidate();
}

void CHistoryBar::OnLButtonDown(UINT /*nFlags*/, CPoint /*point*/)
{
/*	SetFocus();

	if (m_Dropped)
	{
		OnCloseDropdown();
	}
	else
	{
		m_Pressed = TRUE;
		OnOpenDropdown();
	}*/
}

void CHistoryBar::OnLButtonUp(UINT /*nFlags*/, CPoint /*point*/)
{
	/*if (m_Pressed)
	{
		m_Pressed = FALSE;
		Invalidate();

		ReleaseCapture();
	}*/
}

void CHistoryBar::OnRButtonUp(UINT nFlags, CPoint point)
{
	CRect rect;
	GetWindowRect(rect);
	point += rect.TopLeft();
	GetParent()->ScreenToClient(&point);

	GetParent()->SendMessage(WM_RBUTTONUP, (WPARAM)nFlags, (LPARAM)((point.y<<16) | point.x));
}
