
// CHeaderArea.cpp: Implementierung der Klasse CHeaderArea
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CHeaderArea
//

extern AFX_EXTENSION_MODULE LFCommDlgDLL;

#define BORDERLEFT     16
#define BORDER         10
#define MARGIN         4

CHeaderArea::CHeaderArea()
	: CWnd()
{
	p_App = LFGetApp();
	m_FontHeight = m_RightEdge = 0;
}

BOOL CHeaderArea::Create(CWnd* pParentWnd, UINT nID, BOOL Shadow)
{
	m_Shadow = Shadow;

	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, LoadCursor(NULL, IDC_ARROW));

	const DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE;
	CRect rect;
	rect.SetRectEmpty();
	return CWnd::CreateEx(WS_EX_CONTROLPARENT, className, _T(""), dwStyle, rect, pParentWnd, nID);
}

BOOL CHeaderArea::OnCommand(WPARAM wParam, LPARAM lParam)
{
	HMENU hMenu = (HMENU)GetParent()->SendMessage(WM_GETMENU, wParam);
	if (hMenu)
	{
		CWnd* pWnd = GetDlgItem((INT)wParam);
		if (pWnd)
		{
			CRect rectWindow;
			pWnd->GetWindowRect(&rectWindow);

			TrackPopupMenu(hMenu, TPM_RIGHTALIGN | TPM_RIGHTBUTTON, rectWindow.right, rectWindow.bottom, 0, GetOwner()->GetSafeHwnd(), NULL);
			return TRUE;
		}

		return FALSE;
	}
	else
	{
		return GetOwner()->SendMessage(WM_COMMAND, wParam, lParam)!=0;
	}
}

void CHeaderArea::SetText(CString Caption, CString Hint, BOOL Repaint)
{
	m_Caption = Caption;
	m_Hint = Hint;

	if (Repaint)
	{
		m_BackBufferL = m_BackBufferH = 0;
		Invalidate();
	}
}

UINT CHeaderArea::GetPreferredHeight()
{
	UINT h = 2*BORDER+MARGIN;

	CDC* dc = GetDC();
	CFont* pOldFont = dc->SelectObject(&p_App->m_CaptionFont);
	h += dc->GetTextExtent(_T("Wy")).cy;
	dc->SelectObject(p_App->m_DefaultFont);
	m_FontHeight = dc->GetTextExtent(_T("Wy")).cy;
	h += m_FontHeight;
	dc->SelectObject(pOldFont);
	ReleaseDC(dc);

	return max(h, max(60, (UINT)m_Buttons.GetCount()*(m_FontHeight+8+MARGIN/2)+MARGIN+MARGIN/2));
}

CHeaderButton* CHeaderArea::AddButton(UINT nID)
{
	CString Caption;
	CString Hint;

	if (nID)
	{
		ENSURE(Caption.LoadString(nID));

		INT pos = Caption.Find(L'\n');
		if (pos!=-1)
		{
			Hint = Caption.Left(pos);
			Caption.Delete(0, pos+1);

			if (Hint.GetLength()>40)
			{
				pos = Hint.Find(L' ', Hint.GetLength()/2);
				if (pos!=-1)
					Hint.SetAt(pos, L'\n');
			}
		}
	}

	CHeaderButton* btn = new CHeaderButton();
	btn->Create(Caption, Hint, this, nID);

	m_Buttons.AddTail(btn);

	return btn;
}

void CHeaderArea::AdjustLayout()
{
	SetRedraw(FALSE);

	CRect rect;
	GetClientRect(rect);

	m_RightEdge = rect.right;
	INT Row = max(MARGIN, (rect.Height()-(UINT)m_Buttons.GetCount()*(m_FontHeight+8+MARGIN/2)+MARGIN/2)/2-1);

	for (POSITION p=m_Buttons.GetHeadPosition(); p; )
	{
		CHeaderButton* btn = m_Buttons.GetNext(p);

		CSize sz;
		UINT CaptionWidth;
		btn->GetPreferredSize(sz, CaptionWidth);
		btn->SetWindowPos(NULL, rect.right-sz.cx-BORDERLEFT, Row, sz.cx, sz.cy, SWP_NOZORDER | SWP_NOACTIVATE);

		m_RightEdge = min(m_RightEdge, rect.right-sz.cx-(INT)CaptionWidth-BORDER-BORDERLEFT-MARGIN);

		Row += sz.cy+MARGIN/2;
	}

	m_BackBufferL = m_BackBufferH = 0;

	SetRedraw(TRUE);
	RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN);
}


BEGIN_MESSAGE_MAP(CHeaderArea, CWnd)
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_THEMECHANGED()
	ON_WM_CTLCOLOR()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_SIZE()
	ON_MESSAGE_VOID(WM_ADJUSTLAYOUT, OnAdjustLayout)
END_MESSAGE_MAP()

void CHeaderArea::OnDestroy()
{
	for (POSITION p=m_Buttons.GetHeadPosition(); p; )
	{
		CHeaderButton* btn = m_Buttons.GetNext(p);
		btn->DestroyWindow();
		delete btn;
	}

	CWnd::OnDestroy();

	if (hBackgroundBrush)
		DeleteObject(hBackgroundBrush);
}

BOOL CHeaderArea::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CHeaderArea::OnPaint()
{
	CPaintDC pDC(this);

	CRect rect;
	GetClientRect(rect);

	CDC dc;
	dc.CreateCompatibleDC(&pDC);
	dc.SetBkMode(TRANSPARENT);

	BOOL Themed = IsCtrlThemed();

	CBitmap* pOldBitmap;
	if ((m_BackBufferL!=rect.Width()) || (m_BackBufferH!=rect.Height()))
	{
		m_BackBufferL = rect.Width();
		m_BackBufferH = rect.Height();

		m_BackBuffer.DeleteObject();
		m_BackBuffer.CreateCompatibleBitmap(&pDC, rect.Width(), rect.Height());
		pOldBitmap = dc.SelectObject(&m_BackBuffer);

		Graphics g(dc);

		if (Themed)
		{
			dc.FillSolidRect(rect, 0xFFFFFF);

			CGdiPlusBitmap* pDivider = p_App->GetCachedResourceImage(IDB_DIVUP, _T("PNG"), LFCommDlgDLL.hResource);
			g.DrawImage(pDivider->m_pBitmap, (rect.Width()-(INT)pDivider->m_pBitmap->GetWidth())/2, rect.Height()-(INT)pDivider->m_pBitmap->GetHeight());

			if (m_Shadow)
			{
				SolidBrush brush1(Color(0x18, 0x00, 0x00, 0x00));
				g.FillRectangle(&brush1, 1, 0, rect.Width()-1, 1);

				SolidBrush brush2(Color(0x0C, 0x00, 0x00, 0x00));
				g.FillRectangle(&brush2, 2, 1, rect.Width()-2, 1);
			}
		}
		else
		{
			CRect rectFill(rect);
			rectFill.bottom--;
			dc.FillSolidRect(rectFill, GetSysColor(COLOR_WINDOW));

			rectFill.top = rectFill.bottom;
			rectFill.bottom = rect.bottom;
			dc.FillSolidRect(rectFill, GetSysColor(COLOR_3DFACE));
		}

		dc.SetTextColor(Themed ? 0x404040 : GetSysColor(COLOR_WINDOWTEXT));

		CFont* pOldFont = dc.SelectObject(&p_App->m_CaptionFont);
		CSize sz = dc.GetTextExtent(m_Caption);
		CRect rectText(BORDERLEFT, BORDER, m_RightEdge, BORDER+sz.cy);
		dc.DrawText(m_Caption, rectText, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

		if (Themed)
		{
			dc.SelectObject(&p_App->m_DefaultFont);
		}
		else
		{
			dc.SelectStockObject(DEFAULT_GUI_FONT);
		}

		sz = dc.GetTextExtent(m_Hint);
		rectText.SetRect(BORDERLEFT, rect.bottom-sz.cy-BORDER-1, m_RightEdge, rect.bottom-BORDER-1);
		dc.DrawText(m_Hint, rectText, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

		dc.SetTextColor(Themed ? 0x333333 : GetSysColor(COLOR_WINDOWTEXT));

		for (POSITION p=m_Buttons.GetHeadPosition(); p; )
		{
			CHeaderButton* btn = m_Buttons.GetNext(p);

			CRect rect;
			btn->GetWindowRect(&rect);
			ScreenToClient(&rect);

			CString Caption;
			UINT CaptionWidth;
			btn->GetCaption(Caption, CaptionWidth);

			CRect rectCaption(rect.left-CaptionWidth-MARGIN, rect.top, rect.left, rect.bottom);
			dc.DrawText(Caption, rectCaption, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
		}

		dc.SelectObject(pOldFont);

		if (hBackgroundBrush)
			DeleteObject(hBackgroundBrush);
		hBackgroundBrush = CreatePatternBrush(m_BackBuffer);
	}
	else
	{
		pOldBitmap = dc.SelectObject(&m_BackBuffer);
	}

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldBitmap);
}

LRESULT CHeaderArea::OnThemeChanged()
{
	AdjustLayout();

	return TRUE;
}

HBRUSH CHeaderArea::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	// Call base class version at first, else it will override changes
	HBRUSH hbr = CWnd::OnCtlColor(pDC, pWnd, nCtlColor);

	if ((nCtlColor==CTLCOLOR_BTN) || (nCtlColor==CTLCOLOR_STATIC))
	{
		CRect rc;
		pWnd->GetWindowRect(&rc);
		ScreenToClient(&rc);

		pDC->SetBkMode(TRANSPARENT);
		pDC->SetBrushOrg(-rc.left, -rc.top);

		hbr = hBackgroundBrush;
	}

	return hbr;
}

void CHeaderArea::OnLButtonDown(UINT nFlags, CPoint point)
{
	ClientToScreen(&point);
	GetParent()->ScreenToClient(&point);
	GetParent()->SendMessage(WM_LBUTTONDOWN, (WPARAM)nFlags, MAKELPARAM(point.x, point.y));
}

void CHeaderArea::OnRButtonUp(UINT nFlags, CPoint point)
{
	ClientToScreen(&point);
	GetParent()->ScreenToClient(&point);
	GetParent()->SendMessage(WM_RBUTTONUP, (WPARAM)nFlags, MAKELPARAM(point.x, point.y));
}

void CHeaderArea::OnSize(UINT nType, INT cx, INT cy)
{
	CWnd::OnSize(nType, cx, cy);

	AdjustLayout();
}

void CHeaderArea::OnAdjustLayout()
{
	AdjustLayout();
}
