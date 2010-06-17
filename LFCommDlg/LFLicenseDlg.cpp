#include "stdafx.h"
#include "LFLicenseDlg.h"
#include "Resource.h"
#include "..\\LFCore\\resource.h"

using namespace Gdiplus;

extern AFX_EXTENSION_MODULE LFCommDlgDLL;

LFLicenseDlg::LFLicenseDlg(CWnd* pParent)
	: LFDialog(IDD_ENTERLICENSEKEY, pParent)
{
	icon = NULL;
}


BEGIN_MESSAGE_MAP(LFLicenseDlg, LFDialog)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

BOOL LFLicenseDlg::OnInitDialog()
{
	LFDialog::OnInitDialog();

	// Icon laden
	icon = new CGdiPlusBitmapResource();
	icon->Load(IDB_KEY, _T("PNG"), LFCommDlgDLL.hResource);

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

void LFLicenseDlg::OnDestroy()
{
	if (icon)
		delete icon;

	LFDialog::OnDestroy();
}

void LFLicenseDlg::OnEraseBkgnd(CDC& dc, Graphics& g, CRect& rect)
{
	LFDialog::OnEraseBkgnd(dc, g, rect);

	if (icon)
	{
		int l = icon->m_pBitmap->GetWidth();
		int h = icon->m_pBitmap->GetHeight();
		g.DrawImage(icon->m_pBitmap, 16, 16, l, h);
	}
}

void LFLicenseDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDOK, m_OkButton);
	DDX_Control(pDX, IDCANCEL, m_CancelButton);
	DDX_Control(pDX, IDC_LOADLICENSE, m_LoadButton);
}
