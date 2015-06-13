
// LFAddStoreDlg.cpp: Implementierung der Klasse LFAddStoreDlg
//

#include "stdafx.h"
#include "LFCommDlg.h"
#include "LFStoreNewDlg.h"
#include <wininet.h>


// LFAddStoreDlg
//

LFAddStoreDlg::LFAddStoreDlg(CWnd* pParentWnd)
	: LFDialog(IDD_ADDSTORE, pParentWnd)
{
}

void LFAddStoreDlg::DoDataExchange(CDataExchange* pDX)
{
	LFDialog::DoDataExchange(pDX);

	DDX_StoreButton(pDX, IDC_ADDSTORE_LIQUIDFOLDERS, m_wndStoreButtons[0], LFTypeSourceInternal);
	DDX_StoreButton(pDX, IDC_ADDSTORE_WINDOWS, m_wndStoreButtons[1], LFTypeSourceWindows);

	for (UINT a=0; a<8; a++)
		DDX_StoreButton(pDX, IDC_ADDSTORE_DROPBOX+a, m_wndStoreButtons[a+2], LFTypeSourceDropbox+a);
}

void LFAddStoreDlg::CheckInternetConnection()
{
	DWORD Flags;
	BOOL Connected = InternetGetConnectedState(&Flags, 0);

#ifndef DEBUG
	Connected = FALSE;
#endif

	m_wndCategory[1].ShowWindow(Connected ? SW_SHOW : SW_HIDE);
	for (UINT a=0; a<8; a++)
		m_wndStoreButtons[a+2].ShowWindow(Connected ? SW_SHOW : SW_HIDE);
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
	LFStoreNewDlg dlg(this);
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
