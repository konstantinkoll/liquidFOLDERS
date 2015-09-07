
// LFSaveFilterDlg.cpp: Implementierung der Klasse LFSaveFilterDlg
//

#include "stdafx.h"
#include "LFCommDlg.h"
#include "LFSaveFilterDlg.h"


// LFSaveFilterDlg
//

LFSaveFilterDlg::LFSaveFilterDlg(CWnd* pParentWnd, CHAR* StoreID, BOOL AllowChooseStore, WCHAR* FileName, WCHAR* Comments)
	: LFDialog(IDD_SAVEFILTER, pParentWnd)
{
	strcpy_s(m_StoreID, LFKeySize, StoreID ? StoreID : "");
	wcscpy_s(m_FileName, 256, FileName ? FileName : L"");
	wcscpy_s(m_Comments, 256, Comments ? Comments : L"");
	m_AllowChooseStore = AllowChooseStore;
}

void LFSaveFilterDlg::DoDataExchange(CDataExchange* pDX)
{
	LFDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_DESTINATION, m_wndStorePanel);
	DDX_Control(pDX, IDC_FILENAME, m_wndEdit);

	if (pDX->m_bSaveAndValidate)
	{
		m_wndEdit.GetWindowText(m_FileName, 256);
		GetDlgItem(IDC_COMMENTS)->GetWindowText(m_Comments, 256);
	}
}


BEGIN_MESSAGE_MAP(LFSaveFilterDlg, LFDialog)
	ON_BN_CLICKED(IDC_CHOOSESTORE, OnChooseStore)
	ON_EN_CHANGE(IDC_FILENAME, OnChange)
	ON_REGISTERED_MESSAGE(LFGetApp()->p_MessageIDs->StoresChanged, OnStoresChanged)
	ON_REGISTERED_MESSAGE(LFGetApp()->p_MessageIDs->StoreAttributesChanged, OnStoresChanged)
	ON_REGISTERED_MESSAGE(LFGetApp()->p_MessageIDs->DefaultStoreChanged, OnStoresChanged)
END_MESSAGE_MAP()

BOOL LFSaveFilterDlg::OnInitDialog()
{
	LFDialog::OnInitDialog();

	// Store
	GetDlgItem(IDC_CHOOSESTORE)->EnableWindow(m_AllowChooseStore);
	OnStoresChanged(NULL, NULL);

	// Name
	m_wndEdit.SetWindowText(m_FileName);
	m_wndEdit.SetSel(0, -1);

	// OK-Button
	OnChange();

	return FALSE;
}

void LFSaveFilterDlg::OnChooseStore()
{
	LFChooseStoreDlg dlg(this);
	if (dlg.DoModal()==IDOK)
	{
		strcpy_s(m_StoreID, LFKeySize, dlg.m_StoreID);
		OnStoresChanged(NULL, NULL);
	}
}

void LFSaveFilterDlg::OnChange()
{
	CString tmpStr;
	m_wndEdit.GetWindowText(tmpStr);
	tmpStr.Trim();

	GetDlgItem(IDOK)->EnableWindow(m_wndStorePanel.IsValidStore() && !tmpStr.IsEmpty());
}

LRESULT LFSaveFilterDlg::OnStoresChanged(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	m_wndStorePanel.SetStore(m_StoreID);
	OnChange();

	return NULL;
}
