
// LFDialog.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "LFCommDlg.h"


BOOL IsPushbutton(CWnd* pWnd)
{
	if (pWnd->SendMessage(WM_GETDLGCODE) & (DLGC_BUTTON | DLGC_DEFPUSHBUTTON | DLGC_UNDEFPUSHBUTTON))
	{
		DWORD dwStyle = pWnd->GetStyle() & BS_TYPEMASK;

		if ((dwStyle==BS_PUSHBUTTON) || (dwStyle==BS_DEFPUSHBUTTON) || (dwStyle==BS_OWNERDRAW))
			return TRUE;
	}

	return FALSE;
}


// LFDialog
//

LFDialog::LFDialog(UINT nIDTemplate, CWnd* pParentWnd, BOOL UAC)
	: CDialog(nIDTemplate, pParentWnd)
{
	if (UAC)
	{
		m_wndDesktopDimmer.Create(this);
		m_pParentWnd = &m_wndDesktopDimmer;
	}

	m_UAC = UAC;

	m_ShowKeyboardCues = FALSE;
	SystemParametersInfo(SPI_GETKEYBOARDCUES, 0, &m_ShowKeyboardCues, 0);

	hIconShield = NULL;
	hBackgroundBrush = NULL;
	m_BackBufferL = m_BackBufferH = m_UACHeight = 0;
	p_BottomLeftControl = NULL;
}

BOOL LFDialog::PreTranslateMessage(MSG* pMsg)
{
	if ((pMsg->message==WM_SYSKEYDOWN) && (!m_ShowKeyboardCues))
	{
		m_ShowKeyboardCues = TRUE;

		for (UINT a=0; a<m_Buttons.m_ItemCount; a++)
			m_Buttons.m_Items[a]->Invalidate();
	}

	return CDialog::PreTranslateMessage(pMsg);
}

void LFDialog::DoDataExchange(CDataExchange* pDX)
{
	for (UINT a=0; a<4; a++)
	{
		CWnd* pWnd = GetDlgItem(IDC_CATEGORY1+a);
		if (pWnd)
			DDX_Control(pDX, IDC_CATEGORY1+a, m_wndCategory[a]);
	}
}

CWnd* LFDialog::GetBottomWnd() const
{
	CWnd* pBottomWnd;
	
	if (m_BottomRightControls.m_ItemCount)
	{
		pBottomWnd = m_BottomRightControls.m_Items[0];
	}
	else
	{
		pBottomWnd = GetDlgItem(IDOK);

		if (!pBottomWnd)
			pBottomWnd = GetDlgItem(IDCANCEL);
	}

	return pBottomWnd;
}

void LFDialog::OnEraseBkgnd(CDC& dc, Graphics& g, CRect& rect)
{
	CWnd* pBottomWnd = GetBottomWnd();
	if (!pBottomWnd)
	{
		rect.SetRectEmpty();
		return;
	}

	CRect rectBorders(0, 0, 7, 7);
	MapDialogRect(&rectBorders);

	CRect rectLayout;
	GetLayoutRect(rectLayout);
	INT Line = rectLayout.bottom;

	BOOL Themed = IsCtrlThemed();

	// Background
	dc.FillSolidRect(0, 0, m_BackBufferL, Line, 0xFFFFFF);
	if (Themed)
	{
		dc.FillSolidRect(0, Line, m_BackBufferL, rect.Height()-Line, 0xFFFFFF);

		CGdiPlusBitmap* pDivider = LFGetApp()->GetCachedResourceImage(IDB_DIVDOWN, _T("PNG"));
		g.DrawImage(pDivider->m_pBitmap, (rect.Width()-(INT)pDivider->m_pBitmap->GetWidth())/2, Line);

		// Child windows
		for (UINT a=0; a<m_Buttons.m_ItemCount; a++)
			if (m_Buttons.m_Items[a]->IsWindowVisible())
			{
				CRect rectBounds;
				m_Buttons.m_Items[a]->GetWindowRect(rectBounds);
				ScreenToClient(rectBounds);

				DrawWhiteButtonBorder(g, rectBounds);
			}
	}
	else
	{
		dc.FillSolidRect(0, Line, m_BackBufferL, rect.Height()-Line, GetSysColor(COLOR_3DFACE));
	}

	// UAC
	if (m_UAC)
	{
		if (Themed)
		{
			g.SetPixelOffsetMode(PixelOffsetModeHalf);

			LinearGradientBrush brush2(Point(0, 0), Point(m_BackBufferL, 0), Color(4, 80, 130), Color(28, 120, 133));
			g.FillRectangle(&brush2, 0, 0, m_BackBufferL, m_UACHeight);
			dc.SetTextColor(0xFFFFFF);
		}
		else
		{
			dc.FillSolidRect(0, 0, m_BackBufferL, m_UACHeight, GetSysColor(COLOR_HIGHLIGHT));
			dc.SetTextColor(GetSysColor(COLOR_HIGHLIGHTTEXT));
		}

		DrawIconEx(dc, rectBorders.right-m_ShieldSize/16, (m_UACHeight-m_ShieldSize)/2, hIconShield, m_ShieldSize, m_ShieldSize, 0, NULL, DI_NORMAL);

		CRect rectText(rect);
		rectText.left = rectBorders.right+rectBorders.right/4+m_ShieldSize;
		rectText.bottom = m_UACHeight;

		CString tmpStr((LPCSTR)IDS_UACMESSAGE);

		CFont* pOldFont = dc.SelectObject(&LFGetApp()->m_UACFont);
		dc.DrawText(tmpStr, rectText, DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_LEFT);
		dc.SelectObject(pOldFont);
	}
}

void LFDialog::SetBottomLeftControl(CWnd* pChildWnd)
{
	ASSERT(pChildWnd);

	p_BottomLeftControl = pChildWnd;
}

void LFDialog::SetBottomLeftControl(UINT nID)
{
	CWnd* pChildWnd = GetDlgItem(nID);
	if (pChildWnd)
		SetBottomLeftControl(pChildWnd);
}

void LFDialog::AddBottomRightControl(CWnd* pChildWnd)
{
	ASSERT(pChildWnd);

	m_BottomRightControls.AddItem(pChildWnd);
}

void LFDialog::AddBottomRightControl(UINT nID)
{
	CWnd* pChildWnd = GetDlgItem(nID);
	if (pChildWnd)
		AddBottomRightControl(pChildWnd);
}

void LFDialog::AdjustLayout()
{
}

void LFDialog::GetLayoutRect(LPRECT lpRect) const
{
	GetClientRect(lpRect);

	CWnd* pBottomWnd = GetBottomWnd();
	if (!pBottomWnd)
		return;

	CRect rectBorders(0, 0, 7, 7);
	MapDialogRect(&rectBorders);

	CRect rectButton;
	pBottomWnd->GetWindowRect(rectButton);
	ScreenToClient(rectButton);
	lpRect->bottom = rectButton.top-rectBorders.Height()-2;

	if (m_UAC)
		lpRect->top = m_UACHeight;
}

void LFDialog::Invalidate(BOOL bErase)
{
	m_BackBufferL = m_BackBufferH = 0;

	CDialog::Invalidate(bErase);
}

void LFDialog::DrawButtonForeground(CDC& dc, LPDRAWITEMSTRUCT lpDrawItemStruct, BOOL Selected)
{
	DrawWhiteButtonForeground(dc, lpDrawItemStruct, Selected, m_ShowKeyboardCues);
}

void LFDialog::DrawButton(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CRect rect(lpDrawItemStruct->rcItem);

	CDC dc;
	dc.Attach(CreateCompatibleDC(lpDrawItemStruct->hDC));
	dc.SetBkMode(TRANSPARENT);

	CBitmap MemBitmap;
	MemBitmap.Attach(CreateCompatibleBitmap(lpDrawItemStruct->hDC, rect.Width(), rect.Height()));
	CBitmap* pOldBitmap = dc.SelectObject(&MemBitmap);

	// State
	BOOL Focused = lpDrawItemStruct->itemState & ODS_FOCUS;
	BOOL Selected = lpDrawItemStruct->itemState & ODS_SELECTED;

	// Background
	FillRect(dc, rect, (HBRUSH)SendMessage(WM_CTLCOLORBTN, (WPARAM)dc.m_hDC, (LPARAM)lpDrawItemStruct->hwndItem));

	// Button
	DrawWhiteButtonBackground(dc, rect, IsCtrlThemed(), Focused, Selected, (BOOL)::SendMessage(lpDrawItemStruct->hwndItem, WM_ISHOVER, NULL, NULL), lpDrawItemStruct->itemState & ODS_DISABLED);
	DrawButtonForeground(dc, lpDrawItemStruct, Selected);

	BitBlt(lpDrawItemStruct->hDC, 0, 0, rect.Width(), rect.Height(), dc.m_hDC, 0, 0, SRCCOPY);
	dc.SelectObject(pOldBitmap);
	DeleteDC(dc.Detach());
	DeleteObject(MemBitmap.Detach());
}


BEGIN_MESSAGE_MAP(LFDialog, CDialog)
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_THEMECHANGED()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_CTLCOLOR()
	ON_WM_INITMENUPOPUP()
	ON_WM_DRAWITEM()
END_MESSAGE_MAP()

BOOL LFDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	if (m_UAC)
	{
		// Schild
		m_UACHeight = MulDiv(40, LOWORD(GetDialogBaseUnits()), 8);
		m_ShieldSize = (m_UACHeight<24) ? 16 : (m_UACHeight<32) ? 24 : (m_UACHeight<48) ? 32 : 48;
		hIconShield = (HICON)LoadImage(AfxGetResourceHandle(), (LFGetApp()->OSVersion==OS_Vista) ? MAKEINTRESOURCE(IDI_SHIELD_VISTA) : IDI_SHIELD, IMAGE_ICON, m_ShieldSize, m_ShieldSize, LR_SHARED);

		// Symbol für dieses Dialogfeld festlegen. Wird automatisch erledigt
		// wenn das Hauptfenster der Anwendung kein Dialogfeld ist
		if (LFGetApp()->OSVersion>OS_Vista)
		{
			HICON hIcon = LFGetApp()->LoadDialogIcon(32518);

			SetIcon(hIcon, FALSE);
			SetIcon(hIcon, TRUE);
		}
	}
	else
		if (GetStyle() & WS_SIZEBOX)
		{
			HICON hIcon = LFGetApp()->LoadDialogIcon(IDR_APPLICATION);

			SetIcon(hIcon, FALSE);
			SetIcon(hIcon, TRUE);
		}

	CRect rect;
	GetClientRect(rect);
	m_LastSize = CPoint(rect.Width(), rect.Height());

	AddBottomRightControl(IDOK);
	AddBottomRightControl(IDCANCEL);

	// Subclass all buttons
	CWnd* pChildWnd = GetWindow(GW_CHILD);

	while (pChildWnd)
	{
		if (IsPushbutton(pChildWnd))
		{
			CHoverButton* pButton = new CHoverButton();
			pButton->SubclassWindow(pChildWnd->GetSafeHwnd());
			pButton->ModifyStyle(BS_TYPEMASK, BS_OWNERDRAW);

			m_Buttons.AddItem(pButton);
		}

		pChildWnd = pChildWnd->GetWindow(GW_HWNDNEXT);
	}

	LFGetApp()->HideTooltip();

	return FALSE;
}

void LFDialog::OnDestroy()
{
	LFGetApp()->HideTooltip();

	for (UINT a=0; a<m_Buttons.m_ItemCount; a++)
	{
		CHoverButton* pButton = m_Buttons.m_Items[a];

		pButton->UnsubclassWindow();
		delete pButton;
	}

	if (IsWindow(m_wndDesktopDimmer))
		m_wndDesktopDimmer.SendMessage(WM_DESTROY);

	DeleteObject(hBackgroundBrush);

	CDialog::OnDestroy();
}

BOOL LFDialog::OnEraseBkgnd(CDC* pDC)
{
	CRect rect;
	GetClientRect(rect);

	if ((m_BackBufferL!=rect.Width()) || (m_BackBufferH!=rect.Height()))
	{
		m_BackBufferL = rect.Width();
		m_BackBufferH = rect.Height();

		CDC dc;
		dc.CreateCompatibleDC(pDC);
		dc.SetBkMode(TRANSPARENT);

		CBitmap MemBitmap;
		MemBitmap.CreateCompatibleBitmap(pDC, rect.Width(), rect.Height());
		CBitmap* pOldBitmap = dc.SelectObject(&MemBitmap);

		Graphics g(dc);
		OnEraseBkgnd(dc, g, rect);

		dc.SelectObject(pOldBitmap);

		DeleteObject(hBackgroundBrush);
		hBackgroundBrush = CreatePatternBrush(MemBitmap);
	}

	FillRect(*pDC, rect, hBackgroundBrush);

	return TRUE;
}

void LFDialog::OnSize(UINT nType, INT cx, INT cy)
{
	CDialog::OnSize(nType, cx, cy);

	CSize szDiff(cx-m_LastSize.x, cy-m_LastSize.y);
	m_LastSize.x = cx;
	m_LastSize.y = cy;

	INT MaxRight = cx;
	for (UINT a=0; a<m_BottomRightControls.m_ItemCount; a++)
	{
		CWnd* pWnd = m_BottomRightControls.m_Items[a];

		CRect rect;
		pWnd->GetWindowRect(rect);
		ScreenToClient(rect);

		pWnd->SetWindowPos(NULL, rect.left+szDiff.cx, rect.top+szDiff.cy, rect.Width(), rect.Height(), SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);

		MaxRight = min(MaxRight, rect.left+szDiff.cx);
	}

	if (p_BottomLeftControl)
	{
		CRect rect;
		p_BottomLeftControl->GetWindowRect(rect);
		ScreenToClient(rect);

		p_BottomLeftControl->SetWindowPos(NULL, rect.left, rect.top+szDiff.cy, MaxRight-rect.left, rect.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
	}

	AdjustLayout();
	Invalidate();
}

void LFDialog::OnSysColorChange()
{
	if (!IsCtrlThemed())
		m_BackBufferL = m_BackBufferH = 0;
}

LRESULT LFDialog::OnThemeChanged()
{
	m_BackBufferL = m_BackBufferH = 0;

	return NULL;
}

HBRUSH LFDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	// Call base class version at first, else it will override changes
	HBRUSH hBrush = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	if (hBackgroundBrush)
		if ((nCtlColor==CTLCOLOR_BTN) || (nCtlColor==CTLCOLOR_STATIC))
		{
			CRect rect;
			pWnd->GetWindowRect(rect);
			ScreenToClient(rect);

			pDC->SetBkMode(TRANSPARENT);	// For common controls
			pDC->SetBrushOrg(-rect.left, -rect.top);

			hBrush = hBackgroundBrush;
		}

	return hBrush;
}

void LFDialog::OnInitMenuPopup(CMenu* pPopupMenu, UINT /*nIndex*/, BOOL /*bSysMenu*/)
{
	ASSERT(pPopupMenu);

	CCmdUI State;
	State.m_pMenu = State.m_pParentMenu = pPopupMenu;
	State.m_nIndexMax = pPopupMenu->GetMenuItemCount();

	ASSERT(!State.m_pOther);
	ASSERT(State.m_pMenu);

	for (State.m_nIndex=0; State.m_nIndex<State.m_nIndexMax; State.m_nIndex++)
	{
		State.m_nID = pPopupMenu->GetMenuItemID(State.m_nIndex);
		if ((State.m_nID) && (State.m_nID!=(UINT)-1))
			State.DoUpdate(this, FALSE);
	}
}

void LFDialog::OnDrawItem(INT /*nIDCtl*/, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	switch (lpDrawItemStruct->CtlType)
	{
	case ODT_BUTTON:
		if ((GetWindowLong(lpDrawItemStruct->hwndItem, GWL_STYLE) & BS_TYPEMASK)==BS_OWNERDRAW)
			DrawButton(lpDrawItemStruct);

		break;
	}
}
