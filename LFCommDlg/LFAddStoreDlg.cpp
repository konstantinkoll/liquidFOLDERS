
// LFAddStoreDlg.cpp: Implementierung der Klasse LFAddStoreDlg
//

#include "stdafx.h"
#include "LFCommDlg.h"
#include "LFCreateStoreDlg.h"
#include <wininet.h>


// LFAddStoreDlg
//

#define BORDER     12

LFAddStoreDlg::LFAddStoreDlg(CWnd* pParentWnd)
	: LFDialog(IDD_ADDSTORE, pParentWnd)
{
	p_Icons = NULL;
}

void LFAddStoreDlg::DrawButtonForeground(CDC& dc, LPDRAWITEMSTRUCT lpDrawItemStruct, BOOL Hover, BOOL Themed)
{
	if ((lpDrawItemStruct->CtlID>=IDC_ADDSTORE_LIQUIDFOLDERS) && (lpDrawItemStruct->CtlID<=IDC_ADDSTORE_YOUTUBE))
	{
		UINT StoreType;
		switch (lpDrawItemStruct->CtlID)
		{
		case IDC_ADDSTORE_LIQUIDFOLDERS:
			StoreType = LFTypeSourceInternal;
			break;

		case IDC_ADDSTORE_WINDOWS:
			StoreType = LFTypeSourceWindows;
			break;

		default:
			StoreType = LFTypeSourceDropbox+lpDrawItemStruct->CtlID-IDC_ADDSTORE_DROPBOX;
		}

		// Content
		CRect rectText(lpDrawItemStruct->rcItem);
	rectText.DeflateRect(BORDER+1, BORDER);
	if (lpDrawItemStruct->itemState & ODS_SELECTED)
		rectText.OffsetRect(1, 1);

	// Icon
	if (p_Icons)
	{
		CPoint pt(rectText.left, rectText.top+(rectText.Height()-m_IconSize)/2);
		p_Icons->DrawEx(&dc, StoreType-1, pt, CSize(m_IconSize, m_IconSize), CLR_NONE, 0xFFFFFF, IsWindowEnabled() ? ILD_TRANSPARENT : ILD_BLEND25);

		rectText.left += m_IconSize+BORDER;
	}

	// Text
	WCHAR Caption[256];
	LFGetSourceName(Caption, 256, StoreType, FALSE);

	WCHAR Hint[256];
	::GetWindowText(lpDrawItemStruct->hwndItem, Hint, 256);

	CFont* pOldFont = dc.SelectObject(&LFGetApp()->m_LargeFont);

	INT HeightCaption = dc.GetTextExtent(Caption).cy;
	HeightCaption += HeightCaption/2;

	dc.SelectObject(&LFGetApp()->m_DefaultFont);

	CRect rectHint(rectText);
	dc.DrawText(Hint, rectHint, DT_CALCRECT | DT_WORDBREAK | DT_END_ELLIPSIS | DT_NOPREFIX);

	rectText.top += (rectText.Height()-HeightCaption-rectHint.Height())/2;

	dc.SetTextColor(IsWindowEnabled() ? Themed ? 0x000000 : GetSysColor(COLOR_WINDOWTEXT) : GetSysColor(COLOR_GRAYTEXT));
	dc.SelectObject(&LFGetApp()->m_LargeFont);
	dc.DrawText(Caption, rectText, DT_SINGLELINE | DT_END_ELLIPSIS);
	rectText.top += HeightCaption;

	dc.SetTextColor(IsWindowEnabled() ? Themed ? 0x404040 : GetSysColor(COLOR_WINDOWTEXT) : GetSysColor(COLOR_GRAYTEXT));
	dc.SelectObject(&LFGetApp()->m_DefaultFont);
	dc.DrawText(Hint, rectText, DT_WORDBREAK | DT_END_ELLIPSIS | DT_NOPREFIX);

	dc.SelectObject(pOldFont);
	}
	else
	{
		LFDialog::DrawButtonForeground(dc, lpDrawItemStruct, Hover, Themed);
	}
}

void LFAddStoreDlg::CheckInternetConnection()
{
	DWORD Flags;
	BOOL Connected = InternetGetConnectedState(&Flags, 0);

#ifndef DEBUG
	Connected = FALSE;
#endif

	m_wndCategory[1].ShowWindow(Connected ? SW_SHOW : SW_HIDE);
	//for (UINT a=0; a<8; a++)
	//	m_wndStoreButtons[a+2].ShowWindow(Connected ? SW_SHOW : SW_HIDE);
}


BEGIN_MESSAGE_MAP(LFAddStoreDlg, LFDialog)
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_REGISTERED_MESSAGE(LFGetApp()->p_MessageIDs->StoresChanged, OnUpdateStores)
	ON_BN_CLICKED(IDC_ADDSTORE_LIQUIDFOLDERS, OnBtnLiquidfolders)
	ON_BN_CLICKED(IDC_ADDSTORE_WINDOWS, OnBtnWindows)
END_MESSAGE_MAP()

BOOL LFAddStoreDlg::OnInitDialog()
{
	LFDialog::OnInitDialog();

	// Categories
	m_wndCategory[0].SetWindowText(LFGetApp()->m_ItemCategories[LFItemCategoryLocal].Caption);
	m_wndCategory[1].SetWindowText(LFGetApp()->m_ItemCategories[LFItemCategoryRemote].Caption);

	// Buttons
	CRect rectButton;
	GetDlgItem(IDC_ADDSTORE_LIQUIDFOLDERS)->GetClientRect(rectButton);

	INT Height = rectButton.Height()-2*BORDER;

	m_IconSize = (Height>=128) ? 128 : (Height>=96) ? 96 : 48;
	p_Icons = (m_IconSize==128) ? &LFGetApp()->m_CoreImageListJumbo : (m_IconSize==96) ? &LFGetApp()->m_CoreImageListHuge : &LFGetApp()->m_CoreImageListExtraLarge;

	// Internet
	CheckInternetConnection();
	SetTimer(1, 1000, NULL);

	// Status
	OnUpdateStores(NULL, NULL);

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

void LFAddStoreDlg::OnDestroy()
{
	KillTimer(1);

	LFDialog::OnDestroy();
}

void LFAddStoreDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent==1)
		CheckInternetConnection();

	LFDialog::OnTimer(nIDEvent);

	// Eat bogus WM_TIMER messages
	MSG msg;
	while (PeekMessage(&msg, m_hWnd, WM_TIMER, WM_TIMER, PM_REMOVE));
}

LRESULT LFAddStoreDlg::OnUpdateStores(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	UINT StoreCount = LFGetStoreCount();

	CString Hint;
	Hint.Format(StoreCount==1 ? IDS_STORES_SINGULAR : IDS_STORES_PLURAL, StoreCount);

	GetDlgItem(IDC_STATUS)->SetWindowText(Hint);

	return NULL;
}

void LFAddStoreDlg::OnBtnLiquidfolders()
{
	LFCreateStoreDlg dlg(this);
	dlg.DoModal();
}

void LFAddStoreDlg::OnBtnWindows()
{
	MessageBox(L"Coming soon!");
	return;

	CString Caption;
	GetWindowText(Caption);
	CString Hint;
	GetDlgItem(IDC_ADDSTORE_WINDOWS)->GetWindowText(Hint);

	LFBrowseForFolderDlg dlg(this, Caption, Hint);
	if (dlg.DoModal()==LFOk)
	{
		// TODO
		CWaitCursor csr;
		UINT Result = LFOk;

		LFErrorBox(Result, GetSafeHwnd());
	}
}
