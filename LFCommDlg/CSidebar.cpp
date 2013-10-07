
// CSidebar.cpp: Implementierung der Klasse CSidebar
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CSidebar
//

#define BORDER     6
#define SHADOW     10

extern AFX_EXTENSION_MODULE LFCommDlgDLL;

CSidebar::CSidebar()
	: CWnd()
{
	p_App = LFGetApp();

	m_Width = m_NumberWidth = 0;
	m_SelectedItem = m_HotItem = -1;
	m_Hover = m_Keyboard = m_ShowNumbers = FALSE;
	hShadow = NULL;
}

BOOL CSidebar::Create(CWnd* pParentWnd, UINT nID, UINT LargeIconsID, UINT SmallIconsID, BOOL ShowNumbers)
{
	// Sidebar with numbers?
	m_ShowNumbers = ShowNumbers;
	if (ShowNumbers)
	{
		CDC* dc = GetDC();
		CFont* pOldFont = dc->SelectObject(&afxGlobalData.fontBold);
		m_NumberWidth = dc->GetTextExtent(_T("888W")).cx+2*BORDER+SHADOW/2;
		dc->SelectObject(pOldFont);
		ReleaseDC(dc);
	}

	// Load icons
	m_LargeIcons.SetImageSize(CSize(32, 32));
	if (LargeIconsID)
		m_LargeIcons.Load(LargeIconsID);

	m_SmallIcons.SetImageSize(CSize(16, 16));
	if (SmallIconsID)
		m_SmallIcons.Load(SmallIconsID);

	// Create
	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, LoadCursor(NULL, IDC_ARROW));

	const DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_TABSTOP;
	CRect rect;
	rect.SetRectEmpty();
	return CWnd::Create(className, _T(""), dwStyle, rect, pParentWnd, nID);
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
		m_TooltipCtrl.Deactivate();
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
			for (WCHAR* pDst=i.Caption; *Caption;)
				*(pDst++) = (WCHAR)toupper(*(Caption++));
		}
	if (Hint)
		wcscpy_s(i.Hint, 256, Hint);

	// Maße
	CSize sz;

	if (i.Caption[0]==L'\0')
	{
		sz.cy = BORDER;
	}
	else
	{
		CDC* dc = GetDC();
		CFont* pOldFont = dc->SelectObject(Selectable ? &p_App->m_LargeFont : &afxGlobalData.fontBold);
		sz = dc->GetTextExtent(i.Caption);
		dc->SelectObject(pOldFont);
		ReleaseDC(dc);

		if (Selectable)
		{
			sz.cx += 16+3*BORDER+m_NumberWidth+SHADOW/2;
			sz.cy = max(16, sz.cy) + 2*BORDER;
		}
		else
		{
			sz.cx += 2*BORDER+SHADOW/2;
			sz.cy += BORDER;
		}

	}

	m_Width = max(m_Width, sz.cx);
	i.Height = sz.cy;

	m_Items.AddItem(i);
}

__forceinline void CSidebar::AddCommand(UINT CmdID, INT IconID, WCHAR* Caption, WCHAR* Hint, BOOL NumberInRed)
{
	AddItem(TRUE, CmdID, IconID, Caption, Hint, NumberInRed);
}

__forceinline void CSidebar::AddCaption(WCHAR* Caption)
{
	AddItem(FALSE, 0, -1, Caption, L"", FALSE);
}

void CSidebar::AddCaption(UINT ResID)
{
	CString tmpStr;
	ENSURE(tmpStr.LoadString(ResID));

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

void CSidebar::Reset(UINT CmdID)
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

void CSidebar::InvalidateItem(INT idx)
{
	if (idx!=-1)
		InvalidateRect(&m_Items.m_Items[idx].Rect);
}

void CSidebar::SelectItem(INT idx)
{
	if (idx!=m_SelectedItem)
	{
		InvalidateItem(m_SelectedItem);
		InvalidateItem(idx);

		m_SelectedItem = idx;
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


BEGIN_MESSAGE_MAP(CSidebar, CWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
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

	hShadow = LoadBitmap(LFCommDlgDLL.hResource, MAKEINTRESOURCE(IDB_SIDEBARSHADOW));

	m_TooltipCtrl.Create(this);

	return 0;
}

void CSidebar::OnDestroy()
{
	DeleteObject(hShadow);

	CWnd::OnDestroy();
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

	CBitmap buffer;
	buffer.CreateCompatibleBitmap(&pDC, rect.Width(), rect.Height());
	CBitmap* pOldBitmap = dc.SelectObject(&buffer);

	// Background
	BOOL Themed = IsCtrlThemed();

	const COLORREF colBg = Themed ? 0x4A3932 : GetSysColor(COLOR_3DDKSHADOW);
	const COLORREF colLi = Themed ? 0x55453C : GetSysColor(COLOR_3DSHADOW);
	const COLORREF colDa = Themed ? 0x382B25 : 0x000000;
	const COLORREF colSe = Themed ? 0x3F2F29 : 0x000000;
	const COLORREF colTx = Themed ? 0xDACCC4 : GetSysColor(COLOR_3DFACE);
	const COLORREF colCp = Themed ? 0x998981 : GetSysColor(COLOR_3DFACE);

	Graphics g(dc);
	g.SetSmoothingMode(SmoothingModeAntiAlias);

	dc.FillSolidRect(rect, colBg);

	// Items
	CRect rectIntersect;

	for (UINT a=0; a<m_Items.m_ItemCount; a++)
		if (IntersectRect(&rectIntersect, &m_Items.m_Items[a].Rect, rectUpdate))
		{
			CRect rectItem(m_Items.m_Items[a].Rect);

			// Background
			if (m_SelectedItem!=(INT)a)
			{
				if (m_Items.m_Items[a].Selectable)
				{
					dc.FillSolidRect(rectItem.left, rectItem.top, rectItem.Width(), 1, colLi);
				}
				else
					if (Themed)
					{
						LinearGradientBrush brush(Point(rectItem.left, rectItem.top), Point(rectItem.left, rectItem.bottom-1), Color(0x43, 0x4A, 0x5E), Color(0x39, 0x40, 0x52));
						g.FillRectangle(&brush, rectItem.left, rectItem.top, rectItem.Width(), rectItem.Height()-1);
					}

				dc.FillSolidRect(rectItem.left, rectItem.bottom-1, rectItem.Width(), 1, colDa);
			}
			else
			{
				dc.FillSolidRect(rectItem.left, rectItem.top, rectItem.Width(), rectItem.Height(), colSe);
			}

			// Icon
			if (m_Items.m_Items[a].Selectable)
			{
				rectItem.DeflateRect(BORDER, BORDER);

				if (m_Items.m_Items[a].IconID!=-1)
				{
					CAfxDrawState ds;
					m_SmallIcons.PrepareDrawImage(ds);
					m_SmallIcons.Draw(&dc, rectItem.left, rectItem.top+(rectItem.Height()-16)/2, m_Items.m_Items[a].IconID);
					m_SmallIcons.EndDrawImage(ds);
				}

				rectItem.left += 16+BORDER;

				// Number
				if (m_ShowNumbers && (m_Items.m_Items[a].Number))
				{
					CRect rectNumber(rectItem.right-m_NumberWidth+BORDER/2, rectItem.top-2, rectItem.right-BORDER/2-SHADOW/2, rectItem.bottom);

					if (m_Items.m_Items[a].NumberInRed)
					{
						if (Themed)
						{
							GraphicsPath path;
							CreateRoundRectangle(rectNumber, 6, path);

							SolidBrush brush(Color(0xFF, 0, 0));
							g.FillPath(&brush, &path);

							Pen pen(Color(0xFF, 0xFF, 0xFF), 1.6f);
							g.DrawPath(&pen, &path);

							rectNumber.right += 2;
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
						dc.SetTextColor(colCp);

						rectNumber.top += 6;
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

					CFont* pOldFont = dc.SelectObject(m_Items.m_Items[a].NumberInRed ? &afxGlobalData.fontBold : &p_App->m_SmallFont);
					dc.DrawText(tmpStr, rectNumber, DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | (m_Items.m_Items[a].NumberInRed ? DT_CENTER : DT_RIGHT));
					dc.SelectObject(pOldFont);
				}

				dc.SetTextColor((m_HotItem==(INT)a) || ((m_SelectedItem==(INT)a) && (m_HotItem==-1)) ? 0xFFFFFF : colTx);
			}
			else
			{
				rectItem.left += BORDER;
				rectItem.bottom -= 2;
				dc.SetTextColor(colCp);
			}

			// Text
			if (m_Items.m_Items[a].Caption[0])
			{
				CFont* pOldFont = dc.SelectObject(m_Items.m_Items[a].Selectable ? &p_App->m_LargeFont : &afxGlobalData.fontBold);

				if (Themed)
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

	if (m_Items.m_ItemCount)
		dc.FillSolidRect(rect.left, m_Items.m_Items[m_Items.m_ItemCount-1].Rect.bottom, rect.Width(), 1, colLi);

	if (Themed)
	{
		HDC hdcMem = CreateCompatibleDC(dc);
		HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hShadow);

		BLENDFUNCTION BF = { AC_SRC_OVER, 0, 0xFF, AC_SRC_ALPHA };
		AlphaBlend(dc, rect.right-SHADOW, 0, SHADOW, 5, hdcMem, 0, 0, SHADOW, 5, BF);

		for (INT y=5; y<rect.Height(); y++)
			AlphaBlend(dc, rect.right-SHADOW, y, SHADOW, 1, hdcMem, 0, 4, SHADOW, 1, BF);

		SelectObject(hdcMem, hbmOld);
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
		if ((m_TooltipCtrl.IsWindowVisible()) && (Item!=m_HotItem))
			m_TooltipCtrl.Deactivate();

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
	m_TooltipCtrl.Deactivate();
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
			if (!m_TooltipCtrl.IsWindowVisible())
			{
				INT idx = m_Items.m_Items[m_HotItem].IconID;
				HICON hIcon = (idx!=-1) ? m_LargeIcons.ExtractIcon(idx) : NULL;

				ClientToScreen(&point);
				m_TooltipCtrl.Track(point, hIcon, hIcon ? CSize(32, 32) : CSize(0, 0), m_Items.m_Items[m_HotItem].Caption, m_Items.m_Items[m_HotItem].Hint+AppendTooltip(m_Items.m_Items[m_HotItem].CmdID));
			}
	}
	else
	{
		m_TooltipCtrl.Deactivate();
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
	INT idx = m_SelectedItem;

	switch (nChar)
	{
	case VK_RETURN:
	case VK_EXECUTE:
		if (idx!=-1)
			GetOwner()->PostMessage(WM_COMMAND, m_Items.m_Items[idx].CmdID);
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
			if (--idx<0)
				idx = m_Items.m_ItemCount-1;
			if (m_Items.m_Items[idx].Selectable)
			{
				SelectItem(idx);
				m_Keyboard = TRUE;
				break;
			}
		}
		return;
	case VK_DOWN:
		for (UINT a=0; a<m_Items.m_ItemCount; a++)
		{
			if (++idx>=(INT)m_Items.m_ItemCount)
				idx = 0;
			if (m_Items.m_Items[idx].Selectable)
			{
				SelectItem(idx);
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
