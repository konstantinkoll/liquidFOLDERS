
// CExplorerHeader.cpp: Implementierung der Klasse CExplorerHeader
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CExplorerHeader
//

extern AFX_EXTENSION_MODULE LFCommDlgDLL;

#define BORDERLEFT      16
#define BORDER          10
#define MARGIN          4

CExplorerHeader::CExplorerHeader()
	: CWnd()
{
	m_CaptionCol = 0x993300;
	m_HintCol = 0x79675A;
	hBackgroundBrush = NULL;
	m_GradientLine = TRUE;
}

BOOL CExplorerHeader::Create(CWnd* pParentWnd, UINT nID)
{
	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, LoadCursor(NULL, IDC_ARROW));

	const DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE;
	CRect rect;
	rect.SetRectEmpty();
	return CWnd::CreateEx(WS_EX_CONTROLPARENT, className, _T(""), dwStyle, rect, pParentWnd, nID);
}

BOOL CExplorerHeader::OnCommand(WPARAM wParam, LPARAM /*lParam*/)
{
	HMENU hMenu = (HMENU)GetOwner()->SendMessage(WM_GETMENU, wParam);
	if (hMenu)
	{
		CWnd* pWnd = GetDlgItem(wParam);
		if (pWnd)
		{
			CRect rectWindow;
			pWnd->GetWindowRect(&rectWindow);

			TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, rectWindow.left, rectWindow.bottom, 0, GetOwner()->GetSafeHwnd(), NULL);
			return TRUE;
		}
	}

	return FALSE;
}

void CExplorerHeader::SetText(CString Caption, CString Hint, BOOL Repaint)
{
	m_Caption = Caption;
	m_Hint = Hint;

	if (Repaint)
		Invalidate();
}

void CExplorerHeader::SetColors(COLORREF CaptionCol, COLORREF HintCol, BOOL Repaint)
{
	if (CaptionCol!=(COLORREF)-1)
		m_CaptionCol = CaptionCol;
	if (HintCol!=(COLORREF)-1)
		m_HintCol = HintCol;

	if (Repaint)
		Invalidate();
}

void CExplorerHeader::SetLineStyle(BOOL GradientLine, BOOL Repaint)
{
	m_GradientLine = GradientLine;

	if (Repaint)
		Invalidate();
}

UINT CExplorerHeader::GetPreferredHeight()
{
	LOGFONT lf;
	UINT h = 3*BORDER;

	((LFApplication*)AfxGetApp())->m_CaptionFont.GetLogFont(&lf);
	h += abs(lf.lfHeight);

	((LFApplication*)AfxGetApp())->m_DefaultFont.GetLogFont(&lf);
	h += abs(lf.lfHeight);

	return max(h, max(60, m_Buttons.GetCount()*((UINT)abs(lf.lfHeight)+8+MARGIN)+MARGIN+1));
}

CHeaderButton* CExplorerHeader::AddButton(UINT nID)
{
	CString Caption;
	CString Hint;
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

	CHeaderButton* btn = new CHeaderButton();
	btn->Create(Caption, Hint, this, nID);

	m_Buttons.AddTail(btn);

	return btn;
}

void CExplorerHeader::AdjustLayout()
{
	SetRedraw(FALSE);

	CRect rect;
	GetClientRect(rect);

	INT Row = MARGIN;

	for (POSITION p=m_Buttons.GetHeadPosition(); p; )
	{
		CHeaderButton* btn = m_Buttons.GetNext(p);

		CSize sz;
		btn->GetPreferredSize(sz);
		btn->SetWindowPos(NULL, rect.right-sz.cx-BORDER, Row, sz.cx, sz.cy, SWP_NOZORDER | SWP_NOACTIVATE);

		Row += sz.cy+MARGIN;
	}

	SetRedraw(TRUE);
	RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN);
}


BEGIN_MESSAGE_MAP(CExplorerHeader, CWnd)
	ON_WM_CREATE()
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

INT CExplorerHeader::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	m_Background.LoadBitmap(IDB_EXPLORERGRADIENT);
	hBackgroundBrush = CreatePatternBrush(m_Background);

	return 0;
}

void CExplorerHeader::OnDestroy()
{
	CWnd::OnDestroy();

	for (POSITION p=m_Buttons.GetHeadPosition(); p; )
	{
		CHeaderButton* btn = m_Buttons.GetNext(p);
		btn->DestroyWindow();
		delete btn;
	}

	if (hBackgroundBrush)
		DeleteObject(hBackgroundBrush);
}

BOOL CExplorerHeader::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CExplorerHeader::OnPaint()
{
	CPaintDC pDC(this);

	CRect rect;
	GetClientRect(rect);

	CDC dc;
	dc.CreateCompatibleDC(&pDC);
	dc.SetBkMode(TRANSPARENT);

	CBitmap buffer;
	buffer.CreateCompatibleBitmap(&pDC, rect.Width(), rect.Height());
	CBitmap* pOldBitmap = dc.SelectObject(&buffer);

	BOOL Themed = IsCtrlThemed();
	if (Themed)
	{
		CRect rectBackground(rect);
		if (rectBackground.bottom>60)
		{
			rectBackground.bottom = 60;
			dc.FillSolidRect(0, 60, rect.Width(), rect.Height()-60, 0xFFFFFF);
		}
		FillRect(dc, rectBackground, hBackgroundBrush);

		#define LineCol 0xF5E5D6
		if (m_GradientLine)
		{
			if (rect.Width()>4*BORDERLEFT)
			{
				Graphics g(dc);

				Color c1;
				c1.SetFromCOLORREF(0xFFFFFF);
				Color c2;
				c2.SetFromCOLORREF(LineCol);

				LinearGradientBrush brush1(Point(0, 0), Point(BORDERLEFT*2, 0), c1, c2);
				g.FillRectangle(&brush1, 0, rect.bottom-1, BORDERLEFT*2, 1);

				LinearGradientBrush brush2(Point(rect.right, 0), Point(rect.right-BORDERLEFT*2, 0), c1, c2);
				g.FillRectangle(&brush2, rect.right-BORDERLEFT*2, rect.bottom-1, BORDERLEFT*2, 1);

				dc.FillSolidRect(BORDERLEFT*2, rect.bottom-1, rect.Width()-BORDERLEFT*4, 1, LineCol);
			}
		}
		else
		{
			dc.FillSolidRect(0, rect.bottom-1, rect.Width(), 1, LineCol);
		}
	}
	else
	{
		dc.FillSolidRect(rect, GetSysColor(COLOR_WINDOW));
	}

	CFont* pOldFont = dc.SelectObject(&((LFApplication*)AfxGetApp())->m_CaptionFont);
	CSize sz = dc.GetTextExtent(m_Caption, m_Caption.GetLength());
	CRect rectText(BORDERLEFT, BORDER, rect.right, BORDER+sz.cy);
	dc.SetTextColor(Themed ? m_CaptionCol : GetSysColor(COLOR_WINDOWTEXT));
	dc.DrawText(m_Caption, rectText, DT_SINGLELINE | DT_END_ELLIPSIS);

	dc.SelectObject(&((LFApplication*)AfxGetApp())->m_DefaultFont);
	sz = dc.GetTextExtent(m_Hint, m_Hint.GetLength());
	rectText.SetRect(BORDERLEFT, rect.bottom-sz.cy-BORDER-1, rect.right, rect.bottom-BORDER-1);
	dc.SetTextColor(Themed ? m_HintCol : GetSysColor(COLOR_WINDOWTEXT));
	dc.DrawText(m_Hint, rectText, DT_SINGLELINE | DT_END_ELLIPSIS);

	dc.SelectObject(pOldFont);

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldBitmap);
}

LRESULT CExplorerHeader::OnThemeChanged()
{
	AdjustLayout();

	return TRUE;
}

HBRUSH CExplorerHeader::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	// Call base class version at first, else it will override changes
	HBRUSH hbr = CWnd::OnCtlColor(pDC, pWnd, nCtlColor);

	if (IsCtrlThemed())
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

void CExplorerHeader::OnLButtonDown(UINT nFlags, CPoint point)
{
	ClientToScreen(&point);
	GetParent()->ScreenToClient(&point);
	GetParent()->SendMessage(WM_LBUTTONDOWN, (WPARAM)nFlags, MAKELPARAM(point.x, point.y));
}

void CExplorerHeader::OnRButtonUp(UINT nFlags, CPoint point)
{
	ClientToScreen(&point);
	GetParent()->ScreenToClient(&point);
	GetParent()->SendMessage(WM_RBUTTONUP, (WPARAM)nFlags, MAKELPARAM(point.x, point.y));
}

void CExplorerHeader::OnSize(UINT nType, INT cx, INT cy)
{
	CWnd::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CExplorerHeader::OnAdjustLayout()
{
	AdjustLayout();
}
