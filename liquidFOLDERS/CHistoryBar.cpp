
// CHistoryBar.cpp: Implementierung der Klasse CHistoryBar
//

#include "stdafx.h"
#include "liquidFOLDERS.h"


// Breadcrumbs
//

#define NOPART     -3
#define VIEW       -2
#define RELOAD     -1

void AddBreadcrumbItem(BreadcrumbItem** bi, LFFilter* pFilter, FVPersistentData& Data)
{
	BreadcrumbItem* add = new BreadcrumbItem;
	add->pNext = *bi;
	add->pFilter = pFilter;
	add->Data = Data;
	*bi = add;
}

void ConsumeBreadcrumbItem(BreadcrumbItem** bi, LFFilter** ppFilter, FVPersistentData* Data)
{
	*ppFilter = NULL;
	ZeroMemory(Data, sizeof(FVPersistentData));

	if (*bi)
	{
		*ppFilter = (*bi)->pFilter;
		*Data = (*bi)->Data;

		BreadcrumbItem* pVictim = *bi;
		*bi = (*bi)->pNext;
		delete pVictim;
	}
}

void DeleteBreadcrumbs(BreadcrumbItem** bi)
{
	while (*bi)
	{
		BreadcrumbItem* pVictim = *bi;
		*bi = (*bi)->pNext;

		LFFreeFilter(pVictim->pFilter);
		delete pVictim;
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
	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW, theApp.LoadStandardCursor(IDC_ARROW));

	CRect rect;
	rect.SetRectEmpty();
	return CWnd::Create(className, _T(""), WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE, rect, pParentWnd, nID);
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
	HistoryItem Item;
	ZeroMemory(&Item, sizeof(Item));

	wcscpy_s(Item.Name, 256, Filter->ResultName);
	Item.Width = pDC->GetTextExtent(Item.Name, (INT)wcslen(Item.Name)).cx+2*MARGIN;

	m_Breadcrumbs.AddItem(Item);
}

void CHistoryBar::SetHistory(LFFilter* ActiveFilter, BreadcrumbItem* Breadcrumbs)
{
	m_IsEmpty = FALSE;
	m_Breadcrumbs.m_ItemCount = 0;

	CDC* pDC = GetDC();
	CFont* pOldFont = pDC->SelectObject(&theApp.m_DefaultFont);

	AddFilter(ActiveFilter, pDC);
	while (Breadcrumbs)
	{
		AddFilter(Breadcrumbs->pFilter, pDC);
		Breadcrumbs = Breadcrumbs->pNext;
	}

	pDC->SelectObject(pOldFont);
	ReleaseDC(pDC);

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

	HBITMAP hBitmap = CreateTransparentBitmap(rectClient.Width(), rectClient.Height());
	HBITMAP hOldBitmap = (HBITMAP)dc.SelectObject(hBitmap);

	BOOL Themed = IsCtrlThemed();

	CGlassWindow* pCtrlSite = (CGlassWindow*)GetParent();
	pCtrlSite->DrawFrameBackground(&dc, rectClient);
	const BYTE Alpha = (LFGetApp()->OSVersion==OS_Eight) ? 0xFF : (((m_Hover!=NOPART) || (m_Pressed!=NOPART)) && !m_IsEmpty) ? 0xF0 : 0xD0;

	CRect rectContent(rectClient);
	if (Themed)
	{
		Graphics g(dc);

		CRect rectBounds(rectContent);
		rectContent.DeflateRect(2, 2);

		SolidBrush brush1(Color(Alpha, 0xFF, 0xFF, 0xFF));
		g.FillRectangle(&brush1, rectContent.left, rectContent.top, rectContent.Width(), rectContent.Height());
		g.SetSmoothingMode(SmoothingModeAntiAlias);

		GraphicsPath path;
		CreateRoundRectangle(rectBounds, 2, path);

		Pen pen(Color(0x40, 0xFF, 0xFF, 0xFF));
		g.DrawPath(&pen, &path);

		rectBounds.DeflateRect(1, 1);

		if (LFGetApp()->OSVersion==OS_Eight)
		{
			CreateRoundRectangle(rectBounds, 0, path);

			Pen pen(Color(0x40, 0x38, 0x38, 0x38));
			g.DrawPath(&pen, &path);
		}
		else
		{
			CreateRoundRectangle(rectBounds, 1, path);

			LinearGradientBrush brush2(Point(0, rectBounds.top), Point(0, rectBounds.bottom), Color(Alpha, 0x50, 0x50, 0x50), Color(Alpha, 0xB0, 0xB0, 0xB0));
			pen.SetBrush(&brush2);

			g.DrawPath(&pen, &path);
		}
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

	if (m_IsEmpty)
	{
		CRect rectText(rectContent);
		rectText.DeflateRect(BORDER, 0);

		pOldFont = dc.SelectObject(&theApp.m_ItalicFont);
		dc.SetTextColor(pCtrlSite->GetDesign()==GWD_DEFAULT ? GetSysColor(COLOR_3DSHADOW) : 0x808080);
		dc.DrawText(m_EmptyHint, rectText, DT_SINGLELINE | DT_END_ELLIPSIS | DT_VCENTER);
	}
	else
	{
		pOldFont = dc.SelectObject(&theApp.m_DefaultFont);
		dc.SetTextColor(pCtrlSite->GetDesign()==GWD_DEFAULT ? GetSysColor(COLOR_WINDOWTEXT) : 0x000000);

		for (UINT a=0; a<m_Breadcrumbs.m_ItemCount; a++)
		{
			// Item
			HistoryItem* pHistoryItem = &m_Breadcrumbs.m_Items[a];

			CRect rectItem(rectContent);
			rectItem.left = pHistoryItem->Left;
			rectItem.right = pHistoryItem->Right;

			CRect rectItemText(rectItem);

			if (rectItem.left<BORDER/2)
				rectItem.left = BORDER/2;

			BOOL Selected = (m_Hover==(INT)a) && (m_Pressed==(INT)a);

			DrawSubitemBackground(dc, rectItem, Themed, Selected, (m_Hover==(INT)a) && (m_Pressed==NOPART), TRUE);

			rectItemText.DeflateRect(MARGIN, 0);
			if (rectItemText.left<BORDER/2)
				rectItemText.left = BORDER/2;
			if (Selected)
				rectItemText.OffsetRect(1, 1);

			dc.DrawText(pHistoryItem->Name, (INT)wcslen(pHistoryItem->Name), rectItemText, DT_SINGLELINE | DT_VCENTER | DT_RIGHT);

			// Arrow
			if (a>0)
			{
				CRect rectArrow(rectContent);
				rectArrow.left = pHistoryItem->Right;
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
	GetObject(hBitmap, sizeof(BITMAP), &bmp);
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
	DeleteObject(hBitmap);
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
