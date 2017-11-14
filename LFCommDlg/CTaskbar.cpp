
// CTaskbar.cpp: Implementierung der Klasse CTaskbar
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CTaskbar
//

#define BORDER           4
#define BORDERLEFT       (BACKSTAGEBORDER-(BORDER+2))
#define MARGIN           16
#define TEXTUREWIDTH     16

CTaskbar::CTaskbar()
	: CFrontstageWnd()
{
	p_ButtonIcons = p_TooltipIcons = NULL;
	m_FirstRight = (UINT)-1;
	m_BackBufferH = 0;
	hBackgroundBrush = NULL;
}

BOOL CTaskbar::Create(CWnd* pParentWnd, CIcons& LargeIcons, CIcons& SmallIcons, UINT ResID, UINT nID)
{
	// Load icons
	p_ButtonIcons = &SmallIcons;
	SmallIcons.Load(ResID);

	p_TooltipIcons = &LargeIcons;
	LargeIcons.Load(ResID, LI_FORTOOLTIPS);

	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, LFGetApp()->LoadStandardCursor(IDC_ARROW));

	return CFrontstageWnd::CreateEx(WS_EX_CONTROLPARENT, className, _T(""), WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE, CRect(0, 0, 0, 0), pParentWnd, nID);
}

BOOL CTaskbar::OnCommand(WPARAM wParam, LPARAM lParam)
{
	return (BOOL)GetOwner()->SendMessage(WM_COMMAND, wParam, lParam);
}

UINT CTaskbar::GetPreferredHeight() const
{
	return 4*BORDER+max(p_ButtonIcons->GetIconSize(), LFGetApp()->m_DefaultFont.GetFontHeight()-2)+(IsCtrlThemed() ? 3 : 2);
}

CTaskButton* CTaskbar::AddButton(UINT nID, INT IconID, BOOL ForceIcon, BOOL AddRight, BOOL ForceSmall)
{
	CString Caption((LPCSTR)nID);
	CString Hint;

	INT Pos = Caption.Find(L'\n');
	if (Pos!=-1)
	{
		Hint = Caption.Left(Pos);
		Caption.Delete(0, Pos+1);

		if (Hint.GetLength()>40)
		{
			Pos = Hint.Find(L' ', Hint.GetLength()/2);
			if (Pos!=-1)
				Hint.SetAt(Pos, L'\n');
		}
	}

	CTaskButton* pTaskButton = new CTaskButton();
	pTaskButton->Create(this, nID, Caption, Hint, p_ButtonIcons, p_TooltipIcons, IconID, AddRight | ForceSmall, !ForceIcon);

	if (AddRight && (m_FirstRight==-1))
		m_FirstRight = m_Buttons.m_ItemCount;

	m_Buttons.AddItem(pTaskButton);

	return pTaskButton;
}

void CTaskbar::AdjustLayout()
{
	if (!m_Buttons.m_ItemCount)
		return;

	CRect rect;
	GetClientRect(rect);

	INT Row = BORDER-1;
	INT Height = rect.Height()-2*BORDER+(IsCtrlThemed() ? 1 : 2);

	INT RPos = rect.right+BORDER-BORDERLEFT;

	for (UINT a=m_Buttons.m_ItemCount-1; a>=m_FirstRight; a--)
	{
		CTaskButton* pTaskButton = m_Buttons[a];
		if (pTaskButton->IsWindowEnabled())
		{
			const INT Width = pTaskButton->GetPreferredWidth();
			RPos -= Width+BORDER;
			if (RPos>=BORDERLEFT)
			{
				pTaskButton->SetWindowPos(NULL, RPos, Row, Width, Height, SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOCOPYBITS);
			}
			else
			{
				pTaskButton->ShowWindow(SW_HIDE);
			}
		}
		else
		{
			pTaskButton->ShowWindow(SW_HIDE);
		}
	}

	UINT FirstSmall = min(m_Buttons.m_ItemCount, m_FirstRight);

Nochmal:
	UINT Count = 0;

	INT LPos = rect.left+BORDERLEFT;
	for (UINT a=0; a<min(m_Buttons.m_ItemCount, m_FirstRight); a++)
	{
		CTaskButton* pTaskButton = m_Buttons[a];
		if (pTaskButton->IsWindowEnabled())
		{
			const INT Width = pTaskButton->GetPreferredWidth(Count++>=FirstSmall);
			if (LPos+Width+MARGIN<RPos)
			{
				pTaskButton->SetWindowPos(NULL, LPos, Row, Width, Height, SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOCOPYBITS);
				pTaskButton->ShowWindow(SW_SHOW);
			}
			else
			{
				if (FirstSmall>0)
				{
					FirstSmall--;
					goto Nochmal;
				}

				pTaskButton->ShowWindow(SW_HIDE);
			}

			LPos += Width+MARGIN;
		}
		else
		{
			pTaskButton->ShowWindow(SW_HIDE);
		}
	}

	RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN);
}

void CTaskbar::DrawTaskbarShadow(Graphics& g, const CRect& rectClient)
{
	g.SetPixelOffsetMode(PixelOffsetModeHalf);

	SolidBrush brush1(Color(0x18000000));
	g.FillRectangle(&brush1, rectClient.left, rectClient.top, rectClient.Width(), 1);

	SolidBrush brush2(Color(0x0C000000));
	g.FillRectangle(&brush2, rectClient.left, rectClient.top+1, rectClient.Width(), 1);
}


BEGIN_MESSAGE_MAP(CTaskbar, CFrontstageWnd)
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_THEMECHANGED()
	ON_WM_CTLCOLOR()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_MESSAGE_VOID(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
	ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()

void CTaskbar::OnDestroy()
{
	for (UINT a=0; a<m_Buttons.m_ItemCount; a++)
	{
		CTaskButton* pTaskButton = m_Buttons[a];
		pTaskButton->DestroyWindow();
		delete pTaskButton;
	}

	DeleteObject(hBackgroundBrush);

	CFrontstageWnd::OnDestroy();
}

BOOL CTaskbar::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CTaskbar::OnPaint()
{
	CPaintDC pDC(this);

	CRect rect;
	GetClientRect(rect);

	if (IsCtrlThemed())
	{
		if (m_BackBufferH!=rect.Height())
		{
			m_BackBufferH = rect.Height();

			DeleteObject(hBackgroundBrush);

			CDC dc;
			dc.CreateCompatibleDC(&pDC);
			dc.SetBkMode(TRANSPARENT);

			CBitmap MemBitmap;
			MemBitmap.CreateCompatibleBitmap(&pDC, TEXTUREWIDTH, rect.Height());
			CBitmap* pOldBitmap = dc.SelectObject(&MemBitmap);

			const UINT Line = (rect.Height()-2)*2/5;

			dc.FillSolidRect(0, 0, TEXTUREWIDTH, Line, 0xFFFFFF);
			dc.FillSolidRect(0, rect.bottom-1, TEXTUREWIDTH, 1, 0x97908B);

			Graphics g(dc);
			g.SetPixelOffsetMode(PixelOffsetModeHalf);

			LinearGradientBrush brush(Point(0, Line), Point(0, rect.bottom), Color(0xFFFFFFFF), Color(0xFFE5E9EE));
			g.FillRectangle(&brush, 0, Line, TEXTUREWIDTH, rect.bottom-Line-1);

			dc.SelectObject(pOldBitmap);

			hBackgroundBrush = CreatePatternBrush(MemBitmap);
		}

		FillRect(pDC, rect, hBackgroundBrush);
	}
	else
	{
		DeleteObject(hBackgroundBrush);
		hBackgroundBrush = NULL;

		pDC.FillSolidRect(rect, GetSysColor(COLOR_3DFACE));
	}
}

void CTaskbar::OnSysColorChange()
{
	m_BackBufferH = 0;
}

LRESULT CTaskbar::OnThemeChanged()
{
	m_BackBufferH = 0;
	AdjustLayout();

	return NULL;
}

HBRUSH CTaskbar::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	// Call base class version at first, else it will override changes
	HBRUSH hBrush = CFrontstageWnd::OnCtlColor(pDC, pWnd, nCtlColor);

	if (hBackgroundBrush)
		if ((nCtlColor==CTLCOLOR_BTN) || (nCtlColor==CTLCOLOR_STATIC))
		{
			CRect rect;
			pWnd->GetWindowRect(rect);
			ScreenToClient(rect);

			pDC->SetBrushOrg(-rect.left, -rect.top);

			hBrush = hBackgroundBrush;
		}

	return hBrush;
}

void CTaskbar::OnSize(UINT nType, INT cx, INT cy)
{
	CFrontstageWnd::OnSize(nType, cx, cy);

	OnIdleUpdateCmdUI();
	AdjustLayout();
}

void CTaskbar::OnSetFocus(CWnd* /*pOldWnd*/)
{
	for (UINT a=0; a<m_Buttons.m_ItemCount; a++)
		if (m_Buttons[a]->IsWindowEnabled())
		{
			m_Buttons[a]->SetFocus();
			break;
		}
}

void CTaskbar::OnIdleUpdateCmdUI()
{
	BOOL Update = FALSE;

	for (UINT a=0; a<m_Buttons.m_ItemCount; a++)
	{
		CTaskButton* pTaskButton = m_Buttons[a];
		BOOL Enabled = pTaskButton->IsWindowEnabled();

		CCmdUI cmdUI;
		cmdUI.m_nID = pTaskButton->GetDlgCtrlID();
		cmdUI.m_pOther = pTaskButton;
		cmdUI.DoUpdate(GetOwner(), TRUE);

		Update |= (pTaskButton->IsWindowEnabled()!=Enabled);
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

	CMenu Menu;
	if (!Menu.CreatePopupMenu())
		return;

	BOOL NeedsSeparator = FALSE;

	for (UINT a=0; a<m_Buttons.m_ItemCount; a++)
	{
		if ((INT)a==m_FirstRight)
			NeedsSeparator = (Menu.GetMenuItemCount()>0);

		CTaskButton* pTaskButton = m_Buttons[a];
		if (pTaskButton->IsWindowEnabled())
		{
			if (NeedsSeparator)
			{
				Menu.AppendMenu(MF_SEPARATOR);
				NeedsSeparator = FALSE;
			}

			CString tmpStr;
			pTaskButton->GetWindowText(tmpStr);

			Menu.AppendMenu(0, pTaskButton->GetDlgCtrlID(), _T("&")+tmpStr);
		}
	}

	Menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pos.x, pos.y, this, NULL);
}
