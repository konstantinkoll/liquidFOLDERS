
// LFDialog.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "LFCommDlg.h"
#include "LFCore.h"
#include "Resource.h"

using namespace Gdiplus;


// LFDialog
//

extern AFX_EXTENSION_MODULE LFCommDlgDLL;

LFDialog::LFDialog(UINT nIDTemplate, UINT _Design, CWnd* pParent)
	: CDialog(nIDTemplate, pParent)
{
	m_nIDTemplate = nIDTemplate;
	m_Design = _Design;
	p_App = (LFApplication*)AfxGetApp();
	hIconS = hIconL = hIconShield = NULL;
	hBackgroundBrush = NULL;
	backdrop = logo = NULL;
	BackBufferL = BackBufferH = UACHeight = 0;
}

void LFDialog::OnEraseBkgnd(CDC& dc, Graphics& g, CRect& rect)
{
	CWnd* pOkWnd = GetDlgItem(IDOK);
	if (!pOkWnd)
	{
		rect.SetRectEmpty();
		return;
	}

	CRect borders(0, 0, 7, 7);
	MapDialogRect(&borders);

	CRect btn;
	pOkWnd->GetWindowRect(&btn);
	ScreenToClient(&btn);

	CRect layout;
	GetLayoutRect(layout);
	int Line = layout.bottom;

	BOOL Themed = IsCtrlThemed();
	switch (m_Design)
	{
	case LFDS_Blue:
		{
			if (Themed)
			{
				int l = backdrop->m_pBitmap->GetWidth();
				int h = backdrop->m_pBitmap->GetHeight();
				if ((l<rect.Width()) || (h<rect.Height()))
				{
					double f = max((double)rect.Width()/l, (double)rect.Height()/h);
					l = (int)(l*f);
					h = (int)(h*f);
				}
				g.DrawImage(backdrop->m_pBitmap, rect.Width()-l, rect.Height()-h, l, h);

				SolidBrush brush1(Color(180, 255, 255, 255));
				g.FillRectangle(&brush1, 0, 0, BackBufferL, Line);
				brush1.SetColor(Color(224, 205, 250, 255));
				g.FillRectangle(&brush1, 0, Line++, BackBufferL, 1);
				brush1.SetColor(Color(180, 183, 210, 240));
				g.FillRectangle(&brush1, 0, Line++, BackBufferL, 1);
				brush1.SetColor(Color(224, 247, 250, 254));
				g.FillRectangle(&brush1, 0, Line, BackBufferL, 1);
				LinearGradientBrush brush2(Point(0, Line++), Point(0, rect.Height()), Color(128, 255, 255, 255), Color(64, 128, 192, 255));
				g.FillRectangle(&brush2, 0, Line, BackBufferL, rect.Height()-Line);
			}
			else
			{
				dc.FillSolidRect(rect, GetSysColor(COLOR_3DFACE));
			}

			int l = logo->m_pBitmap->GetWidth();
			int h = logo->m_pBitmap->GetHeight();
			g.DrawImage(logo->m_pBitmap, rect.Width()-l-8, 8, l, h);

			break;
		}
	case LFDS_White:
	case LFDS_UAC:
		{
			dc.FillSolidRect(0, 0, BackBufferL, Line, 0xFFFFFF);
			if (Themed)
			{
				dc.FillSolidRect(0, Line++, BackBufferL, 1, 0xDFDFDF);
				dc.FillSolidRect(0, Line, BackBufferL, rect.Height()-Line, 0xF0F0F0);
				dc.FillSolidRect(0, btn.bottom+borders.Height()+1, BackBufferL, 1, 0xDFDFDF);
				dc.FillSolidRect(0, btn.bottom+borders.Height()+2, BackBufferL, 1, 0xFFFFFF);
			}
			else
			{
				dc.FillSolidRect(0, Line++, BackBufferL, rect.Height()-Line, GetSysColor(COLOR_3DFACE));
			}

			if (m_Design!=LFDS_UAC)
				break;

			if (Themed)
			{
				LinearGradientBrush brush2(Point(0, 0), Point(BackBufferL, 0), Color(4, 80, 130), Color(28, 120, 133));
				g.FillRectangle(&brush2, 0, 0, BackBufferL, UACHeight);
				dc.SetTextColor(0xFFFFFF);
			}
			else
			{
				dc.FillSolidRect(0, 0, BackBufferL, UACHeight, GetSysColor(COLOR_HIGHLIGHT));
				dc.SetTextColor(GetSysColor(COLOR_HIGHLIGHTTEXT));
			}

			DrawIconEx(dc.m_hDC, borders.right, (UACHeight-ShieldSize)/2, hIconShield, ShieldSize, ShieldSize, 0, NULL, DI_NORMAL);

			CRect rectText(rect);
			rectText.left = borders.right+ShieldSize;
			rectText.bottom = UACHeight;

			CString tmpStr;
			ENSURE(tmpStr.LoadString(IDS_UACMESSAGE));

			CFont* pOldFont = dc.SelectObject(&((LFApplication*)AfxGetApp())->m_CaptionFont);
			dc.DrawText(tmpStr, -1, rectText, DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_LEFT);
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

void LFDialog::AddBottomControl(CWnd* pChildWnd)
{
	ASSERT(pChildWnd);

	BottomControls.push_back(pChildWnd);
}

void LFDialog::AddBottomControl(UINT nID)
{
	CWnd* pChildWnd = GetDlgItem(nID);
	if (pChildWnd)
		AddBottomControl(pChildWnd);
}

void LFDialog::AdjustLayout()
{
}

void LFDialog::GetLayoutRect(LPRECT lpRect) const
{
	GetClientRect(lpRect);

	CRect borders(0, 0, 7, 7);
	MapDialogRect(&borders);

	CRect btn;
	GetDlgItem(IDOK)->GetWindowRect(&btn);
	ScreenToClient(&btn);
	lpRect->bottom = btn.top-borders.Height()-(m_Design==LFDS_Blue ? 3 : 1);

	if (m_Design==LFDS_UAC)
		lpRect->top = UACHeight;
}


BEGIN_MESSAGE_MAP(LFDialog, CDialog)
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_THEMECHANGED()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_CTLCOLOR()
	ON_WM_DESTROY()
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
		backdrop = new CGdiPlusBitmapResource();
		backdrop->Load(IDB_BACKDROP, _T("PNG"), LFCommDlgDLL.hResource);

		// Logo laden
		logo = new CGdiPlusBitmapResource();
		logo->Load(IDB_LOGO, _T("PNG"), LFCommDlgDLL.hResource);

		break;
	case LFDS_UAC:
		// Schild
		UACHeight = MulDiv(40, LOWORD(GetDialogBaseUnits()), 8);
		ShieldSize = (UACHeight<24) ? 16 : (UACHeight<32) ? 24 : (UACHeight<48) ? 32 : 48;
		hIconShield = (HICON)LoadImage(LFCommDlgDLL.hResource, IDI_SHIELD, IMAGE_ICON, ShieldSize, ShieldSize, LR_DEFAULTCOLOR);

		p_App->PlayWarningSound();
		break;
	}

	CRect rect;
	GetClientRect(rect);
	LastSize = CPoint(rect.Width(), rect.Height());

	AddBottomControl(IDOK);
	AddBottomControl(IDCANCEL);

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

void LFDialog::OnDestroy()
{
	if (backdrop)
		delete backdrop;
	if (logo)
		delete logo;
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
	if ((BackBufferL!=rect.Width()) || (BackBufferH!=rect.Height()))
	{
		BackBufferL = rect.Width();
		BackBufferH = rect.Height();

		BackBuffer.DeleteObject();
		BackBuffer.CreateCompatibleBitmap(pDC, rect.Width(), rect.Height());
		pOldBitmap = dc.SelectObject(&BackBuffer);

		Graphics g(dc.m_hDC);
		g.SetCompositingMode(CompositingModeSourceOver);

		OnEraseBkgnd(dc, g, rect);

		if (hBackgroundBrush)
			DeleteObject(hBackgroundBrush);
		hBackgroundBrush = CreatePatternBrush(BackBuffer);
	}
	else
	{
		pOldBitmap = dc.SelectObject(&BackBuffer);
	}

	pDC->BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldBitmap);

	return TRUE;
}

void LFDialog::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	CPoint diff(cx-LastSize.x, cy-LastSize.y);
	LastSize.x = cx;
	LastSize.y = cy;

	std::list<CWnd*>::iterator ppWnd = BottomControls.begin();
	while (ppWnd!=BottomControls.end())
	{
		CRect rect;
		(*ppWnd)->GetWindowRect(rect);
		ScreenToClient(&rect);

		(*ppWnd)->SetWindowPos(NULL, rect.left+diff.x, rect.top+diff.y, rect.Width(), rect.Height(), SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
		ppWnd++;
	}

	AdjustLayout();

	BackBufferL = BackBufferH = 0;
	Invalidate();
}

LRESULT LFDialog::OnThemeChanged()
{
	BackBufferL = BackBufferH = 0;
	return TRUE;
}

void LFDialog::OnSysColorChange()
{
	if (!IsCtrlThemed())
		BackBufferL = BackBufferH = 0;
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

void LFDialog::OnEnterLicenseKey()
{
	if (!(GetExStyle() & WS_EX_APPWINDOW))
		ShowWindow(SW_HIDE);

	LFLicenseDlg dlg(this);
	dlg.DoModal();

	CheckLicenseKey();
	if (!(GetExStyle() & WS_EX_APPWINDOW))
		ShowWindow(SW_SHOW);
}
