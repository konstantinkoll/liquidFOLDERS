#include "StdAfx.h"
#include "LFCore.h"
#include "LFStoreMaintenanceDlg.h"
#include "Resource.h"


// LFStoreMaintenanceDlg
//

extern AFX_EXTENSION_MODULE LFCommDlgDLL;

LFStoreMaintenanceDlg::LFStoreMaintenanceDlg(LFMaintenanceDlgParameters* pParameters, CWnd* pParentWnd)
	: CDialog(IDD_STOREMAINTENANCE, pParentWnd)
{
	parameters = pParameters;
}


BEGIN_MESSAGE_MAP(LFStoreMaintenanceDlg, CDialog)
END_MESSAGE_MAP()

BOOL LFStoreMaintenanceDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Symbol für dieses Dialogfeld festlegen. Wird automatisch erledigt
	// wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	HICON hIcon = LoadIcon(LFCommDlgDLL.hResource, MAKEINTRESOURCE(IDD_STOREMAINTENANCE));
	SetIcon(hIcon, FALSE);
	SetIcon(hIcon, TRUE);

	SetNumber(IDC_SERVICED, parameters->Repaired);
	SetNumber(IDC_WRITEPROTECTED, parameters->NoAccess);
	SetNumber(IDC_NOFREESPACE, parameters->NoFreeSpace);
	SetNumber(IDC_ERROR, parameters->RepairError);

	return TRUE;
}

void LFStoreMaintenanceDlg::SetNumber(UINT ID, UINT Number)
{
	CString tmpStr;

	if (Number)
	{
		CString mask;
		ENSURE(mask.LoadString(Number==1 ? IDS_STORES_SINGULAR : IDS_STORES_PLURAL));
		tmpStr.Format(mask, Number);
	}
	else
	{
		tmpStr = "\u2014";
	}

	GetDlgItem(ID)->SetWindowText(tmpStr);
}
