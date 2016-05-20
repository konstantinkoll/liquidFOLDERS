
// CHeaderArea.cpp: Implementierung der Klasse CHeaderArea
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CHeaderArea
//

#define BORDER     BACKSTAGEBORDER
#define MARGIN     4

CHeaderArea::CHeaderArea()
	: CFrontstageWnd()
{
	hBackgroundBrush = NULL;
	m_BackBufferL = m_BackBufferH = m_RightEdge = 0;
}

BOOL CHeaderArea::Create(CWnd* pParentWnd, UINT nID, BOOL Shadow)
{
	m_Shadow = Shadow;

	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, LFGetApp()->LoadStandardCursor(IDC_ARROW));

	return CFrontstageWnd::CreateEx(WS_EX_CONTROLPARENT, className, _T(""), WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE, CRect(0, 0, 0, 0), pParentWnd, nID);
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
			pWnd->GetWindowRect(rectWindow);

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

void CHeaderArea::SetText(LPCWSTR Caption, LPCWSTR Hint, BOOL Repaint)
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
	UINT Height = 2*BORDER+MARGIN+LFGetApp()->m_CaptionFont.GetFontHeight()+LFGetApp()->m_DefaultFont.GetFontHeight();

	return max(Height, max(60, (UINT)m_Buttons.m_ItemCount*(LFGetApp()->m_DefaultFont.GetFontHeight()+8+MARGIN/2)+MARGIN+MARGIN/2));
}

CHeaderButton* CHeaderArea::AddButton(UINT nID)
{
	CString Caption;
	CString Hint;

	if (nID)
	{
		ENSURE(Caption.LoadString(nID));

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
	}

	CHeaderButton* pHeaderButton = new CHeaderButton();
	pHeaderButton->Create(this, nID, Caption, Hint);

	m_Buttons.AddItem(pHeaderButton);

	return pHeaderButton;
}

void CHeaderArea::AdjustLayout()
{
	CRect rect;
	GetClientRect(rect);

	m_RightEdge = rect.right;
	INT Row = max(MARGIN, (rect.Height()-(UINT)m_Buttons.m_ItemCount*(LFGetApp()->m_DefaultFont.GetFontHeight()+8+MARGIN/2)+MARGIN/2)/2);

	for (UINT a=0; a<m_Buttons.m_ItemCount; a++)
	{
		CHeaderButton* pHeaderButton = m_Buttons[a];

		CSize Size;
		INT CaptionWidth;
		pHeaderButton->GetPreferredSize(&Size, CaptionWidth);
		pHeaderButton->SetWindowPos(NULL, rect.right-Size.cx-BORDER+6, Row, Size.cx, Size.cy, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOCOPYBITS);

		m_RightEdge = min(m_RightEdge, rect.right-Size.cx-(INT)CaptionWidth-2*BORDER-MARGIN+6);

		Row += Size.cy+MARGIN/2;
	}

	m_BackBufferL = m_BackBufferH = 0;
	RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN);
}


BEGIN_MESSAGE_MAP(CHeaderArea, CFrontstageWnd)
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
	for (UINT a=0; a<m_Buttons.m_ItemCount; a++)
	{
		CHeaderButton* pHeaderButton = m_Buttons[a];
		pHeaderButton->DestroyWindow();
		delete pHeaderButton;
	}

	DeleteObject(hBackgroundBrush);

	CFrontstageWnd::OnDestroy();
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

	if ((m_BackBufferL!=rect.Width()) || (m_BackBufferH!=rect.Height()))
	{
		m_BackBufferL = rect.Width();
		m_BackBufferH = rect.Height();

		DeleteObject(hBackgroundBrush);

		CDC dc;
		dc.CreateCompatibleDC(&pDC);
		dc.SetBkMode(TRANSPARENT);

		CBitmap MemBitmap;
		MemBitmap.CreateCompatibleBitmap(&pDC, rect.Width(), rect.Height());
		CBitmap* pOldBitmap = dc.SelectObject(&MemBitmap);

		Graphics g(dc);

		BOOL Themed = IsCtrlThemed();
		if (Themed)
		{
			dc.FillSolidRect(rect, 0xFFFFFF);

			Bitmap* pDivider = LFGetApp()->GetCachedResourceImage(IDB_DIVUP);
			g.DrawImage(pDivider, (rect.Width()-(INT)pDivider->GetWidth())/2, rect.Height()-(INT)pDivider->GetHeight());

			if (m_Shadow)
			{
				CTaskbar::DrawTaskbarShadow(g, rect);
			}
			else
			{
				dc.FillSolidRect(0, 0, rect.Width(), 1, 0xFFFFFF);
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

		CFont* pOldFont = dc.SelectObject(&LFGetApp()->m_DefaultFont);

		dc.SetTextColor(Themed ? 0x333333 : GetSysColor(COLOR_WINDOWTEXT));

		for (UINT a=0; a<m_Buttons.m_ItemCount; a++)
		{
			const CHeaderButton* pHeaderButton = m_Buttons[a];

			CRect rect;
			pHeaderButton->GetWindowRect(rect);
			ScreenToClient(rect);

			CString Caption;
			INT CaptionWidth;
			pHeaderButton->GetCaption(Caption, CaptionWidth);

			CRect rectCaption(rect.left-CaptionWidth-MARGIN, rect.top, rect.left, rect.bottom);
			dc.DrawText(Caption, rectCaption, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
		}

		if (m_RightEdge-BORDER>=32)
		{
			dc.SetTextColor(Themed ? 0x404040 : GetSysColor(COLOR_WINDOWTEXT));

			CRect rectText(BORDER, rect.bottom-dc.GetTextExtent(m_Hint).cy-BORDER-1, m_RightEdge, rect.bottom-BORDER-1);
			dc.DrawText(m_Hint, rectText, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

			dc.SelectObject(&LFGetApp()->m_CaptionFont);
			rectText.top = BORDER;
			dc.DrawText(m_Caption, rectText, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
		}

		dc.SelectObject(pOldFont);
		dc.SelectObject(pOldBitmap);

		hBackgroundBrush = CreatePatternBrush(MemBitmap);
	}

	FillRect(pDC, rect, hBackgroundBrush);
}

LRESULT CHeaderArea::OnThemeChanged()
{
	AdjustLayout();

	return NULL;
}

HBRUSH CHeaderArea::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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
	CFrontstageWnd::OnSize(nType, cx, cy);

	AdjustLayout();
}

void CHeaderArea::OnAdjustLayout()
{
	AdjustLayout();
}
