
// CSidebar.cpp: Implementierung der Klasse CSidebar
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CSidebar
//

#define BACKGROUNDTOP     400
#define BORDER            6
#define SHADOW            10

HBITMAP hShadow = NULL;
HBRUSH hBackgroundTop = NULL;
HBRUSH hBackgroundBottom = NULL;

CSidebar::CSidebar()
	: CWnd()
{
	m_Width = m_NumberWidth = m_IconSize = 0;
	m_SelectedItem = m_HotItem = -1;
	m_Hover = m_Keyboard = m_ShowNumbers = FALSE;
	p_Icons = NULL;
}

BOOL CSidebar::Create(CWnd* pParentWnd, UINT nID, UINT LargeIconsID, UINT SmallIconsID, BOOL ShowNumbers)
{
	// Sidebar with numbers?
	m_ShowNumbers = ShowNumbers;

	// Load icons
	m_LargeIcons.SetImageSize(CSize(32, 32));
	if (LargeIconsID)
		m_LargeIcons.Load(LargeIconsID);

	m_SmallIcons.SetImageSize(CSize(16, 16));
	if (SmallIconsID)
		m_SmallIcons.Load(SmallIconsID);

	LOGFONT lf;
	LFGetApp()->m_DefaultFont.GetLogFont(&lf);

	m_IconSize = abs(lf.lfHeight)>=24 ? 32 : 16;
	p_Icons = (m_IconSize==32) ? &m_LargeIcons : &m_SmallIcons;

	// Create
	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, LFGetApp()->LoadStandardCursor(IDC_ARROW));

	CRect rect;
	rect.SetRectEmpty();
	return CWnd::Create(className, _T(""), WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_TABSTOP, rect, pParentWnd, nID);
}

BOOL CSidebar::PreTranslateMessage(MSG* pMsg)
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

	return CWnd::PreTranslateMessage(pMsg);
}

void CSidebar::AddItem(BOOL Selectable, UINT CmdID, INT IconID, WCHAR* Caption, WCHAR* Hint, BOOL NumberInRed)
{
	// Hinzufügen
	SidebarItem i;
	ZeroMemory(&i, sizeof(i));
	i.Selectable = Selectable;
	i.CmdID = CmdID;
	i.IconID = IconID;

	if (Caption)
		if (Selectable)
		{
			wcscpy_s(i.Caption, 256, Caption);
			i.NumberInRed = NumberInRed;
		}
		else
		{
			for (WCHAR* pDst=i.Caption; *Caption; )
				*(pDst++) = (WCHAR)towupper(*(Caption++));
		}

		if (Hint)
		wcscpy_s(i.Hint, 256, Hint);

	// Metrik
	CSize Size;

	if (i.Caption[0]==L'\0')
	{
		Size.cy = BORDER+2;
	}
	else
	{
		CDC* pDC = GetDC();
		CFont* pOldFont = pDC->SelectObject(Selectable ? &LFGetApp()->m_LargeFont : &afxGlobalData.fontBold);
		Size = pDC->GetTextExtent(i.Caption);
		pDC->SelectObject(pOldFont);
		ReleaseDC(pDC);

		if (Selectable)
		{
			Size.cx += m_IconSize+3*BORDER+m_NumberWidth+SHADOW/2;
			Size.cy = max(m_IconSize, Size.cy) + 2*BORDER;
		}
		else
		{
			Size.cx += 2*BORDER+SHADOW/2;
			Size.cy += BORDER+1;
		}

	}

	m_Width = max(m_Width, Size.cx);
	i.Height = Size.cy;

	m_Items.AddItem(i);
}

void CSidebar::AddCommand(UINT CmdID, INT IconID, WCHAR* Caption, WCHAR* Hint, BOOL NumberInRed)
{
	AddItem(TRUE, CmdID, IconID, Caption, Hint, NumberInRed);
}

void CSidebar::AddCaption(WCHAR* Caption)
{
	AddItem(FALSE, 0, -1, Caption, L"", FALSE);
}

void CSidebar::AddCaption(UINT ResID)
{
	CString tmpStr((LPCSTR)ResID);

	AddCaption(tmpStr.GetBuffer());
}

void CSidebar::ResetNumbers()
{
	for (UINT a=0; a<m_Items.m_ItemCount; a++)
		m_Items.m_Items[a].Number = 0;

	Invalidate();
}

void CSidebar::SetNumber(UINT CmdID, UINT Number)
{
	for (UINT a=0; a<m_Items.m_ItemCount; a++)
		if (m_Items.m_Items[a].CmdID==CmdID)
		{
			m_Items.m_Items[a].Number = Number;
			InvalidateItem(a);
			break;
		}
}

INT CSidebar::GetPreferredWidth()
{
	return m_Width;
}

void CSidebar::SetSelection(UINT CmdID)
{
	m_SelectedItem = m_HotItem = -1;
	m_HotItem = 0;
	m_Hover = FALSE;

	for (UINT a=0; a<m_Items.m_ItemCount; a++)
		if ((m_Items.m_Items[a].Selectable) && (m_Items.m_Items[a].CmdID==CmdID))
		{
			m_SelectedItem = a;
			break;
		}

	Invalidate();
}

INT CSidebar::ItemAtPosition(CPoint point)
{
	for (UINT a=0; a<m_Items.m_ItemCount; a++)
		if (m_Items.m_Items[a].Selectable)
			if (PtInRect(&m_Items.m_Items[a].Rect, point))
				return a;

	return -1;
}

void CSidebar::InvalidateItem(INT Index)
{
	if (Index!=-1)
		InvalidateRect(&m_Items.m_Items[Index].Rect);
}

void CSidebar::SelectItem(INT Index)
{
	if (Index!=m_SelectedItem)
	{
		InvalidateItem(m_SelectedItem);
		InvalidateItem(Index);

		m_SelectedItem = Index;
	}
}

void CSidebar::AdjustLayout()
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

CString CSidebar::AppendTooltip(UINT /*CmdID*/)
{
	return _T("");
}

void CSidebar::PrepareBitmaps()
{
	if (!hShadow)
		hShadow = LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_SIDEBARSHADOW));

	if (!hBackgroundTop)
	{
		CGdiPlusBitmap* pTexture = LFGetApp()->GetCachedResourceImage(IDB_SIDEBARBACKGROUND, _T("PNG"));

		CDC* pDC = GetDC();

		CDC dc;
		dc.CreateCompatibleDC(pDC);

		CBitmap MemBitmap;
		MemBitmap.CreateCompatibleBitmap(pDC, pTexture->m_pBitmap->GetWidth(), BACKGROUNDTOP);
		CBitmap* pOldBitmap = dc.SelectObject(&MemBitmap);

		Graphics g(dc);

		TextureBrush brush1(pTexture->m_pBitmap);
		g.FillRectangle(&brush1, 0, 0, pTexture->m_pBitmap->GetWidth(), BACKGROUNDTOP);

		LinearGradientBrush brush2(Point(0, 0), Point(0, BACKGROUNDTOP), Color(0x30, 0x00, 0x00, 0x00), Color(0x60, 0x00, 0x00, 0x00));
		g.FillRectangle(&brush2, 0, 0, pTexture->m_pBitmap->GetWidth(), BACKGROUNDTOP);

		dc.SelectObject(pOldBitmap);
		ReleaseDC(pDC);

		hBackgroundTop = CreatePatternBrush(MemBitmap);
	}

	if (!hBackgroundBottom)
	{
		CGdiPlusBitmap* pTexture = LFGetApp()->GetCachedResourceImage(IDB_SIDEBARBACKGROUND, _T("PNG"));

		CDC* pDC = GetDC();

		CDC dc;
		dc.CreateCompatibleDC(pDC);

		CBitmap MemBitmap;
		MemBitmap.CreateCompatibleBitmap(pDC, pTexture->m_pBitmap->GetWidth(), pTexture->m_pBitmap->GetHeight());
		CBitmap* pOldBitmap = dc.SelectObject(&MemBitmap);

		Graphics g(dc);

		TextureBrush brush1(pTexture->m_pBitmap);
		g.FillRectangle(&brush1, 0, 0, pTexture->m_pBitmap->GetWidth(), pTexture->m_pBitmap->GetHeight());

		SolidBrush brush2(Color(0x60, 0x00, 0x00, 0x00));
		g.FillRectangle(&brush2, 0, 0, pTexture->m_pBitmap->GetWidth(), pTexture->m_pBitmap->GetHeight());

		dc.SelectObject(pOldBitmap);
		ReleaseDC(pDC);

		hBackgroundBottom = CreatePatternBrush(MemBitmap);
	}
}


BEGIN_MESSAGE_MAP(CSidebar, CWnd)
	ON_WM_CREATE()
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
END_MESSAGE_MAP()

INT CSidebar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	// Metrik
	if (m_ShowNumbers)
	{
		CDC* pDC = GetDC();
		CFont* pOldFont = pDC->SelectObject(&afxGlobalData.fontBold);
		m_NumberWidth = pDC->GetTextExtent(_T("888W")).cx+2*BORDER+SHADOW/2;
		pDC->SelectObject(pOldFont);
		ReleaseDC(pDC);
	}

	// Texturen
	if (IsCtrlThemed())
		PrepareBitmaps();

	return 0;
}

BOOL CSidebar::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CSidebar::OnPaint()
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

	// Background
	BOOL Themed = IsCtrlThemed();

	if (Themed)
	{
		PrepareBitmaps();

		FillRect(dc, CRect(rect.left, rect.top, rect.right, BACKGROUNDTOP), hBackgroundTop);

		if (rect.Height()>BACKGROUNDTOP)
			FillRect(dc, CRect(rect.left, BACKGROUNDTOP, rect.right, rect.bottom), hBackgroundBottom);
	}
	else
	{
		FillRect(dc, rect, (HBRUSH)GetStockObject(DKGRAY_BRUSH));
	}

	const COLORREF colTex = Themed ? 0xDACCC4 : GetSysColor(COLOR_3DFACE);
	const COLORREF colSel = Themed ? 0xFFFFFF : GetSysColor(COLOR_HIGHLIGHTTEXT);
	const COLORREF colCap = Themed ? 0xF0F0F0 : GetSysColor(COLOR_3DFACE);
	const COLORREF colNum = Themed ? 0x998981 : GetSysColor(COLOR_3DSHADOW);

	Graphics g(dc);
	g.SetPixelOffsetMode(PixelOffsetModeHalf);

	// Items
	CRect rectIntersect;

	for (UINT a=0; a<m_Items.m_ItemCount; a++)
		if (IntersectRect(&rectIntersect, &m_Items.m_Items[a].Rect, rectUpdate))
		{
			CRect rectItem(m_Items.m_Items[a].Rect);

			// Background
			if (Themed)
			{
				if (m_Items.m_Items[a].Selectable)
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

					if (m_SelectedItem==(INT)a)
					{
						LinearGradientBrush brush3(Point(rectItem.left, rectItem.top), Point(rectItem.left, rectItem.bottom-1), Color(0xC0, 0x20, 0xA0, 0xFF), Color(0xC0, 0x10, 0x78, 0xFF));
						g.FillRectangle(&brush3, rectItem.left, rectItem.top, rectItem.Width(), rectItem.Height()-1);

						SolidBrush brush4(Color(0x40, 0xFF, 0xFF, 0xFF));
						g.FillRectangle(&brush4, rectItem.left, rectItem.top, rectItem.Width(), 1);
					}
				}
				else
				{
					SolidBrush brush1(Color(0x68, 0x00, 0x00, 0x00));
					g.FillRectangle(&brush1, rectItem.left, rectItem.top, rectItem.Width(), rectItem.Height());

					SolidBrush brush2(Color(0x80, 0x00, 0x00, 0x00));
					g.FillRectangle(&brush2, rectItem.left, rectItem.bottom-1, rectItem.Width(), 1);

					SolidBrush brush3(Color(0x28, 0xFF, 0xFF, 0xFF));
					g.FillRectangle(&brush3, rectItem.left, rectItem.top, rectItem.Width(), 1);
				}
			}
			else
			{
				if (!m_Items.m_Items[a].Selectable)
				{
					dc.FillSolidRect(rectItem.left, rectItem.top, rectItem.Width(), 1, GetSysColor(COLOR_WINDOWTEXT));
					dc.FillSolidRect(rectItem.left, rectItem.bottom-1, rectItem.Width(), 1, GetSysColor(COLOR_WINDOWTEXT));
				}
				else
					if (m_SelectedItem==(INT)a)
					{
						dc.FillSolidRect(rectItem, GetSysColor(COLOR_HIGHLIGHT));
					}
			}

			// Icon
			if (m_Items.m_Items[a].Selectable)
			{
				rectItem.DeflateRect(BORDER, BORDER);

				if (m_Items.m_Items[a].IconID!=-1)
				{
					CAfxDrawState ds;
					p_Icons->PrepareDrawImage(ds);
					p_Icons->Draw(&dc, rectItem.left, rectItem.top+(rectItem.Height()-m_IconSize)/2, m_Items.m_Items[a].IconID);
					p_Icons->EndDrawImage(ds);
				}

				rectItem.left += m_IconSize+BORDER;

				// Number
				if (m_ShowNumbers && (m_Items.m_Items[a].Number))
				{
					CRect rectNumber(rectItem.right-m_NumberWidth+BORDER/2, rectItem.top-2, rectItem.right-BORDER/2-SHADOW/2, rectItem.bottom);

					if (m_Items.m_Items[a].NumberInRed)
					{
						if (Themed)
						{
							g.SetSmoothingMode(SmoothingModeAntiAlias);

							GraphicsPath path;
							CreateRoundRectangle(rectNumber, 6, path);

							Matrix m1;
							m1.Translate(3.5f, 3.5f);
							path.Transform(&m1);

							if (m_SelectedItem!=(INT)a)
							{
								SolidBrush brushShadow(Color(0x40, 0x00, 0x00, 0x00));
								g.FillPath(&brushShadow, &path);
							}

							Matrix m2;
							m2.Translate(-2.5f, -2.5f);
							path.Transform(&m2);

							SolidBrush brushRed(Color(0xFF, 0, 0));
							g.FillPath(&brushRed, &path);

							Pen pen(Color(0xFF, 0xFF, 0xFF), 2.0f);
							g.DrawPath(&pen, &path);

							g.SetSmoothingMode(SmoothingModeNone);

							rectNumber.right += 3;
							rectNumber.bottom++;
						}
						else
						{
							dc.FillSolidRect(rectNumber, 0x0000FF);
							dc.Draw3dRect(rectNumber, 0xFFFFFF, 0xFFFFFF);
						}

						dc.SetTextColor(0xFFFFFF);
					}
					else
					{
						rectNumber.top += 5;

						dc.SetTextColor((m_SelectedItem==(INT)a) ? colSel : colNum);
					}

					CString tmpStr;
					if (m_Items.m_Items[a].Number>=1000000)
					{
						tmpStr.Format(_T("%dm"), m_Items.m_Items[a].Number/1000000);
					}
					else
						if (m_Items.m_Items[a].Number>=1000)
						{
							tmpStr.Format(_T("%dk"), m_Items.m_Items[a].Number/1000);
						}
						else
						{
							tmpStr.Format(_T("%d"), m_Items.m_Items[a].Number);
						}

					CFont* pOldFont = dc.SelectObject(m_Items.m_Items[a].NumberInRed ? &afxGlobalData.fontBold : &LFGetApp()->m_SmallFont);
					dc.DrawText(tmpStr, rectNumber, DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | (m_Items.m_Items[a].NumberInRed ? DT_CENTER : DT_RIGHT));
					dc.SelectObject(pOldFont);
				}

				dc.SetTextColor((m_HotItem==(INT)a) || (m_SelectedItem==(INT)a) ? colSel : colTex);
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
				CFont* pOldFont = dc.SelectObject(m_Items.m_Items[a].Selectable ? &LFGetApp()->m_LargeFont : &afxGlobalData.fontBold);

				if (!m_Items.m_Items[a].Selectable)
				{
					rectItem.OffsetRect(0, 1);
				}
				else
					if (Themed && (m_SelectedItem!=(INT)a))
					{
						rectItem.OffsetRect(1, 1);
						COLORREF col = dc.SetTextColor(0x000000);
						dc.DrawText(m_Items.m_Items[a].Caption, rectItem, DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | DT_END_ELLIPSIS);
						dc.SetTextColor(col);
						rectItem.OffsetRect(-1, -1);
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

void CSidebar::OnSize(UINT nType, INT cx, INT cy)
{
	CWnd::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CSidebar::OnMouseMove(UINT nFlags, CPoint point)
{
	INT Item = ItemAtPosition(point);

	if (!m_Hover)
	{
		m_Hover = TRUE;

		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_LEAVE | TME_HOVER;
		tme.dwHoverTime = LFHOVERTIME;
		tme.hwndTrack = m_hWnd;
		TrackMouseEvent(&tme);
	}
	else
		if ((LFGetApp()->IsTooltipVisible()) && (Item!=m_HotItem))
			LFGetApp()->HideTooltip();

	if (m_HotItem!=Item)
	{
		InvalidateItem(m_SelectedItem);
		InvalidateItem(m_HotItem);
		m_HotItem = Item;
		InvalidateItem(m_HotItem);
	}

	if ((nFlags & MK_LBUTTON) && (Item!=-1))
		SelectItem(Item);

	if (nFlags & MK_RBUTTON)
		SetFocus();
}

void CSidebar::OnMouseLeave()
{
	LFGetApp()->HideTooltip();
	InvalidateItem(m_SelectedItem);
	InvalidateItem(m_HotItem);

	m_Hover = FALSE;
	m_HotItem = -1;
}

void CSidebar::OnMouseHover(UINT nFlags, CPoint point)
{
	if ((nFlags & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON | MK_XBUTTON1 | MK_XBUTTON2))==0)
	{
		if (m_HotItem!=-1)
			if (!LFGetApp()->IsTooltipVisible())
			{
				INT Index = m_Items.m_Items[m_HotItem].IconID;
				HICON hIcon = (Index!=-1) ? m_LargeIcons.ExtractIcon(Index) : NULL;

				CString Hint = m_Items.m_Items[m_HotItem].Hint;
				CString Append = AppendTooltip(m_Items.m_Items[m_HotItem].CmdID);
				if (!Hint.IsEmpty() && !Append.IsEmpty())
					Hint += _T("\n");

				LFGetApp()->ShowTooltip(this, point, m_Items.m_Items[m_HotItem].Caption, Hint+Append, hIcon);
			}
	}
	else
	{
		LFGetApp()->HideTooltip();
	}

	TRACKMOUSEEVENT tme;
	tme.cbSize = sizeof(TRACKMOUSEEVENT);
	tme.dwFlags = TME_LEAVE | TME_HOVER;
	tme.dwHoverTime = LFHOVERTIME;
	tme.hwndTrack = m_hWnd;
	TrackMouseEvent(&tme);
}

void CSidebar::OnLButtonDown(UINT /*nFlags*/, CPoint point)
{
	SetFocus();

	INT Item = ItemAtPosition(point);
	if (Item!=-1)
		SelectItem(Item);
}

void CSidebar::OnLButtonUp(UINT /*nFlags*/, CPoint point)
{
	INT Item = ItemAtPosition(point);
	if (Item!=-1)
	{
		SelectItem(Item);

		GetOwner()->PostMessage(WM_COMMAND, m_Items.m_Items[Item].CmdID);
	}
}

void CSidebar::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	INT Index = m_SelectedItem;

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
				SelectItem(a);
				m_Keyboard = TRUE;
				break;
			}

		return;

	case VK_END:
		for (INT a=(INT)m_Items.m_ItemCount-1; a>=0; a--)
			if (m_Items.m_Items[a].Selectable)
			{
				SelectItem(a);
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
				SelectItem(Index);
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
				SelectItem(Index);
				m_Keyboard = TRUE;
				break;
			}
		}

		return;
	}

	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CSidebar::OnContextMenu(CWnd* /*pWnd*/, CPoint /*pos*/)
{
}
