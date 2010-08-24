// LFDialog.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "LFDialog.h"
#include "LFLicenseDlg.h"
#include "LFCore.h"
#include "Resource.h"

using namespace Gdiplus;


// LFDialog
//

extern AFX_EXTENSION_MODULE LFCommDlgDLL;

LFDialog::LFDialog(UINT nIDTemplate, UINT nIDStyle, CWnd* pParent)
	: CDialog(nIDTemplate, pParent)
{
	m_nIDTemplate = nIDTemplate;
	m_nIDStyle = nIDStyle;
	hIconS = hIconL = hIconShield = NULL;
	hBackgroundBrush = NULL;
	backdrop = logo = NULL;
	BackBufferL = BackBufferH = 0;
}


BEGIN_MESSAGE_MAP(LFDialog, CDialog)
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_ENTERLICENSEKEY, OnEnterLicenseKey)
END_MESSAGE_MAP()

BOOL LFDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	if (m_nIDStyle!=LFDS_UAC)
	{
		// Symbol für dieses Dialogfeld festlegen. Wird automatisch erledigt
		// wenn das Hauptfenster der Anwendung kein Dialogfeld ist
		hIconS = (HICON)LoadImage(LFCommDlgDLL.hResource, MAKEINTRESOURCE(m_nIDTemplate), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
		SetIcon(hIconS, FALSE);
		hIconL = (HICON)LoadImage(LFCommDlgDLL.hResource, MAKEINTRESOURCE(m_nIDTemplate), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR);
		SetIcon(hIconL, TRUE);
	}
	else
	{
		((LFApplication*)AfxGetApp())->PlayWarningSound();
	}

	switch (m_nIDStyle)
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

		((LFApplication*)AfxGetApp())->PlayWarningSound();
		break;
	}

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
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

void LFDialog::OnEraseBkgnd(CDC& dc, Graphics& g, CRect& rect)
{
	CRect borders(0, 0, 7, 7);
	MapDialogRect(&borders);

	CRect btn;
	GetDlgItem(IDOK)->GetWindowRect(&btn);
	ScreenToClient(&btn);
	int Line = btn.top-borders.Height()-3;

	switch (m_nIDStyle)
	{
	case LFDS_Blue:
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
			brush1.SetColor(Color(255, 205, 250, 255));
			g.FillRectangle(&brush1, 0, Line++, BackBufferL, 1);
			brush1.SetColor(Color(255, 183, 210, 240));
			g.FillRectangle(&brush1, 0, Line++, BackBufferL, 1);
			brush1.SetColor(Color(255, 247, 250, 254));
			g.FillRectangle(&brush1, 0, Line++, BackBufferL, 1);
			LinearGradientBrush brush2(Point(0, Line-1), Point(0, rect.Height()), Color(200, 255, 255, 255), Color(0, 255, 255, 255));
			g.FillRectangle(&brush2, 0, Line, BackBufferL, rect.Height()-Line);

			l = logo->m_pBitmap->GetWidth();
			h = logo->m_pBitmap->GetHeight();
			g.DrawImage(logo->m_pBitmap, rect.Width()-l-8, 8, l, h);

			break;
		}
	case LFDS_White:
	case LFDS_UAC:
		{
			dc.FillSolidRect(0, 0, BackBufferL, ++Line, 0xFFFFFF);
			dc.FillSolidRect(0, Line++, BackBufferL, 1, 0xDFDFDF);
			dc.FillSolidRect(0, Line, BackBufferL, rect.Height()-Line, 0xF0F0F0);
			dc.FillSolidRect(0, btn.bottom+borders.Height()+1, BackBufferL, 1, 0xDFDFDF);
			dc.FillSolidRect(0, btn.bottom+borders.Height()+2, BackBufferL, 1, 0xFFFFFF);

			if (m_nIDStyle!=LFDS_UAC)
				break;

			LinearGradientBrush brush2(Point(0, 0), Point(rect.Width(), 0), Color(4, 80, 130), Color(28, 120, 133));
			g.FillRectangle(&brush2, 0, 0, rect.Width(), UACHeight);

			DrawIconEx(dc.m_hDC, borders.right, (UACHeight-ShieldSize)/2, hIconShield, ShieldSize, ShieldSize, 0, NULL, DI_NORMAL);

			CRect rectText(rect);
			rectText.left = borders.right+ShieldSize;
			rectText.bottom = UACHeight;

			CString tmpStr;
			ENSURE(tmpStr.LoadString(IDS_UACMESSAGE));

			CFont* pOldFont = dc.SelectObject(&((LFApplication*)AfxGetApp())->m_CaptionFont);
			dc.SetTextColor(0xFFFFFF);
			dc.DrawText(tmpStr, -1, rectText, DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_LEFT);
			dc.SelectObject(pOldFont);

			break;
		}
	}
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
