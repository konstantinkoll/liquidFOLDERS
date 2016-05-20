
// CBackstageBar.cpp: Implementierung der Klasse CBackstageBar
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CBackstageBar
//

#define NOPART     -2
#define VIEW       -1

HBITMAP BackstageBarIcons[2][3][6] = { NULL };

CBackstageBar::CBackstageBar(BOOL Small)
	: CWnd()
{
	if (Small)
	{
		p_Font = &LFGetApp()->m_SmallFont;
		m_IconSize = (p_Font->GetFontHeight()-3) | 1;

		if (m_IconSize<9)
			m_IconSize = 9;

		if (m_IconSize>27)
			m_IconSize = 27;

		m_ButtonSize = m_IconSize+6;
	}
	else
	{
		p_Font = &LFGetApp()->m_DefaultFont;
		m_ButtonSize = GetPreferredHeight()-1;
		m_IconSize = (m_ButtonSize-4) & ~3;

		if (m_IconSize<16)
			m_IconSize = 16;

		if (m_IconSize>32)
			m_IconSize = 32;
	}

	m_Small = Small;
	m_Hover = m_Pressed = NOPART;
}

BOOL CBackstageBar::Create(CWnd* pParentWnd, UINT nID, INT Spacer, BOOL ReverseOrder)
{
	m_Spacer = Spacer;
	m_ReverseOrder = ReverseOrder;

	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW, LFGetApp()->LoadStandardCursor(IDC_ARROW));

	return CWnd::CreateEx(WS_EX_NOACTIVATE, className, _T(""), WS_CHILD | WS_BORDER | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE, CRect(0, 0, 0, 0), pParentWnd, nID);
}

UINT CBackstageBar::GetPreferredHeight()
{
	return LFGetApp()->m_DefaultFont.GetFontHeight()+7;
}

UINT CBackstageBar::GetPreferredWidth() const
{
	UINT Width = m_BarItems.m_ItemCount*m_ButtonSize+2;

	if (m_BarItems.m_ItemCount)
		Width += m_BarItems.m_ItemCount*m_Spacer;

	return Width;
}

void CBackstageBar::Reset()
{
	m_BarItems.m_ItemCount = 0;
	m_Hover = m_Pressed = NOPART;
}

void CBackstageBar::AddItem(UINT Command, INT IconID, INT PreferredWidth, BOOL Red, WCHAR* pName)
{
	BarItem Item;
	ZeroMemory(&Item, sizeof(Item));

	Item.Command = Command;
	Item.IconID = IconID;
	Item.PreferredWidth = PreferredWidth ? PreferredWidth : m_ButtonSize;
	Item.Red = Red;
	Item.Enabled = TRUE;

	if (pName)
		wcscpy_s(Item.Name, 256, pName);

	m_BarItems.AddItem(Item);
}

void CBackstageBar::AdjustLayout()
{
	if (!m_BarItems.m_ItemCount)
		return;

	CRect rect;
	GetClientRect(rect);

	// Reset width
	for (UINT a=0; a<m_BarItems.m_ItemCount; a++)
		m_BarItems[a].Width = 0;

	INT UnallocatedWidth = rect.Width()-2-(m_BarItems.m_ItemCount-1)*m_Spacer;

	// Calc layout
Iterate:
	INT TooSmallCount = 0;
	INT Chunk = UnallocatedWidth;

	for (UINT a=0; a<m_BarItems.m_ItemCount; a++)
	{
		INT Diff = m_BarItems[a].PreferredWidth-m_BarItems[a].Width;

		if (Diff>0)
		{
			Chunk = min(Chunk, Diff);
			TooSmallCount++;
		}
	}

	if (TooSmallCount)
	{
		Chunk = min(Chunk, UnallocatedWidth/TooSmallCount);

		if (Chunk>0)
		{
			for (UINT a=0; a<m_BarItems.m_ItemCount; a++)
				if (m_BarItems[a].Width<m_BarItems[a].PreferredWidth)
				{
					m_BarItems[a].Width += Chunk;
					UnallocatedWidth -= Chunk;
				}

			goto Iterate;
		}
	}

	// Set layout
	INT Left = 1;
	for (UINT a=0; a<m_BarItems.m_ItemCount; a++)
	{
		BarItem* pBarItem = &m_BarItems[m_ReverseOrder ? m_BarItems.m_ItemCount-a-1 : a];

		pBarItem->Left = Left;
		pBarItem->Right = pBarItem->Left+pBarItem->Width;

		Left += pBarItem->Width+m_Spacer;
	}

	Invalidate();
}

INT CBackstageBar::HitTest(const CPoint& point) const
{
	CRect rect;
	GetClientRect(rect);

	if (!rect.PtInRect(point))
		return NOPART;

	if ((point.y>=1) && (point.y<rect.bottom-1))
		for (UINT a=0; a<m_BarItems.m_ItemCount; a++)
			if ((m_Pressed<0) || (m_Pressed==(INT)a))
				if (m_BarItems[a].Enabled)
					if ((point.x>=m_BarItems[a].Left) && (point.x<m_BarItems[a].Right))
						return a;

	return VIEW;
}

HBITMAP CBackstageBar::LoadMaskedIcon(UINT nID, INT Size, COLORREF clr)
{
	// RGB to BGR
	clr = (_byteswap_ulong(clr) >> 8) | 0xFF000000;

	HBITMAP hBitmap = CreateTransparentBitmap(Size, Size);

	// Get mask from icon
	HICON hIcon = (HICON)LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(nID), IMAGE_ICON, Size, Size, LR_SHARED);

	CDC dc;
	dc.CreateCompatibleDC(NULL);
	HBITMAP hOldBitmap = (HBITMAP)dc.SelectObject(hBitmap);

	DrawIconEx(dc, 0, 0, hIcon, Size, Size, 0, NULL, DI_NORMAL);
	DestroyIcon(hIcon);

	dc.SelectObject(hOldBitmap);

	// Set colors
	BITMAP Bitmap;
	if (GetObject(hBitmap, sizeof(Bitmap), &Bitmap))
		for (LONG Row=0; Row<Bitmap.bmHeight; Row++)
		{
			BYTE* Ptr = (BYTE*)Bitmap.bmBits+Bitmap.bmWidthBytes*Row;

			for (LONG Column=0; Column<Bitmap.bmWidth; Column++)
			{
				const BYTE Alpha = *(Ptr+3);

				switch (Alpha)
				{
				case 0xFF:
					*((COLORREF*)Ptr) = clr;

				case 0x00:
					break;

				default:
					*Ptr = (clr & 0xFF)*Alpha/255;
					*(Ptr+1) = ((clr>>8) & 0xFF)*Alpha/255;
					*(Ptr+2) = (BYTE)(clr>>16)*Alpha/255;
				}

				Ptr += 4;
			}
		}

	return hBitmap;
}

void CBackstageBar::DrawItem(CDC& dc, CRect& rectItem, UINT Index, UINT State, BOOL /*Themed*/)
{
	ASSERT(State<3);

	const INT IconID = m_BarItems[Index].IconID;
	if ((IconID>=0) && (IconID<6))
	{
		// Icon
		HBITMAP* pBitmap = &BackstageBarIcons[m_Small ? 0 : 1][State][IconID];

		if (!*pBitmap)
			*pBitmap = LoadMaskedIcon(IDI_BACKSTAGEBAR_FIRST+IconID, m_IconSize, dc.GetTextColor());

		CDC dcIcon;
		dcIcon.CreateCompatibleDC(&dc);
		HBITMAP hOldBitmap = (HBITMAP)dcIcon.SelectObject(*pBitmap);

		const BLENDFUNCTION BF = { AC_SRC_OVER, 0, State<2 ? 0xFF : 0xA0, AC_SRC_ALPHA };
		AlphaBlend(dc, rectItem.left+(rectItem.Width()-m_IconSize)/2, rectItem.top+(rectItem.Height()-m_IconSize)/2, m_IconSize, m_IconSize, dcIcon, 0, 0, m_IconSize, m_IconSize, BF);

		dcIcon.SelectObject(hOldBitmap);
	}
}


BEGIN_MESSAGE_MAP(CBackstageBar, CWnd)
	ON_WM_NCCALCSIZE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_THEMECHANGED()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONUP()
	ON_WM_SIZE()
END_MESSAGE_MAP()

void CBackstageBar::OnNcCalcSize(BOOL /*bCalcValidRects*/, NCCALCSIZE_PARAMS* /*lpncsp*/)
{
}

BOOL CBackstageBar::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CBackstageBar::OnPaint()
{
	CPaintDC pDC(this);

	CRect rect;
	GetClientRect(rect);

	CDC dc;
	dc.CreateCompatibleDC(&pDC);
	dc.SetBkMode(TRANSPARENT);

	HBITMAP hBitmap = CreateTransparentBitmap(rect.Width(), rect.Height());
	HBITMAP hOldBitmap = (HBITMAP)dc.SelectObject(hBitmap);

	// Background
	FillRect(dc, rect, (HBRUSH)GetParent()->SendMessage(WM_CTLCOLORBTN, (WPARAM)dc.m_hDC, (LPARAM)m_hWnd));

	// Border
	BOOL Themed = IsCtrlThemed();
	if (!Themed)
		dc.Draw3dRect(rect, 0x000000, 0x000000);

	Graphics g(dc);
	g.SetSmoothingMode(SmoothingModeAntiAlias);

	// Items
	CFont* pOldFont = dc.SelectObject(p_Font);

	for (UINT a=0; a<m_BarItems.m_ItemCount; a++)
	{
		const BarItem* pBarItem = &m_BarItems[a];
		CRect rectItem(pBarItem->Left, 0, pBarItem->Right, rect.bottom-1);

		DrawBackstageButtonBackground(dc, g, rectItem, m_Hover==(INT)a, m_Pressed==(INT)a, pBarItem->Enabled, Themed, pBarItem->Red);
		DrawItem(dc, rectItem, a, pBarItem->Enabled ? (m_Hover==(INT)a) || (m_Pressed==(INT)a) ? 1 : 0 : 2, Themed);
	}

	dc.SelectObject(pOldFont);

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);

	dc.SelectObject(hOldBitmap);
	DeleteObject(hBitmap);
}

LRESULT CBackstageBar::OnThemeChanged()
{
	HBITMAP* pBitmap = &BackstageBarIcons[0][0][0];

	for (UINT a=0; a<sizeof(BackstageBarIcons)/sizeof(HBITMAP); a++)
	{
		DeleteObject(pBitmap[a]);
		pBitmap[a] = NULL;
	}

	return NULL;
}

void CBackstageBar::OnMouseMove(UINT /*nFlags*/, CPoint point)
{
	if (m_Hover==NOPART)
	{
		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_LEAVE;
		tme.dwHoverTime = HOVERTIME;
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

void CBackstageBar::OnMouseLeave()
{
	m_Hover = m_Pressed = NOPART;
	Invalidate();
}

void CBackstageBar::OnLButtonDown(UINT /*nFlags*/, CPoint point)
{
	m_Hover = m_Pressed = HitTest(point);
	Invalidate();

	if (m_Hover>=0)
		SetCapture();
}

void CBackstageBar::OnLButtonUp(UINT /*nFlags*/, CPoint point)
{
	INT ID = HitTest(point);
	if ((ID==m_Pressed) && (ID>=0))
		OnClickButton(ID);

	m_Hover = ID;
	m_Pressed = NOPART;
	Invalidate();

	ReleaseCapture();
}

void CBackstageBar::OnRButtonUp(UINT nFlags, CPoint point)
{
	m_Hover = m_Pressed = NOPART;
	ReleaseCapture();

	Invalidate();
	UpdateWindow();

	CRect rect;
	GetWindowRect(rect);

	point += rect.TopLeft();
	GetParent()->ScreenToClient(&point);

	GetParent()->SendMessage(WM_RBUTTONUP, (WPARAM)nFlags, (LPARAM)((point.y<<16) | (point.x & 0xFFFF)));
}

void CBackstageBar::OnSize(UINT nType, INT cx, INT cy)
{
	CWnd::OnSize(nType, cx, cy);

	AdjustLayout();
}
