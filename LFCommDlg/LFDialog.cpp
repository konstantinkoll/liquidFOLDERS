
// LFDialog.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "LFCommDlg.h"
#include "LFCore.h"
#include "Resource.h"


// LFDialog
//

extern AFX_EXTENSION_MODULE LFCommDlgDLL;

LFDialog::LFDialog(UINT nIDTemplate, UINT Design, CWnd* pParent)
	: CDialog(nIDTemplate, pParent)
{
	m_nIDTemplate = nIDTemplate;
	m_Design = Design;
	p_App = (LFApplication*)AfxGetApp();
	hIconS = hIconL = hIconShield = NULL;
	hBackgroundBrush = NULL;
	m_pBackdrop = m_pLogo = NULL;
	m_BackBufferL = m_BackBufferH = m_UACHeight = 0;
	p_BottomLeftControl = NULL;
}

void LFDialog::DoDataExchange(CDataExchange* pDX)
{
	CWnd* pWnd = GetDlgItem(IDC_GROUPBOX1);
	if (pWnd)
		DDX_Control(pDX, IDC_GROUPBOX1, m_GroupBox1);

	pWnd = GetDlgItem(IDC_GROUPBOX2);
	if (pWnd)
		DDX_Control(pDX, IDC_GROUPBOX2, m_GroupBox2);
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
	switch (m_Design)
	{
	case LFDS_Blue:
		{
			if (Themed)
			{
				INT l = m_pBackdrop->m_pBitmap->GetWidth();
				INT h = m_pBackdrop->m_pBitmap->GetHeight();
				if ((l<rect.Width()) || (h<rect.Height()))
				{
					DOUBLE f = max((DOUBLE)rect.Width()/l, (DOUBLE)rect.Height()/h);
					l = (INT)(l*f);
					h = (INT)(h*f);
				}
				g.DrawImage(m_pBackdrop->m_pBitmap, rect.Width()-l, rect.Height()-h, l, h);

				SolidBrush brush1(Color(180, 255, 255, 255));
				g.FillRectangle(&brush1, 0, 0, m_BackBufferL, Line);
				brush1.SetColor(Color(224, 205, 250, 255));
				g.FillRectangle(&brush1, 0, Line++, m_BackBufferL, 1);
				brush1.SetColor(Color(180, 183, 210, 240));
				g.FillRectangle(&brush1, 0, Line++, m_BackBufferL, 1);
				brush1.SetColor(Color(224, 247, 250, 254));
				g.FillRectangle(&brush1, 0, Line, m_BackBufferL, 1);
				LinearGradientBrush brush2(Point(0, Line++), Point(0, rect.Height()), Color(120, 255, 255, 255), Color(32, 128, 192, 255));
				g.FillRectangle(&brush2, 0, Line, m_BackBufferL, rect.Height()-Line);
			}
			else
			{
				dc.FillSolidRect(rect, GetSysColor(COLOR_3DFACE));
			}

			INT l = m_pLogo->m_pBitmap->GetWidth();
			INT h = m_pLogo->m_pBitmap->GetHeight();
			g.DrawImage(m_pLogo->m_pBitmap, rect.Width()-l-8, 8, l, h);

			break;
		}
	case LFDS_White:
	case LFDS_UAC:
		{
			dc.FillSolidRect(0, 0, m_BackBufferL, Line, 0xFFFFFF);
			if (Themed)
			{
				dc.FillSolidRect(0, Line++, m_BackBufferL, 1, 0xDFDFDF);
				dc.FillSolidRect(0, Line, m_BackBufferL, rect.Height()-Line, 0xF0F0F0);
				dc.FillSolidRect(0, btn.bottom+borders.Height()+1, m_BackBufferL, 1, 0xDFDFDF);
				dc.FillSolidRect(0, btn.bottom+borders.Height()+2, m_BackBufferL, 1, 0xFFFFFF);
			}
			else
			{
				dc.FillSolidRect(0, Line++, m_BackBufferL, rect.Height()-Line, GetSysColor(COLOR_3DFACE));
			}

			if (m_Design!=LFDS_UAC)
				break;

			if (Themed)
			{
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

			CFont* pOldFont = dc.SelectObject(&LFGetApp()->m_CaptionFont);
			dc.DrawText(tmpStr, rectText, DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_LEFT);
			dc.SelectObject(pOldFont);

			break;
		}
	}
}

void LFDialog::CheckLicenseKey(LFLicense* License)
{
	// Ggf. "Lizenzschlüssel eingeben" verschwinden lassen
	if (LFIsLicensed(License))
	{
		CWnd* btn = GetDlgItem(IDC_ENTERLICENSEKEY);

		if (btn)
			btn->ShowWindow(SW_HIDE);
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

	CRect borders(0, 0, 7, 7);
	MapDialogRect(&borders);

	CWnd* pBottomWnd = GetBottomWnd();
	if (!pBottomWnd)
		return;

	CRect btn;
	pBottomWnd->GetWindowRect(&btn);
	ScreenToClient(&btn);
	lpRect->bottom = btn.top-borders.Height()-(m_Design==LFDS_Blue ? 3 : 1);

	if (m_Design==LFDS_UAC)
		lpRect->top = m_UACHeight;
}

UINT LFDialog::GetDesign() const
{
	return m_Design;
}


BEGIN_MESSAGE_MAP(LFDialog, CDialog)
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_THEMECHANGED()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_CTLCOLOR()
	ON_WM_INITMENUPOPUP()
	ON_BN_CLICKED(IDC_ENTERLICENSEKEY, OnEnterLicenseKey)
END_MESSAGE_MAP()

BOOL LFDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	if (m_Design!=LFDS_UAC)
	{
		// Symbol für dieses Dialogfeld festlegen. Wird automatisch erledigt
		// wenn das Hauptfenster der Anwendung kein Dialogfeld ist
		hIconS = (HICON)LoadImage(LFCommDlgDLL.hResource, MAKEINTRESOURCE(m_nIDTemplate), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
		SetIcon(hIconS, FALSE);
		hIconL = (HICON)LoadImage(LFCommDlgDLL.hResource, MAKEINTRESOURCE(m_nIDTemplate), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR);
		SetIcon(hIconL, TRUE);
	}

	switch (m_Design)
	{
	case LFDS_Blue:
		// Hintergrundbild laden
		m_pBackdrop = new CGdiPlusBitmapResource();
		ENSURE(m_pBackdrop->Load(IDB_BACKDROP, _T("PNG"), LFCommDlgDLL.hResource));

		// m_pLogo laden
		m_pLogo = new CGdiPlusBitmapResource();
		ENSURE(m_pLogo->Load(IDB_LOGO, _T("PNG"), LFCommDlgDLL.hResource));

		break;
	case LFDS_UAC:
		// Schild
		m_UACHeight = MulDiv(40, LOWORD(GetDialogBaseUnits()), 8);
		m_ShieldSize = (m_UACHeight<24) ? 16 : (m_UACHeight<32) ? 24 : (m_UACHeight<48) ? 32 : 48;
		hIconShield = (HICON)LoadImage(LFCommDlgDLL.hResource, (p_App->OSVersion==OS_Vista) ? MAKEINTRESOURCE(IDI_SHIELD_VISTA) : IDI_SHIELD, IMAGE_ICON, m_ShieldSize, m_ShieldSize, LR_DEFAULTCOLOR);

		p_App->PlayWarningSound();
		break;
	}

	CRect rect;
	GetClientRect(rect);
	m_LastSize = CPoint(rect.Width(), rect.Height());

	AddBottomRightControl(IDOK);
	AddBottomRightControl(IDCANCEL);

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

void LFDialog::OnDestroy()
{
	if (m_pBackdrop)
		delete m_pBackdrop;
	if (m_pLogo)
		delete m_pLogo;
	if (hIconL)
		DestroyIcon(hIconL);
	if (hIconS)
		DestroyIcon(hIconS);
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

		pWnd->SetWindowPos(NULL, rect.left+diff.x, rect.top+diff.y, rect.Width(), rect.Height(), SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);

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
	state.m_pMenu = pPopupMenu;
	ASSERT(state.m_pOther==NULL);
	ASSERT(state.m_pParentMenu==NULL);

	HMENU hParentMenu;
	if (AfxGetThreadState()->m_hTrackingMenu==pPopupMenu->m_hMenu)
	{
		state.m_pParentMenu = pPopupMenu;
	}
	else
	{
		hParentMenu = ::GetMenu(m_hWnd);
		if (hParentMenu)
		{
			INT nIndexMax = GetMenuItemCount(hParentMenu);
			for (INT nIndex=0; nIndex<nIndexMax; nIndex++)
				if (GetSubMenu(hParentMenu, nIndex)==pPopupMenu->m_hMenu)
				{
					state.m_pParentMenu = CMenu::FromHandle(hParentMenu);
					break;
				}
		}
	}

	state.m_nIndexMax = pPopupMenu->GetMenuItemCount();
	for (state.m_nIndex=0; state.m_nIndex<state.m_nIndexMax; state.m_nIndex++)
	{
		state.m_nID = pPopupMenu->GetMenuItemID(state.m_nIndex);
		if (!state.m_nID)
			continue;

		ASSERT(!state.m_pOther);
		ASSERT(state.m_pMenu);
		if (state.m_nID ==(UINT)-1)
		{
			state.m_pSubMenu = pPopupMenu->GetSubMenu(state.m_nIndex);
			if ((!state.m_pSubMenu) || ((state.m_nID=state.m_pSubMenu->GetMenuItemID(0))== 0) || (state.m_nID==(UINT)-1))
				continue;

			state.DoUpdate(this, TRUE);
		}
		else
		{
			state.m_pSubMenu = NULL;
			state.DoUpdate(this, FALSE);
		}

		UINT nCount = pPopupMenu->GetMenuItemCount();
		if (nCount<state.m_nIndexMax)
		{
			state.m_nIndex -= (state.m_nIndexMax-nCount);
			while ((state.m_nIndex<nCount) && (pPopupMenu->GetMenuItemID(state.m_nIndex)==state.m_nID))
				state.m_nIndex++;
		}
		state.m_nIndexMax = nCount;
	}
}

void LFDialog::OnEnterLicenseKey()
{
	LFLicenseDlg dlg(this);
	dlg.DoModal();

	CheckLicenseKey();
}
