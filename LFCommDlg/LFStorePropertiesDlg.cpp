#include "StdAfx.h"
#include "LFStorePropertiesDlg.h"
#include "Resource.h"
#include "LFCore.h"
#include "LFApplication.h"


// LFStorePropertiesDlg
//

extern AFX_EXTENSION_MODULE LFCommDlgDLL;
extern LFMessageIDs* MessageIDs;

LFStorePropertiesDlg::LFStorePropertiesDlg(CWnd* pParentWnd, char* _StoreID)
	: CDialog(IDD_STOREPROPERTIES, pParentWnd)
{
	if (LFGetStoreSettings(_StoreID, &store)==LFOk)
	{
		key = store.GUID;
	}
	else
	{
		ZeroMemory(&key, sizeof(key));
	}
}

LFStorePropertiesDlg::~LFStorePropertiesDlg()
{
}

BEGIN_MESSAGE_MAP(LFStorePropertiesDlg, CDialog)
	ON_WM_DESTROY()
	ON_REGISTERED_MESSAGE(MessageIDs->StoresChanged, UpdateStore)
	ON_REGISTERED_MESSAGE(MessageIDs->StoreAttributesChanged, UpdateStore)
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
	SendMessage(MessageIDs->StoresChanged, LFMSGF_IntStores | LFMSGF_ExtHybStores);

	// Titelleiste
	CString text;
	GetWindowText(text);
	CString caption;
	caption.Format(text, store.StoreName);
	SetWindowText(caption);

	return TRUE;
}

void LFStorePropertiesDlg::OnDestroy()
{
	CDialog::OnDestroy();
}

LRESULT LFStorePropertiesDlg::UpdateStore(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	CEdit* edit1 = (CEdit*)GetDlgItem(IDC_STORENAME);
	CEdit* edit2 = (CEdit*)GetDlgItem(IDC_COMMENT);

	if (LFGetStoreSettings(key, &store)==LFOk)
	{
		if (edit1->LineLength()==0)
			edit1->SetWindowText(store.StoreName);
		if (edit2->LineLength()==0)
			edit2->SetWindowText(store.Comment);

		edit1->EnableWindow(TRUE);
		edit2->EnableWindow(TRUE);
		GetDlgItem(IDOK)->EnableWindow(TRUE);

		wchar_t tmpStr[256];
		LFTimeToString(store.CreationTime, tmpStr, 256);
		GetDlgItem(IDC_CREATED)->SetWindowText(tmpStr);
		LFTimeToString(store.FileTime, tmpStr, 256);
		GetDlgItem(IDC_UPDATED)->SetWindowText(tmpStr);

		OLECHAR szGUID[MAX_PATH];
		StringFromGUID2(store.GUID, szGUID, MAX_PATH);
		GetDlgItem(IDC_GUID)->SetWindowText(szGUID);

		GetDlgItem(IDC_LASTSEENCAPTION)->EnableWindow(store.StoreMode!=LFStoreModeInternal);
		GetDlgItem(IDC_LASTSEEN)->SetWindowText(store.LastSeen);

		size_t sz = strlen(store.DatPath)+1;
		MultiByteToWideChar(CP_ACP, 0, store.DatPath, (int)sz, (LPWSTR)tmpStr, (int)sz);
		GetDlgItem(IDC_DATPATH)->SetWindowText(tmpStr);

		sz = strlen(store.IdxPathMain)+1;
		MultiByteToWideChar(CP_ACP, 0, store.IdxPathMain, (int)sz, (LPWSTR)tmpStr, (int)sz);
		GetDlgItem(IDC_IDXPATHMAIN)->SetWindowText(tmpStr);

		sz = strlen(store.IdxPathAux)+1;
		MultiByteToWideChar(CP_ACP, 0, store.IdxPathAux, (int)sz, (LPWSTR)tmpStr, (int)sz);
		GetDlgItem(IDC_IDXPATHAUX)->SetWindowText(tmpStr);
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

void LFStorePropertiesDlg::DoDataExchange(CDataExchange* pDX)
{
	// DDX nur beim Verlassen des Dialogs
	if (pDX->m_bSaveAndValidate)
	{
		CString name;
		GetDlgItem(IDC_STORENAME)->GetWindowText(name);
		CString comment;
		GetDlgItem(IDC_COMMENT)->GetWindowText(comment);

		UINT res = LFSetStoreAttributes(store.StoreID, name.GetBuffer(), comment.GetBuffer());
		if (res!=LFOk)
		{
			LFErrorBox(res);
			pDX->Fail();
		}
	}
}
