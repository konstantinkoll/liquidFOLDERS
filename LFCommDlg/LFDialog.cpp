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

LFDialog::LFDialog(UINT nIDTemplate, CWnd* pParent)
	: CDialog(nIDTemplate, pParent)
{
	m_nIDTemplate = nIDTemplate;
	backdrop = logo = NULL;
	m_hIconL = m_hIconS = NULL;
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

	// Symbol für dieses Dialogfeld festlegen. Wird automatisch erledigt
	// wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	m_hIconS = (HICON)LoadImage(LFCommDlgDLL.hResource, MAKEINTRESOURCE(m_nIDTemplate), IMAGE_ICON, 16, 16, LR_LOADTRANSPARENT);
	SetIcon(m_hIconS, FALSE);
	m_hIconL = (HICON)LoadImage(LFCommDlgDLL.hResource, MAKEINTRESOURCE(m_nIDTemplate), IMAGE_ICON, 32, 32, LR_LOADTRANSPARENT);
	SetIcon(m_hIconL, TRUE);

	// Hintergrundbild laden
	backdrop = new CGdiPlusBitmapResource();
	backdrop->Load(IDB_BACKDROP, _T("PNG"), LFCommDlgDLL.hResource);

	// Logo laden
	logo = new CGdiPlusBitmapResource();
	logo->Load(IDB_LOGO, _T("PNG"), LFCommDlgDLL.hResource);

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

BOOL LFDialog::OnEraseBkgnd(CDC* pDC)
{
	CRect rect;
	GetClientRect(rect);

	CDC dc;
	dc.CreateCompatibleDC(pDC);

	CBitmap* pOldBitmap;
	if ((BackBufferL!=rect.Width()) || (BackBufferH!=rect.Height()))
	{
		BackBufferL = rect.Width();
		BackBufferH = rect.Height();

		BackBuffer.CreateCompatibleBitmap(pDC, rect.Width(), rect.Height());
		pOldBitmap = dc.SelectObject(&BackBuffer);

		Graphics g(dc.m_hDC);
		g.SetCompositingMode(CompositingModeSourceOver);

		OnEraseBkgnd(dc, g, rect);
	}
	else
	{
		pOldBitmap = dc.SelectObject(&BackBuffer);
	}

	pDC->BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldBitmap);

	return TRUE;
}

void LFDialog::OnEraseBkgnd(CDC& /*dc*/, Graphics& g, CRect& rect)
{
	int l = backdrop->m_pBitmap->GetWidth();
	int h = backdrop->m_pBitmap->GetHeight();
	g.DrawImage(backdrop->m_pBitmap, rect.Width()-l, rect.Height()-h, l, h);

	CRect btn;
	GetDlgItem(IDOK)->GetWindowRect(&btn);
	ScreenToClient(&btn);

	int Line = btn.top-(rect.Height()-btn.bottom)-3;

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
}

HBRUSH LFDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	// Call base class version at first, else it will override changes
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	if ((nCtlColor==CTLCOLOR_BTN) || (nCtlColor==CTLCOLOR_STATIC))
	{
		pDC->SetBkMode(TRANSPARENT);
		hbr = (HBRUSH)GetStockObject(HOLLOW_BRUSH);
	}

	return hbr;
}

void LFDialog::OnDestroy()
{
	if (backdrop)
		delete backdrop;
	if (logo)
		delete logo;
	if (m_hIconL)
		DeleteObject(m_hIconL);
	if (m_hIconS)
		DeleteObject(m_hIconS);

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

CBitmap* LFDialog::GetBackBuffer()
{
	return &BackBuffer;
}
