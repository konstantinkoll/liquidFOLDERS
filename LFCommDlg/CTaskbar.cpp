
// CTaskbar.cpp: Implementierung der Klasse CTaskbar
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CTaskbar
//

#define BORDERLEFT      16
#define BORDER          4

CTaskbar::CTaskbar()
	: CWnd()
{
	hBackgroundBrush = NULL;
	BackBufferL = BackBufferH = 0;
}

BOOL CTaskbar::Create(CWnd* pParentWnd, UINT ResID, UINT nID)
{
	Icons.SetImageSize(CSize(16, 16));
	Icons.Load(ResID);

	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, LoadCursor(NULL, IDC_ARROW));

	const DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE;
	CRect rect;
	rect.SetRectEmpty();
	return CWnd::CreateEx(WS_EX_CONTROLPARENT, className, _T("Taskbar"), dwStyle, rect, pParentWnd, nID);
}

LRESULT CTaskbar::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message==WM_COMMAND)
		return GetParent()->SendMessage(message, wParam, lParam);

	return CWnd::DefWindowProc(message, wParam, lParam);
}

UINT CTaskbar::GetPreferredHeight()
{
	LOGFONT lf;
	UINT h = 4*BORDER+(IsCtrlThemed() ? 4 : 3);

	((LFApplication*)AfxGetApp())->m_DefaultFont.GetLogFont(&lf);
	h += abs(lf.lfHeight);

	return h;
}

CTaskButton* CTaskbar::AddButton(UINT nID, CString Text, int IconID, BOOL bAddRight, BOOL bOnlyIcon)
{
	CTaskButton* btn = new CTaskButton();
	btn->Create(bOnlyIcon ? _T("") : Text, bOnlyIcon ? Text : _T(""), &Icons,
		bOnlyIcon || (((LFApplication*)AfxGetApp())->OSVersion<OS_Seven) ? IconID : -1,
		this, nID);

	CCmdUI cmdUI;
	cmdUI.m_nID = nID;
	cmdUI.m_pOther = btn;
	cmdUI.DoUpdate(GetParent(), TRUE);

	list<CTaskButton*>* li = bAddRight ? &ButtonsRight : &ButtonsLeft;
	li->push_back(btn);

	return btn;
}

void CTaskbar::AdjustLayout()
{
	CRect rect;
	GetClientRect(rect);

	int Row = BORDER-1;
	int h = rect.Height()-2*BORDER+(IsCtrlThemed() ? 1 : 2);

	int RPos = rect.right+2*BORDER-BORDERLEFT;
	std::list<CTaskButton*>::reverse_iterator ppBtnR = ButtonsRight.rbegin();
	while (ppBtnR!=ButtonsRight.rend())
	{
		if ((*ppBtnR)->IsWindowEnabled())
		{
			int l = (*ppBtnR)->GetPreferredWidth();
			RPos -= l+BORDER;
			if (RPos>=BORDERLEFT)
			{
				(*ppBtnR)->SetWindowPos(NULL, RPos, Row, l, h, SWP_NOZORDER | SWP_NOACTIVATE);
				(*ppBtnR)->Invalidate();
				(*ppBtnR)->ShowWindow(SW_SHOW);
			}
			else
			{
				(*ppBtnR)->ShowWindow(SW_HIDE);
			}
		}
		else
		{
			(*ppBtnR)->ShowWindow(SW_HIDE);
		}

		ppBtnR++;
	}

	int LPos = rect.left+BORDERLEFT-BORDER;
	std::list<CTaskButton*>::iterator ppBtn = ButtonsLeft.begin();
	while (ppBtn!=ButtonsLeft.end())
	{
		if ((*ppBtn)->IsWindowEnabled())
		{
			int l = (*ppBtn)->GetPreferredWidth();
			if (LPos+l+BORDERLEFT-BORDER<RPos)
			{
				(*ppBtn)->SetWindowPos(NULL, LPos, Row, l, h, SWP_NOZORDER | SWP_NOACTIVATE);
				(*ppBtn)->Invalidate();
				(*ppBtn)->ShowWindow(SW_SHOW);
			}
			else
			{
				(*ppBtn)->ShowWindow(SW_HIDE);
			}
			LPos += l+BORDERLEFT;
		}
		else
		{
			(*ppBtn)->ShowWindow(SW_HIDE);
		}

		ppBtn++;
	}
}


BEGIN_MESSAGE_MAP(CTaskbar, CWnd)
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_CTLCOLOR()
	ON_WM_SIZE()
	ON_MESSAGE_VOID(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
END_MESSAGE_MAP()

void CTaskbar::OnDestroy()
{
	CWnd::OnDestroy();

	std::list<CTaskButton*>::iterator ppBtn = ButtonsRight.begin();
	while (ppBtn!=ButtonsRight.end())
	{
		(*ppBtn)->DestroyWindow();
		delete *ppBtn;
		ppBtn++;
	}

	ppBtn = ButtonsLeft.begin();
	while (ppBtn!=ButtonsLeft.end())
	{
		(*ppBtn)->DestroyWindow();
		delete *ppBtn;
		ppBtn++;
	}

	if (hBackgroundBrush)
		DeleteObject(hBackgroundBrush);
}

BOOL CTaskbar::OnEraseBkgnd(CDC* pDC)
{
	CRect rect;
	GetClientRect(rect);

	CDC dc;
	dc.CreateCompatibleDC(pDC);
	dc.SetBkMode(TRANSPARENT);

	CBitmap* pOldBitmap;
	if ((BackBufferL!=rect.Width()) || (BackBufferH!=rect.Height()))
	{
		BackBufferL = rect.Width();
		BackBufferH = rect.Height();

		BackBuffer.DeleteObject();
		BackBuffer.CreateCompatibleBitmap(pDC, rect.Width(), rect.Height());
		pOldBitmap = dc.SelectObject(&BackBuffer);

		Graphics g(dc);

		if (!IsCtrlThemed())
		{
			dc.FillSolidRect(rect, GetSysColor(COLOR_3DFACE));
		}
		else
			switch (((LFApplication*)AfxGetApp())->OSVersion)
			{
			case OS_XP:
				{
					dc.FillSolidRect(rect, GetSysColor(COLOR_3DFACE));

					Color c1;
					c1.SetFromCOLORREF(GetSysColor(COLOR_3DHIGHLIGHT));
					Color c2;
					c2.SetFromCOLORREF(GetSysColor(COLOR_3DFACE));
					Color c3;
					c3.SetFromCOLORREF(GetSysColor(COLOR_3DSHADOW));
				
					UINT border = rect.Height()/4;

					LinearGradientBrush brush3(Point(0, 0), Point(0, border), c1, c2);
					g.FillRectangle(&brush3, 0, 0, rect.right, border);

					LinearGradientBrush brush4(Point(0, rect.bottom-border), Point(0, rect.bottom), c2, c3);
					g.FillRectangle(&brush4, 0, rect.bottom-border, rect.right, border);

					break;
				}
			case OS_Vista:
				{
					Color c1(0x04, 0x48, 0x75);
					Color c2(0x19, 0x6C, 0x77);
					Color c3(0x80, 0xC0, 0xFF, 0xE0);

					LinearGradientBrush brush1(Point(0, 0), Point(rect.right, 0), c1, c2);
					g.FillRectangle(&brush1, 0, 0, rect.right, rect.bottom);

					SolidBrush brush2(Color(0x60, 0x00, 0x00, 0x00));
					g.FillRectangle(&brush2, 0, rect.bottom-1, rect.right, 1);

					UINT line = rect.Height()/2;

					LinearGradientBrush brush3(Point(0, 0), Point(0, line), Color(128, 0xFF, 0xFF, 0xFF), Color(24, 0xFF, 0xFF, 0xFF));
					g.FillRectangle(&brush3, 0, 0, rect.right, line);

					LinearGradientBrush brush4(Point(0, line), Point(0, rect.bottom-1), Color(0, 0xFF, 0xFF, 0xFF), c3);
					g.FillRectangle(&brush4, 0, line+1, rect.right, rect.bottom-line-2);

					SolidBrush brush5(Color(64, 0xFF, 0xFF, 0xFF));
					g.FillRectangle(&brush5, 0, 0, rect.right, 1);
					g.FillRectangle(&brush5, 0, rect.bottom-2, rect.right, 1);
					g.FillRectangle(&brush5, 0, 0, 1, rect.bottom-1);
					g.FillRectangle(&brush5, rect.right-1, 0, 1, rect.bottom-1);
				}
			case OS_Seven:
				{
					UINT line = (rect.Height()-2)/2;

					LinearGradientBrush brush1(Point(0, 0), Point(0, line+1), Color(0xFD, 0xFE, 0xFF), Color(0xE6, 0xF0, 0xFA));
					g.FillRectangle(&brush1, 1, 0, rect.right-2, line+1);

					LinearGradientBrush brush2(Point(0, line+2), Point(0, rect.bottom-4), Color(0xDC, 0xE6, 0xF4), Color(0xDD, 0xE9, 0xF7));
					g.FillRectangle(&brush2, 1, line+1, rect.right-2, rect.bottom-line-4);

					LinearGradientBrush brush3(Point(0, 1), Point(0, rect.bottom-4), Color(0xFF, 0xFF, 0xFF), Color(0xEE, 0xF4, 0xFB));
					g.FillRectangle(&brush3, 0, 0, 1, rect.bottom-3);
					g.FillRectangle(&brush3, rect.right-1, 0, 1, rect.bottom-3);

					dc.FillSolidRect(0, rect.bottom-3, rect.right, 1, 0xFBEFE4);
					dc.FillSolidRect(0, rect.bottom-2, rect.right, 1, 0xEADACD);
					dc.FillSolidRect(0, rect.bottom-1, rect.right, 1, 0xC3AFA0);
					break;
				}
			}

		if (hBackgroundBrush)
			DeleteObject(hBackgroundBrush);
		hBackgroundBrush = CreatePatternBrush(BackBuffer);
	}
	else
	{
		pOldBitmap = dc.SelectObject(&BackBuffer);
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
	BackBufferL = BackBufferH = 0;
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

void CTaskbar::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CTaskbar::OnIdleUpdateCmdUI()
{
	BOOL Update = FALSE;

	std::list<CTaskButton*>::iterator ppBtn = ButtonsRight.begin();
	while (ppBtn!=ButtonsRight.end())
	{
		BOOL Enabled = (*ppBtn)->IsWindowEnabled();

		CCmdUI cmdUI;
		cmdUI.m_nID = (*ppBtn)->GetDlgCtrlID();
		cmdUI.m_pOther = *ppBtn;
		cmdUI.DoUpdate(GetParent(), TRUE);

		Update |= ((*ppBtn)->IsWindowEnabled()!=Enabled);
		ppBtn++;
	}

	ppBtn = ButtonsLeft.begin();
	while (ppBtn!=ButtonsLeft.end())
	{
		BOOL Enabled = (*ppBtn)->IsWindowEnabled();

		CCmdUI cmdUI;
		cmdUI.m_nID = (*ppBtn)->GetDlgCtrlID();
		cmdUI.m_pOther = *ppBtn;
		cmdUI.DoUpdate(GetParent(), TRUE);

		Update |= ((*ppBtn)->IsWindowEnabled()!=Enabled);
		ppBtn++;
	}

	if (Update)
		AdjustLayout();
}
