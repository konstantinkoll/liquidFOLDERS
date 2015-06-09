
// LFDialog.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "LFCommDlg.h"
#include "LFCore.h"
#include "Resource.h"


// LFDialog
//

LFDialog::LFDialog(UINT nIDTemplate, CWnd* pParentWnd, BOOL UAC)
	: CDialog(nIDTemplate, pParentWnd)
{
	m_nIDTemplate = nIDTemplate;
	m_UAC = UAC;

	p_App = LFGetApp();
	hIconShield = NULL;
	hBackgroundBrush = NULL;
	m_BackBufferL = m_BackBufferH = m_UACHeight = 0;
	p_BottomLeftControl = NULL;
}

void LFDialog::DoDataExchange(CDataExchange* pDX)
{
	for (UINT a=0; a<4; a++)
	{
		CWnd* pWnd = GetDlgItem(IDC_CATEGORY1+a);
		if (pWnd)
			DDX_Control(pDX, IDC_CATEGORY1+a, m_wndCategory[a]);
	}

	for (UINT a=0; a<3; a++)
	{
		CWnd* pWnd = GetDlgItem(IDC_GROUPBOX1+a);
		if (pWnd)
			DDX_Control(pDX, IDC_GROUPBOX1+a, m_wndGroupBox[a]);
	}
}

CWnd* LFDialog::GetBottomWnd() const
{
	CWnd* pBottomWnd = GetDlgItem(IDOK);
	if (!pBottomWnd)
		pBottomWnd = GetDlgItem(IDCANCEL);

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

	CRect borders(0, 0, 7, 7);
	MapDialogRect(&borders);

	CRect btn;
	pBottomWnd->GetWindowRect(&btn);
	ScreenToClient(&btn);

	CRect layout;
	GetLayoutRect(layout);
	INT Line = layout.bottom;

	BOOL Themed = IsCtrlThemed();

	// Background
	dc.FillSolidRect(0, 0, m_BackBufferL, Line, 0xFFFFFF);
	if (Themed)
	{
		dc.FillSolidRect(0, Line, m_BackBufferL, rect.Height()-Line, 0xFFFFFF);

		CGdiPlusBitmap* pDivider = p_App->GetCachedResourceImage(IDB_DIVDOWN, _T("PNG"), AfxGetResourceHandle());
		g.DrawImage(pDivider->m_pBitmap, (rect.Width()-(INT)pDivider->m_pBitmap->GetWidth())/2, Line);
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
			CGdiPlusBitmap* pDivider = p_App->GetCachedResourceImage(IDB_DIV, _T("PNG"), AfxGetResourceHandle());
			g.DrawImage(pDivider->m_pBitmap, (rect.Width()-(INT)pDivider->m_pBitmap->GetWidth())/2, btn.bottom+borders.Height()+1);

			LinearGradientBrush brush2(Point(0, 0), Point(m_BackBufferL, 0), Color(4, 80, 130), Color(28, 120, 133));
			g.FillRectangle(&brush2, 0, 0, m_BackBufferL, m_UACHeight);
			dc.SetTextColor(0xFFFFFF);
		}
		else
		{
			dc.FillSolidRect(0, 0, m_BackBufferL, m_UACHeight, GetSysColor(COLOR_HIGHLIGHT));
			dc.SetTextColor(GetSysColor(COLOR_HIGHLIGHTTEXT));
		}

		DrawIconEx(dc, borders.right, (m_UACHeight-m_ShieldSize)/2, hIconShield, m_ShieldSize, m_ShieldSize, 0, NULL, DI_NORMAL);

		CRect rectText(rect);
		rectText.left = borders.right+m_ShieldSize;
		rectText.bottom = m_UACHeight;

		CString tmpStr;
		ENSURE(tmpStr.LoadString(IDS_UACMESSAGE));

		CFont* pOldFont = dc.SelectObject(&p_App->m_UACFont);
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

	m_BottomRightControls.AddTail(pChildWnd);
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

	CRect borders(0, 0, 7, 7);
	MapDialogRect(&borders);

	CRect btn;
	pBottomWnd->GetWindowRect(&btn);
	ScreenToClient(&btn);
	lpRect->bottom = btn.top-borders.Height()-2;

	if (m_UAC)
		lpRect->top = m_UACHeight;
}


BEGIN_MESSAGE_MAP(LFDialog, CDialog)
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_THEMECHANGED()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_CTLCOLOR()
	ON_WM_INITMENUPOPUP()
END_MESSAGE_MAP()

BOOL LFDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	if (m_UAC)
	{
		// Schild
		m_UACHeight = MulDiv(40, LOWORD(GetDialogBaseUnits()), 8);
		m_ShieldSize = (m_UACHeight<24) ? 16 : (m_UACHeight<32) ? 24 : (m_UACHeight<48) ? 32 : 48;
		hIconShield = (HICON)LoadImage(AfxGetResourceHandle(), (p_App->OSVersion==OS_Vista) ? MAKEINTRESOURCE(IDI_SHIELD_VISTA) : IDI_SHIELD, IMAGE_ICON, m_ShieldSize, m_ShieldSize, LR_DEFAULTCOLOR);

		p_App->PlayWarningSound();
	}
	else
	{
		// Symbol f�r dieses Dialogfeld festlegen. Wird automatisch erledigt
		// wenn das Hauptfenster der Anwendung kein Dialogfeld ist
		SetIcon((HICON)LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(m_nIDTemplate), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR), FALSE);
		SetIcon((HICON)LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(m_nIDTemplate), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR), TRUE);
	}

	CRect rect;
	GetClientRect(rect);
	m_LastSize = CPoint(rect.Width(), rect.Height());

	AddBottomRightControl(IDOK);
	AddBottomRightControl(IDCANCEL);

	return TRUE;  // TRUE zur�ckgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

void LFDialog::OnDestroy()
{
	if (hIconShield)
		DestroyIcon(hIconShield);
	if (hBackgroundBrush)
		DeleteObject(hBackgroundBrush);

	CDialog::OnDestroy();
}

BOOL LFDialog::OnEraseBkgnd(CDC* pDC)
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

		Graphics g(dc);
		g.SetCompositingMode(CompositingModeSourceOver);

		OnEraseBkgnd(dc, g, rect);

		if (hBackgroundBrush)
			DeleteObject(hBackgroundBrush);
		hBackgroundBrush = CreatePatternBrush(m_BackBuffer);
	}
	else
	{
		pOldBitmap = dc.SelectObject(&m_BackBuffer);
	}

	pDC->BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldBitmap);

	return TRUE;
}

void LFDialog::OnSize(UINT nType, INT cx, INT cy)
{
	CDialog::OnSize(nType, cx, cy);

	CPoint diff(cx-m_LastSize.x, cy-m_LastSize.y);
	m_LastSize.x = cx;
	m_LastSize.y = cy;

	INT MaxRight = cx;
	for (POSITION p=m_BottomRightControls.GetHeadPosition(); p; )
	{
		CWnd* pWnd = m_BottomRightControls.GetNext(p);

		CRect rect;
		pWnd->GetWindowRect(&rect);
		ScreenToClient(&rect);

		pWnd->SetWindowPos(NULL, rect.left+diff.x, rect.top+diff.y, rect.Width(), rect.Height(), SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOCOPYBITS);

		MaxRight = min(MaxRight, rect.left+diff.x);
	}

	if (p_BottomLeftControl)
	{
		CRect rect;
		p_BottomLeftControl->GetWindowRect(&rect);
		ScreenToClient(&rect);

		p_BottomLeftControl->SetWindowPos(NULL, rect.left, rect.top+diff.y, MaxRight-rect.left, rect.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
	}

	AdjustLayout();

	m_BackBufferL = m_BackBufferH = 0;
	Invalidate();
}

LRESULT LFDialog::OnThemeChanged()
{
	m_BackBufferL = m_BackBufferH = 0;

	return TRUE;
}

void LFDialog::OnSysColorChange()
{
	if (!IsCtrlThemed())
		m_BackBufferL = m_BackBufferH = 0;
}

HBRUSH LFDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	// Call base class version at first, else it will override changes
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

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

void LFDialog::OnInitMenuPopup(CMenu* pPopupMenu, UINT /*nIndex*/, BOOL /*bSysMenu*/)
{
	ASSERT(pPopupMenu);

	CCmdUI state;
	state.m_pMenu = state.m_pParentMenu = pPopupMenu;
	state.m_nIndexMax = pPopupMenu->GetMenuItemCount();

	ASSERT(!state.m_pOther);
	ASSERT(state.m_pMenu);

	for (state.m_nIndex=0; state.m_nIndex<state.m_nIndexMax; state.m_nIndex++)
	{
		state.m_nID = pPopupMenu->GetMenuItemID(state.m_nIndex);
		if ((state.m_nID) && (state.m_nID!=(UINT)-1))
			state.DoUpdate(this, FALSE);
	}
}
