
// CTaskbar.cpp: Implementierung der Klasse CTaskbar
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CTaskbar
//

#define BORDERLEFT     16
#define BORDER         4

CTaskbar::CTaskbar()
	: CWnd()
{
	hBackgroundBrush = NULL;
	m_BackBufferL = m_BackBufferH = 0;
}

BOOL CTaskbar::Create(CWnd* pParentWnd, UINT LargeResID, UINT SmallResID, UINT nID)
{
	LOGFONT lf;
	LFGetApp()->m_DefaultFont.GetLogFont(&lf);

	m_IconSize = abs(lf.lfHeight)>=24 ? 32 : 16;
	m_Icons.SetImageSize(CSize(m_IconSize, m_IconSize));
	m_Icons.Load(m_IconSize==32 ? LargeResID : SmallResID);

	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, LFGetApp()->LoadStandardCursor(IDC_ARROW));

	CRect rect;
	rect.SetRectEmpty();
	return CWnd::CreateEx(WS_EX_CONTROLPARENT, className, _T(""), WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE, rect, pParentWnd, nID);
}

BOOL CTaskbar::OnCommand(WPARAM wParam, LPARAM lParam)
{
	return (BOOL)GetOwner()->SendMessage(WM_COMMAND, wParam, lParam);
}

UINT CTaskbar::GetPreferredHeight()
{
	LOGFONT lf;
	LFGetApp()->m_DefaultFont.GetLogFont(&lf);

	return 4*BORDER+max(m_IconSize-2, abs(lf.lfHeight))+(IsCtrlThemed() ? 4 : 3);
}

CTaskButton* CTaskbar::AddButton(UINT nID, INT IconID, BOOL ForceIcon, BOOL AddRight, BOOL SupressCaption)
{
	CString Caption((LPCSTR)nID);
	CString Hint;

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

	CTaskButton* btn = new CTaskButton();
	btn->Create(this, nID, AddRight || SupressCaption ? _T("") : Caption, Caption, Hint, &m_Icons, m_IconSize,
		ForceIcon || AddRight || SupressCaption || (LFGetApp()->OSVersion<OS_Seven) ? IconID : -1);

	if (AddRight)
	{
		m_ButtonsRight.AddHead(btn);
	}
	else
	{
		m_ButtonsLeft.AddTail(btn);
	}

	return btn;
}

void CTaskbar::AdjustLayout()
{
	SetRedraw(FALSE);

	CRect rect;
	GetClientRect(rect);

	INT Row = BORDER-1;
	INT h = rect.Height()-2*BORDER+(IsCtrlThemed() ? 1 : 2);

	INT RPos = rect.right+2*BORDER-BORDERLEFT;
	for (POSITION p=m_ButtonsRight.GetHeadPosition(); p; )
	{
		CTaskButton* btn = m_ButtonsRight.GetNext(p);
		if (btn->IsWindowEnabled())
		{
			INT l = btn->GetPreferredWidth();
			RPos -= l+BORDER;
			if (RPos>=BORDERLEFT)
			{
				btn->SetWindowPos(NULL, RPos, Row, l, h, SWP_NOZORDER | SWP_NOACTIVATE);
				btn->ShowWindow(SW_SHOW);
			}
			else
			{
				btn->ShowWindow(SW_HIDE);
			}
		}
		else
		{
			btn->ShowWindow(SW_HIDE);
		}
	}

	INT LPos = rect.left+BORDERLEFT-BORDER;
	for (POSITION p=m_ButtonsLeft.GetHeadPosition(); p; )
	{
		CTaskButton* btn = m_ButtonsLeft.GetNext(p);
		if (btn->IsWindowEnabled())
		{
			INT l = btn->GetPreferredWidth();
			if (LPos+l+BORDERLEFT-BORDER<RPos)
			{
				btn->SetWindowPos(NULL, LPos, Row, l, h, SWP_NOZORDER | SWP_NOACTIVATE);
				btn->ShowWindow(SW_SHOW);
			}
			else
			{
				btn->ShowWindow(SW_HIDE);
			}
			LPos += l+BORDERLEFT;
		}
		else
		{
			btn->ShowWindow(SW_HIDE);
		}
	}

	SetRedraw(TRUE);
	RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN);
}


BEGIN_MESSAGE_MAP(CTaskbar, CWnd)
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_THEMECHANGED()
	ON_WM_CTLCOLOR()
	ON_WM_SIZE()
	ON_MESSAGE_VOID(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
	ON_WM_CONTEXTMENU()
	ON_WM_SETFOCUS()
END_MESSAGE_MAP()

void CTaskbar::OnDestroy()
{
	for (POSITION p=m_ButtonsRight.GetHeadPosition(); p; )
	{
		CTaskButton* btn = m_ButtonsRight.GetNext(p);
		btn->DestroyWindow();
		delete btn;
	}

	for (POSITION p=m_ButtonsLeft.GetHeadPosition(); p; )
	{
		CTaskButton* btn = m_ButtonsLeft.GetNext(p);
		btn->DestroyWindow();
		delete btn;
	}

	if (hBackgroundBrush)
		DeleteObject(hBackgroundBrush);

	CWnd::OnDestroy();
}

BOOL CTaskbar::OnEraseBkgnd(CDC* pDC)
{
	CRect rect;
	GetClientRect(rect);

	CDC dc;
	dc.CreateCompatibleDC(pDC);
	dc.SetBkMode(TRANSPARENT);

	CBitmap* pOldBitmap;
	if ((m_BackBufferL!=rect.Width()) || (m_BackBufferH!=rect.Height()))
	{
		m_BackBufferL = rect.Width();
		m_BackBufferH = rect.Height();

		m_BackBuffer.DeleteObject();
		m_BackBuffer.CreateCompatibleBitmap(pDC, rect.Width(), rect.Height());
		pOldBitmap = dc.SelectObject(&m_BackBuffer);

		if (IsCtrlThemed())
		{
			dc.FillSolidRect(0, 0, rect.Width(), rect.Height()-1, 0xFFFFFF);
			dc.FillSolidRect(0, rect.bottom-1, rect.Width(), 1, 0x97908B);

			const UINT line = (rect.Height()-2)*2/5;

			Graphics g(dc);
			LinearGradientBrush brush(Point(0, line-1), Point(0, rect.bottom-1), Color(0xFF, 0xFF, 0xFF), Color(0xE5, 0xE9, 0xEE));

			if (GetParent()->GetStyle() & WS_BORDER)
			{
				g.FillRectangle(&brush, 0, line, rect.right, rect.bottom-line-1);
			}
			else
			{
				g.FillRectangle(&brush, 1, line, rect.right-2, rect.bottom-line-1);
			}
		}
		else
		{
			dc.FillSolidRect(rect, GetSysColor(COLOR_3DFACE));
		}

		if (hBackgroundBrush)
			DeleteObject(hBackgroundBrush);
		hBackgroundBrush = CreatePatternBrush(m_BackBuffer);
	}
	else
	{
		pOldBitmap = dc.SelectObject(&m_BackBuffer);
	}

	dc.SelectObject(pOldBitmap);
	return TRUE;
}

void CTaskbar::OnPaint()
{
	CPaintDC pDC(this);

	CRect rect;
	GetClientRect(rect);

	FillRect(pDC, rect, hBackgroundBrush);
}

void CTaskbar::OnSysColorChange()
{
	m_BackBufferL = m_BackBufferH = 0;
}

LRESULT CTaskbar::OnThemeChanged()
{
	m_BackBufferL = m_BackBufferH = 0;
	AdjustLayout();

	return TRUE;
}

HBRUSH CTaskbar::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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

void CTaskbar::OnSize(UINT nType, INT cx, INT cy)
{
	CWnd::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CTaskbar::OnIdleUpdateCmdUI()
{
	BOOL Update = FALSE;

	for (POSITION p=m_ButtonsRight.GetHeadPosition(); p; )
	{
		CTaskButton* btn = m_ButtonsRight.GetNext(p);
		BOOL Enabled = btn->IsWindowEnabled();

		CCmdUI cmdUI;
		cmdUI.m_nID = btn->GetDlgCtrlID();
		cmdUI.m_pOther = btn;
		cmdUI.DoUpdate(GetOwner(), TRUE);

		if (btn->IsWindowEnabled()!=Enabled)
			Update = TRUE;
	}

	for (POSITION p=m_ButtonsLeft.GetHeadPosition(); p; )
	{
		CTaskButton* btn = m_ButtonsLeft.GetNext(p);
		BOOL Enabled = btn->IsWindowEnabled();

		CCmdUI cmdUI;
		cmdUI.m_nID = btn->GetDlgCtrlID();
		cmdUI.m_pOther = btn;
		cmdUI.DoUpdate(GetOwner(), TRUE);

		if (btn->IsWindowEnabled()!=Enabled)
			Update = TRUE;
	}

	if (Update)
		AdjustLayout();
}

void CTaskbar::OnContextMenu(CWnd* /*pWnd*/, CPoint pos)
{
	if ((pos.x<0) || (pos.y<0))
	{
		CRect rect;
		GetClientRect(rect);

		pos.x = (rect.left+rect.right)/2;
		pos.y = (rect.top+rect.bottom)/2;
		ClientToScreen(&pos);
	}

	CMenu menu;
	if (!menu.CreatePopupMenu())
		return;

	for (POSITION p=m_ButtonsLeft.GetHeadPosition(); p; )
	{
		CTaskButton* btn = m_ButtonsLeft.GetNext(p);
		if (btn->IsWindowEnabled())
		{
			CString tmpStr;
			btn->GetWindowText(tmpStr);
			menu.AppendMenu(0, btn->GetDlgCtrlID(), _T("&")+tmpStr);
		}
	}

	BOOL NeedsSeparator = menu.GetMenuItemCount();

	for (POSITION p=m_ButtonsRight.GetTailPosition(); p; )
	{
		CTaskButton* btn = m_ButtonsRight.GetPrev(p);
		if (btn->IsWindowEnabled())
		{
			if (NeedsSeparator)
			{
				menu.AppendMenu(MF_SEPARATOR);
				NeedsSeparator = FALSE;
			}

			CString tmpStr;
			btn->GetWindowText(tmpStr);
			menu.AppendMenu(0, btn->GetDlgCtrlID(), _T("&")+tmpStr);
		}
	}

	menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pos.x, pos.y, this, NULL);
}

void CTaskbar::OnSetFocus(CWnd* /*pOldWnd*/)
{
	for (POSITION p=m_ButtonsLeft.GetHeadPosition(); p; )
	{
		CTaskButton* btn = m_ButtonsLeft.GetNext(p);
		if (btn->IsWindowEnabled())
		{
			btn->SetFocus();
			break;
		}
	}
}
