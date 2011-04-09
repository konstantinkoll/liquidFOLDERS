
// LFStorePropertiesDlg.cpp: Implementierung der Klasse LFStorePropertiesDlg
//

#include "StdAfx.h"
#include "LFStorePropertiesDlg.h"
#include "Resource.h"
#include "LFCore.h"
#include "LFApplication.h"


// LFStorePropertiesDlg
//

extern AFX_EXTENSION_MODULE LFCommDlgDLL;
extern LFMessageIDs* MessageIDs;

LFStorePropertiesDlg::LFStorePropertiesDlg(CHAR* StoreID, CWnd* pParentWnd)
	: CDialog(IDD_STOREPROPERTIES, pParentWnd)
{
	if (LFGetStoreSettings(StoreID, &m_Store)==LFOk)
	{
		m_Key = m_Store.guid;
	}
	else
	{
		ZeroMemory(&m_Key, sizeof(m_Key));
	}
}

void LFStorePropertiesDlg::DoDataExchange(CDataExchange* pDX)
{
	// DDX nur beim Verlassen des Dialogs
	if (pDX->m_bSaveAndValidate)
	{
		CString name;
		GetDlgItem(IDC_STORENAME)->GetWindowText(name);
		CString comment;
		GetDlgItem(IDC_COMMENT)->GetWindowText(comment);

		UINT res = LFSetStoreAttributes(m_Store.StoreID, name.GetBuffer(), comment.GetBuffer());
		if (res!=LFOk)
		{
			LFErrorBox(res);
			pDX->Fail();
		}
	}
}


BEGIN_MESSAGE_MAP(LFStorePropertiesDlg, CDialog)
	ON_REGISTERED_MESSAGE(MessageIDs->StoresChanged, OnUpdateStore)
	ON_REGISTERED_MESSAGE(MessageIDs->StoreAttributesChanged, OnUpdateStore)
END_MESSAGE_MAP()

BOOL LFStorePropertiesDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Symbol für dieses Dialogfeld festlegen. Wird automatisch erledigt
	// wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	HICON hIcon = LoadIcon(LFCommDlgDLL.hResource, MAKEINTRESOURCE(IDD_STOREPROPERTIES));
	SetIcon(hIcon, FALSE);
	SetIcon(hIcon, TRUE);

	// Store
	SendNotifyMessage(MessageIDs->StoresChanged, LFMSGF_IntStores | LFMSGF_ExtHybStores, NULL);

	// Titelleiste
	CString text;
	GetWindowText(text);
	CString caption;
	caption.Format(text, m_Store.StoreName);
	SetWindowText(caption);

	return TRUE;
}

LRESULT LFStorePropertiesDlg::OnUpdateStore(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	CEdit* edit1 = (CEdit*)GetDlgItem(IDC_STORENAME);
	CEdit* edit2 = (CEdit*)GetDlgItem(IDC_COMMENT);

	if (LFGetStoreSettings(m_Key, &m_Store)==LFOk)
	{
		if (edit1->LineLength()==0)
			edit1->SetWindowText(m_Store.StoreName);
		if (edit2->LineLength()==0)
			edit2->SetWindowText(m_Store.Comment);

		edit1->EnableWindow(TRUE);
		edit2->EnableWindow(TRUE);
		GetDlgItem(IDOK)->EnableWindow(TRUE);

		WCHAR tmpStr[256];
		LFTimeToString(m_Store.CreationTime, tmpStr, 256);
		GetDlgItem(IDC_CREATED)->SetWindowText(tmpStr);
		LFTimeToString(m_Store.FileTime, tmpStr, 256);
		GetDlgItem(IDC_UPDATED)->SetWindowText(tmpStr);
		LFTimeToString(m_Store.MaintenanceTime, tmpStr, 256);
		GetDlgItem(IDC_MAINTENANCE)->SetWindowText(tmpStr);

		OLECHAR szGUID[MAX_PATH];
		StringFromGUID2(m_Store.guid, szGUID, MAX_PATH);
		GetDlgItem(IDC_GUID)->SetWindowText(szGUID);

		GetDlgItem(IDC_LASTSEENCAPTION)->EnableWindow(m_Store.StoreMode!=LFStoreModeInternal);
		GetDlgItem(IDC_LASTSEEN)->SetWindowText(m_Store.LastSeen);
		GetDlgItem(IDC_DATPATH)->SetWindowText(m_Store.DatPath);
		GetDlgItem(IDC_IDXPATHMAIN)->SetWindowText(m_Store.IdxPathMain);
		GetDlgItem(IDC_IDXPATHAUX)->SetWindowText(m_Store.IdxPathAux);

		LFUINTToString(m_Store.IndexVersion, tmpStr, 256);
		GetDlgItem(IDC_IDXVERSION)->SetWindowText(tmpStr);
	}
	else
	{
		edit1->EnableWindow(FALSE);
		edit2->EnableWindow(FALSE);
		GetDlgItem(IDOK)->EnableWindow(FALSE);
		GetDlgItem(IDCANCEL)->SetFocus();
	}

	return NULL;
}
