
// CBackstageSidebar.cpp: Implementierung der Klasse CBackstageSidebar
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CBackstageSidebar
//

#define BORDER         6
#define BORDERLEFT     BACKSTAGERADIUS-BORDER
#define SHADOW         10

HBITMAP hShadow = NULL;

CBackstageSidebar::CBackstageSidebar()
	: CFrontstageWnd()
{
	p_ButtonIcons = p_TooltipIcons = NULL;
	m_Width = m_CountWidth = m_IconSize = 0;
	m_SelectedItem = m_HotItem = m_PressedItem = -1;
	m_Hover = m_Keyboard = m_ShowCounts = FALSE;
}

BOOL CBackstageSidebar::Create(CWnd* pParentWnd, CIcons& LargeIcons, UINT LargeResID, CIcons& SmallIcons, UINT SmallResID, UINT nID, BOOL ShowCounts)
{
	// Load icons
	m_IconSize = LFGetApp()->m_DefaultFont.GetFontHeight()>=24 ? 32 : 16;

	if (m_IconSize<32)
	{
		p_ButtonIcons = &SmallIcons;
		p_ButtonIcons->Load(SmallResID, m_IconSize);
	}
	else
	{
		p_ButtonIcons = &LargeIcons;
	}

	p_TooltipIcons = &LargeIcons;
	p_TooltipIcons->Load(LargeResID, 32);

	// Sidebar with numbers?
	m_ShowCounts = ShowCounts;

	if (ShowCounts)
		m_CountWidth = LFGetApp()->m_SmallBoldFont.GetTextExtent(_T("888W")).cx+2*BORDER+SHADOW/2;

	// Create
	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, LFGetApp()->LoadStandardCursor(IDC_ARROW));

	return CFrontstageWnd::Create(className, _T(""), WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_TABSTOP, CRect(0, 0, 0, 0), pParentWnd, nID);
}

BOOL CBackstageSidebar::PreTranslateMessage(MSG* pMsg)
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

void CBackstageSidebar::AddItem(BOOL Selectable, UINT CmdID, INT IconID, LPCWSTR Caption, LPCWSTR Hint, COLORREF Color)
{
	// Hinzufügen
	SidebarItem Item;
	ZeroMemory(&Item, sizeof(Item));
	Item.Selectable = Selectable;
	Item.CmdID = CmdID;
	Item.IconID = IconID;

	if (Caption)
		if (Selectable)
		{
			wcscpy_s(Item.Caption, 256, Caption);
			Item.Color = Color;
		}
		else
		{
			for (WCHAR* pDst=Item.Caption; *Caption; )
				*(pDst++) = (WCHAR)towupper(*(Caption++));
		}

	if (Hint)
		wcscpy_s(Item.Hint, 256, Hint);

	// Metrik
	CSize Size;

	if (Item.Caption[0]==L'\0')
	{
		Size.cx = 0;
		Size.cy = BORDER+2;
	}
	else
	{
		Size = (Selectable ? &LFGetApp()->m_LargeFont : &LFGetApp()->m_SmallBoldFont)->GetTextExtent(Item.Caption);

		if (Selectable)
		{
			Size.cx += m_IconSize+3*BORDER+BORDERLEFT+m_CountWidth+SHADOW/2;
			Size.cy = max(m_IconSize, Size.cy)+2*BORDER;
		}
		else
		{
			Size.cx += 2*BORDER+BORDERLEFT+SHADOW/2;
			Size.cy += BORDER+1;
		}
	}

	m_Width = max(m_Width, Size.cx);
	Item.Height = Size.cy;

	m_Items.AddItem(Item);
}

void CBackstageSidebar::AddCommand(UINT CmdID, INT IconID, LPCWSTR Caption, LPCWSTR Hint, COLORREF Color)
{
	AddItem(TRUE, CmdID, IconID, Caption, Hint, Color);
}

void CBackstageSidebar::AddCaption(LPCWSTR Caption)
{
	AddItem(FALSE, 0, -1, Caption, L"");
}

void CBackstageSidebar::AddCaption(UINT ResID)
{
	AddCaption(CString((LPCSTR)ResID));
}

void CBackstageSidebar::ResetCounts()
{
	for (UINT a=0; a<m_Items.m_ItemCount; a++)
		m_Items.m_Items[a].Count = 0;

	Invalidate();
}

void CBackstageSidebar::SetCount(UINT CmdID, UINT Count)
{
	for (UINT a=0; a<m_Items.m_ItemCount; a++)
		if (m_Items.m_Items[a].CmdID==CmdID)
		{
			m_Items.m_Items[a].Count = Count;
			InvalidateItem(a);

			break;
		}
}

INT CBackstageSidebar::GetPreferredWidth()
{
	return m_Width;
}

INT CBackstageSidebar::GetMinHeight()
{
	INT Height = m_Items.m_Items[m_Items.m_ItemCount-1].Rect.bottom;

	if (IsCtrlThemed())
		Height += 2;

	return Height;
}

void CBackstageSidebar::SetSelection(UINT CmdID)
{
	m_SelectedItem = m_HotItem = m_PressedItem = -1;
	m_Hover = FALSE;

	for (UINT a=0; a<m_Items.m_ItemCount; a++)
		if ((m_Items.m_Items[a].Selectable) && (m_Items.m_Items[a].CmdID==CmdID))
		{
			m_SelectedItem = a;
			break;
		}

	Invalidate();
}

INT CBackstageSidebar::ItemAtPosition(CPoint point)
{
	for (UINT a=0; a<m_Items.m_ItemCount; a++)
		if (m_Items.m_Items[a].Selectable)
			if (PtInRect(&m_Items.m_Items[a].Rect, point))
				return a;

	return -1;
}

void CBackstageSidebar::InvalidateItem(INT Index)
{
	if (Index!=-1)
		InvalidateRect(&m_Items.m_Items[Index].Rect);
}

void CBackstageSidebar::PressItem(INT Index)
{
	if (Index!=m_PressedItem)
	{
		InvalidateItem(m_SelectedItem);
		InvalidateItem(m_PressedItem);
		m_PressedItem = Index;
		InvalidateItem(m_PressedItem);
	}
}

void CBackstageSidebar::AdjustLayout()
{
	CRect rect;
	GetClientRect(rect);

	INT y = 0;
	for (UINT a=0; a<m_Items.m_ItemCount; a++)
	{
		m_Items.m_Items[a].Rect.top = y;
		y = m_Items.m_Items[a].Rect.bottom = y+m_Items.m_Items[a].Height;

		m_Items.m_Items[a].Rect.left = 0;
		m_Items.m_Items[a].Rect.right = rect.Width();
	}

	Invalidate();
}

CString CBackstageSidebar::AppendTooltip(UINT /*CmdID*/)
{
	return _T("");
}


BEGIN_MESSAGE_MAP(CBackstageSidebar, CFrontstageWnd)
	ON_WM_NCHITTEST()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_MOUSEHOVER()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_KEYDOWN()
	ON_WM_CONTEXTMENU()
	ON_WM_KILLFOCUS()
END_MESSAGE_MAP()

LRESULT CBackstageSidebar::OnNcHitTest(CPoint point)
{
	LRESULT HitTest = CFrontstageWnd::OnNcHitTest(point);

	if (HitTest==HTCLIENT)
	{
		CRect rectWindow;
		GetWindowRect(rectWindow);

		if (point.y>=rectWindow.top+m_Items.m_Items[m_Items.m_ItemCount-1].Rect.bottom)
			HitTest = HTTRANSPARENT;
	}

	return HitTest;
}

BOOL CBackstageSidebar::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CBackstageSidebar::OnPaint()
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

	// Background
	FillRect(dc, rect, (HBRUSH)GetParent()->SendMessage(WM_CTLCOLORBTN, (WPARAM)dc.m_hDC, (LPARAM)m_hWnd));

	BOOL Themed = IsCtrlThemed();

	const COLORREF colTex = Themed ? 0xDACCC4 : GetSysColor(COLOR_3DFACE);
	const COLORREF colSel = Themed ? 0xFFFFFF : GetSysColor(COLOR_HIGHLIGHTTEXT);
	const COLORREF colCap = Themed ? 0xF0F0F0 : GetSysColor(COLOR_3DFACE);
	const COLORREF colNum = Themed ? 0x998981 : GetSysColor(COLOR_3DSHADOW);

	Graphics g(dc);
	g.SetPixelOffsetMode(PixelOffsetModeHalf);

	// Items
	CRect rectIntersect;

	for (UINT a=0; a<m_Items.m_ItemCount; a++)
		if (IntersectRect(&rectIntersect, &m_Items.m_Items[a].Rect, &pDC.m_ps.rcPaint))
		{
			CRect rectItem(m_Items.m_Items[a].Rect);

			const BOOL Highlight = (m_PressedItem!=-1) ? m_PressedItem==(INT)a : m_SelectedItem==(INT)a;

			// Background
			if (m_Items.m_Items[a].Selectable)
			{
				if (Themed)
				{
					if ((a==0) || m_Items.m_Items[a-1].Selectable)
					{
						LinearGradientBrush brush1(Point(rectItem.left, rectItem.top), Point(rectItem.left, rectItem.top+2), Color(0x20, 0xFF, 0xFF, 0xFF), Color(0x00, 0xFF, 0xFF, 0xFF));
						g.FillRectangle(&brush1, rectItem.left, rectItem.top, rectItem.Width(), 2);
					}
					else
					{
						LinearGradientBrush brush2(Point(rectItem.left, rectItem.top), Point(rectItem.left, rectItem.top+8), Color(0x80, 0x00, 0x00, 0x00), Color(0x00, 0x00, 0x00, 0x00));
						g.FillRectangle(&brush2, rectItem.left, rectItem.top, rectItem.Width(), 8);
					}

					if ((a==m_Items.m_ItemCount-1) || m_Items.m_Items[a+1].Selectable)
					{
						LinearGradientBrush brush2(Point(rectItem.left, rectItem.bottom-3), Point(rectItem.left, rectItem.bottom), Color(0x00, 0x00, 0x00, 0x00), Color(0x60, 0x00, 0x00, 0x00));
						g.FillRectangle(&brush2, rectItem.left, rectItem.bottom-2, rectItem.Width(), 2);
					}
					else
					{
						SolidBrush brush2(Color(0x68, 0x00, 0x00, 0x00));
						g.FillRectangle(&brush2, rectItem.left, rectItem.bottom-1, rectItem.Width(), 1);
					}
				}

				DrawBackstageSelection(dc, g, rectItem, Highlight, Themed);
			}
			else
			{
				if (Themed)
				{
					SolidBrush brush1(Color(0x68, 0x00, 0x00, 0x00));
					g.FillRectangle(&brush1, rectItem.left, rectItem.top, rectItem.Width(), rectItem.Height());

					SolidBrush brush2(Color(0x80, 0x00, 0x00, 0x00));
					g.FillRectangle(&brush2, rectItem.left, rectItem.bottom-1, rectItem.Width(), 1);

					SolidBrush brush3(Color(0x28, 0xFF, 0xFF, 0xFF));
					g.FillRectangle(&brush3, rectItem.left, rectItem.top, rectItem.Width(), 1);
				}
				else
				{
					dc.FillSolidRect(rectItem.left, rectItem.top, rectItem.Width(), 1, 0x000000);
					dc.FillSolidRect(rectItem.left, rectItem.bottom-1, rectItem.Width(), 1, 0x000000);
				}
			}

			rectItem.left += BORDERLEFT;

			// Icon
			if (m_Items.m_Items[a].Selectable)
			{
				rectItem.DeflateRect(BORDER, BORDER);

				if (m_Items.m_Items[a].IconID!=-1)
					p_ButtonIcons->Draw(dc, rectItem.left, rectItem.top+(rectItem.Height()-m_IconSize)/2, m_Items.m_Items[a].IconID, Themed && !Highlight);

				rectItem.left += m_IconSize+BORDER;

				// Count
				if (m_ShowCounts && (m_Items.m_Items[a].Count))
				{
					CRect rectCount(rectItem.right-m_CountWidth+BORDER/2, rectItem.top-2, rectItem.right-BORDER/2-SHADOW/2, rectItem.bottom);

					const COLORREF clr = m_Items.m_Items[a].Color;
					if (clr!=(COLORREF)-1)
					{
						if (Themed)
						{
							g.SetSmoothingMode(SmoothingModeAntiAlias);

							GraphicsPath path;
							CreateRoundRectangle(rectCount, 6, path);

							Matrix m1;
							m1.Translate(3.5f, 3.5f);
							path.Transform(&m1);

							if (!Highlight)
							{
								SolidBrush brushShadow(Color(0x40, 0x00, 0x00, 0x00));
								g.FillPath(&brushShadow, &path);
							}

							Matrix m2;
							m2.Translate(-2.5f, -2.5f);
							path.Transform(&m2);

							SolidBrush brushFill(Color(clr & 0xFF, (clr>>8) & 0xFF, (clr>>16) & 0xFF));
							g.FillPath(&brushFill, &path);

							Pen pen(Color(0xFF, 0xFF, 0xFF), 2.0f);
							g.DrawPath(&pen, &path);

							g.SetSmoothingMode(SmoothingModeNone);

							rectCount.right += 3;
							rectCount.bottom++;
						}
						else
						{
							dc.FillSolidRect(rectCount, clr);
							dc.Draw3dRect(rectCount, 0xFFFFFF, 0xFFFFFF);
						}

						dc.SetTextColor(0xFFFFFF);
					}
					else
					{
						rectCount.top += 5;

						dc.SetTextColor(Highlight ? colSel : colNum);
					}

					CString tmpStr;
					if (m_Items.m_Items[a].Count>=1000000)
					{
						tmpStr.Format(_T("%dm"), m_Items.m_Items[a].Count/1000000);
					}
					else
						if (m_Items.m_Items[a].Count>=1000)
						{
							tmpStr.Format(_T("%dk"), m_Items.m_Items[a].Count/1000);
						}
						else
						{
							tmpStr.Format(_T("%d"), m_Items.m_Items[a].Count);
						}

					CFont* pOldFont = dc.SelectObject(clr!=(COLORREF)-1 ? &LFGetApp()->m_SmallBoldFont : &LFGetApp()->m_SmallFont);
					dc.DrawText(tmpStr, rectCount, DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | (clr!=(COLORREF)-1 ? DT_CENTER : DT_RIGHT));
					dc.SelectObject(pOldFont);
				}

				dc.SetTextColor((m_HotItem==(INT)a) || Highlight ? colSel : colTex);
			}
			else
			{
				rectItem.left += BORDER;
				rectItem.bottom -= 2;

				dc.SetTextColor(colCap);
			}

			// Text
			if (m_Items.m_Items[a].Caption[0])
			{
				CFont* pOldFont = dc.SelectObject(m_Items.m_Items[a].Selectable ? &LFGetApp()->m_LargeFont : &LFGetApp()->m_SmallBoldFont);

				if (!m_Items.m_Items[a].Selectable)
				{
					rectItem.OffsetRect(0, 1);
				}
				else
					if (Themed && !Highlight)
					{
						rectItem.OffsetRect(0, -1);
						COLORREF col = dc.SetTextColor(0x000000);

						dc.DrawText(m_Items.m_Items[a].Caption, rectItem, DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | DT_END_ELLIPSIS);

						rectItem.OffsetRect(0, 1);
						dc.SetTextColor(col);
					}

				dc.DrawText(m_Items.m_Items[a].Caption, rectItem, DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | DT_END_ELLIPSIS);

				dc.SelectObject(pOldFont);
			}
		}

	// Untere Begrenzung
	if (m_Items.m_ItemCount)
	{
		CRect rectItem(m_Items.m_Items[m_Items.m_ItemCount-1].Rect);

		if (Themed)
		{
			LinearGradientBrush brush1(Point(rectItem.left, rectItem.bottom), Point(rectItem.left, rectItem.bottom+2), Color(0x20, 0xFF, 0xFF, 0xFF), Color(0x00, 0xFF, 0xFF, 0xFF));
			g.FillRectangle(&brush1, rectItem.left, rectItem.bottom, rectItem.Width(), 2);
		}
		else
		{
		}
	}

	// Schatten
	if (Themed)
	{
		if (!hShadow)
			hShadow = LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_SIDEBARSHADOW));

		HDC hdcMem = CreateCompatibleDC(dc);
		HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, hShadow);

		BLENDFUNCTION BF = { AC_SRC_OVER, 0, 0xFF, AC_SRC_ALPHA };
		AlphaBlend(dc, rect.right-SHADOW, 0, SHADOW, 5, hdcMem, 0, 0, SHADOW, 5, BF);

		for (INT y=5; y<rect.Height(); y++)
			AlphaBlend(dc, rect.right-SHADOW, y, SHADOW, 1, hdcMem, 0, 4, SHADOW, 1, BF);

		SelectObject(hdcMem, hOldBitmap);
		DeleteDC(hdcMem);
	}

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldBitmap);
}

void CBackstageSidebar::OnSize(UINT nType, INT cx, INT cy)
{
	CFrontstageWnd::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CBackstageSidebar::OnMouseMove(UINT nFlags, CPoint point)
{
	INT Item = ItemAtPosition(point);

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

	if (m_HotItem!=Item)
	{
		InvalidateItem(m_HotItem);
		m_HotItem = Item;
		InvalidateItem(m_HotItem);
	}

	if (nFlags & MK_LBUTTON)
		PressItem(Item);

	if (nFlags & MK_RBUTTON)
		SetFocus();
}

void CBackstageSidebar::OnMouseLeave()
{
	LFGetApp()->HideTooltip();
	InvalidateItem(m_SelectedItem);
	InvalidateItem(m_HotItem);
	InvalidateItem(m_PressedItem);

	m_Hover = FALSE;
	m_HotItem = -1;
	
	if (!m_Keyboard)
		m_PressedItem = -1;
}

void CBackstageSidebar::OnMouseHover(UINT nFlags, CPoint point)
{
	if ((nFlags & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON | MK_XBUTTON1 | MK_XBUTTON2))==0)
	{
		if (m_HotItem!=-1)
			if (!LFGetApp()->IsTooltipVisible())
			{
				const SidebarItem* pSidebarItem = &m_Items.m_Items[m_HotItem];

				CString Hint(pSidebarItem->Hint);
				CString Append = AppendTooltip(pSidebarItem->CmdID);
				if (!Hint.IsEmpty() && !Append.IsEmpty())
					Hint += _T("\n");

				LFGetApp()->ShowTooltip(this, point, m_Items.m_Items[m_HotItem].Caption, Hint+Append, p_TooltipIcons->ExtractIcon(pSidebarItem->IconID));
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

void CBackstageSidebar::OnLButtonDown(UINT /*nFlags*/, CPoint point)
{
	SetFocus();

	PressItem(ItemAtPosition(point));
}

void CBackstageSidebar::OnLButtonUp(UINT /*nFlags*/, CPoint point)
{
	INT Item = ItemAtPosition(point);
	if (Item!=-1)
	{
		PressItem(Item);

		GetOwner()->PostMessage(WM_COMMAND, m_Items.m_Items[Item].CmdID);
	}
}

void CBackstageSidebar::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	INT Index = (m_PressedItem!=-1) ? m_PressedItem : m_SelectedItem;

	switch (nChar)
	{
	case VK_RETURN:
	case VK_EXECUTE:
		if (Index!=-1)
			GetOwner()->PostMessage(WM_COMMAND, m_Items.m_Items[Index].CmdID);

		return;

	case VK_HOME:
		for (INT a=0; a<(INT)m_Items.m_ItemCount; a++)
			if (m_Items.m_Items[a].Selectable)
			{
				PressItem(a);
				m_Keyboard = TRUE;

				break;
			}

		return;

	case VK_END:
		for (INT a=(INT)m_Items.m_ItemCount-1; a>=0; a--)
			if (m_Items.m_Items[a].Selectable)
			{
				PressItem(a);
				m_Keyboard = TRUE;

				break;
			}

		return;

	case VK_UP:
		for (UINT a=0; a<m_Items.m_ItemCount; a++)
		{
			if (--Index<0)
				Index = m_Items.m_ItemCount-1;
			if (m_Items.m_Items[Index].Selectable)
			{
				PressItem(Index);
				m_Keyboard = TRUE;

				break;
			}
		}

		return;

	case VK_DOWN:
		for (UINT a=0; a<m_Items.m_ItemCount; a++)
		{
			if (++Index>=(INT)m_Items.m_ItemCount)
				Index = 0;

			if (m_Items.m_Items[Index].Selectable)
			{
				PressItem(Index);
				m_Keyboard = TRUE;

				break;
			}
		}

		return;
	}

	CFrontstageWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CBackstageSidebar::OnContextMenu(CWnd* /*pWnd*/, CPoint /*pos*/)
{
}

void CBackstageSidebar::OnKillFocus(CWnd* /*pNewWnd*/)
{
	if (m_Keyboard)
	{
		PressItem(-1);
		m_Keyboard = FALSE;
	}
}
