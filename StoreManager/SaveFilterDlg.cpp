
// SaveFilterDlg.cpp: Implementierung der Klasse SaveFilterDlg
//

#include "StdAfx.h"
#include "SaveFilterDlg.h"
#include "Resource.h"


// SaveFilterDlg
//

SaveFilterDlg::SaveFilterDlg(CWnd* pParentWnd, CHAR* StoreID, BOOL AllowChooseStore, WCHAR* FileName, WCHAR* Comments)
	: CDialog(IDD_SAVEFILTER, pParentWnd)
{
	strcpy_s(m_StoreID, LFKeySize, StoreID ? StoreID : "");
	wcscpy_s(m_FileName, 256, FileName ? FileName : L"");
	wcscpy_s(m_Comments, 256, Comments ? Comments : L"");
	m_AllowChooseStore = AllowChooseStore;
}

void SaveFilterDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_DESTINATION, m_wndStorePanel);
	DDX_Control(pDX, IDC_FILENAME, m_wndEdit);

	if (pDX->m_bSaveAndValidate)
	{
		m_wndEdit.GetWindowText(m_FileName, 256);
		GetDlgItem(IDC_COMMENTS)->GetWindowText(m_Comments, 256);
	}
}

void SaveFilterDlg::CheckOK()
{
	CString tmpStr;
	m_wndEdit.GetWindowText(tmpStr);
	tmpStr.Trim();

	GetDlgItem(IDOK)->EnableWindow(m_wndStorePanel.IsValidStore() && !tmpStr.IsEmpty());
}


BEGIN_MESSAGE_MAP(SaveFilterDlg, CDialog)
	ON_BN_CLICKED(IDC_CHOOSESTORE, OnChooseStore)
	ON_EN_CHANGE(IDC_FILENAME, OnChange)
	ON_REGISTERED_MESSAGE(((LFApplication*)AfxGetApp())->p_MessageIDs->StoresChanged, OnStoresChanged)
	ON_REGISTERED_MESSAGE(((LFApplication*)AfxGetApp())->p_MessageIDs->StoreAttributesChanged, OnStoresChanged)
	ON_REGISTERED_MESSAGE(((LFApplication*)AfxGetApp())->p_MessageIDs->DefaultStoreChanged, OnStoresChanged)
END_MESSAGE_MAP()

BOOL SaveFilterDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Symbol für dieses Dialogfeld festlegen. Wird automatisch erledigt
	// wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	HICON hIcon = ((LFApplication*)AfxGetApp())->LoadIcon(MAKEINTRESOURCE(IDD_SAVEFILTER));
	SetIcon(hIcon, FALSE);
	SetIcon(hIcon, TRUE);

	// Store
	GetDlgItem(IDC_CHOOSESTORE)->EnableWindow(m_AllowChooseStore);
	OnStoresChanged(NULL, NULL);

	// Name
	m_wndEdit.SetWindowText(m_FileName);
	m_wndEdit.SetSel(0, wcslen(m_FileName)-1);

	// OK-Button
	CheckOK();

	return TRUE;
}

void SaveFilterDlg::OnChooseStore()
{
	LFChooseStoreDlg dlg(this, LFCSD_Mounted);
	if (dlg.DoModal()==IDOK)
	{
		strcpy_s(m_StoreID, LFKeySize, dlg.m_StoreID);
		OnStoresChanged(NULL, NULL);
	}
}

void SaveFilterDlg::OnChange()
{
	CheckOK();
}

LRESULT SaveFilterDlg::OnStoresChanged(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	m_wndStorePanel.SetStore(m_StoreID);
	CheckOK();

	return NULL;
}
