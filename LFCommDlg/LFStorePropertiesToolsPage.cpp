
// LFStorePropertiesToolsPage.cpp: Implementierung der Klasse LFStorePropertiesToolsPage
//

#include "stdafx.h"
#include "LFCommDlg.h"
#include "LFStorePropertiesToolsPage.h"


CString MakeHex(BYTE* x, UINT bCount)
{
	CString tmpStr;
	for (UINT a=0; a<bCount; a++)
	{
		CString digit;
		digit.Format(_T("%.2x"), x[a]);
		tmpStr += digit;
		if (a<bCount-1)
			tmpStr += _T(",");
	}
	return tmpStr;
}

void CEscape(CString &s)
{
	for (INT a = s.GetLength()-1; a>=0; a--)
		if ((s[a]==L'\\') || (s[a]==L'\"'))
			s.Insert(a, L'\\');
}


// LFStorePropertiesToolsPage
//

LFStorePropertiesToolsPage::LFStorePropertiesToolsPage(LFStoreDescriptor* pStore, BOOL* pStoreValid)
	: CPropertyPage()
{
	ASSERT(pStore);
	ASSERT(pStoreValid);

	p_Store = pStore;
	p_StoreValid = pStoreValid;
}


BEGIN_MESSAGE_MAP(LFStorePropertiesToolsPage, CPropertyPage)
	ON_BN_CLICKED(IDC_RUNSYNCHRONIZE, OnRunSynchronize)
	ON_BN_CLICKED(IDC_RUNMAINTENANCE, OnRunMaintenance)
	ON_BN_CLICKED(IDC_RUNBACKUP, OnRunBackup)
	ON_REGISTERED_MESSAGE(LFGetApp()->p_MessageIDs->StoresChanged, OnUpdateStore)
	ON_REGISTERED_MESSAGE(LFGetApp()->p_MessageIDs->StoreAttributesChanged, OnUpdateStore)
END_MESSAGE_MAP()

BOOL LFStorePropertiesToolsPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	GetDlgItem(IDC_MAINTENANCE)->GetWindowText(m_MaskMaintenance);
	GetDlgItem(IDC_SYNCHRONIZED)->GetWindowText(m_MaskSynchronized);

	// Store
	SendMessage(LFGetApp()->p_MessageIDs->StoresChanged);

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

void LFStorePropertiesToolsPage::OnRunSynchronize()
{
	LFRunSynchronization(p_Store->StoreID, this);
}

void LFStorePropertiesToolsPage::OnRunMaintenance()
{
	LFRunMaintenance(this);
}

void LFStorePropertiesToolsPage::OnRunBackup()
{
	CHAR* pStoreIDs;
	UINT Count;
	UINT Result = LFGetAllStores(&pStoreIDs, &Count);
	if (Result!=LFOk)
	{
		LFErrorBox(this, Result);
		return;
	}

	CString tmpStr((LPCSTR)IDS_REGFILEFILTER);
	tmpStr += _T(" (*.reg)|*.reg||");

	CFileDialog dlg(FALSE, _T(".reg"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, tmpStr, this);

	if (dlg.DoModal()==IDOK)
	{
		CWaitCursor csr;

		CStdioFile f;
		if (!f.Open(dlg.GetPathName(), CFile::modeCreate | CFile::modeWrite))
		{
			LFErrorBox(this, LFDriveNotReady);
		}
		else
		{
			try
			{
				f.WriteString(_T("Windows Registry Editor Version 5.00\n"));

				CHAR* Ptr = pStoreIDs;
				for (UINT a=0; a<Count; a++)
				{
					LFStoreDescriptor Store;
					if (LFGetStoreSettings(Ptr, &Store)==LFOk)
						if ((Store.Mode & LFStoreModeIndexMask)!=LFStoreModeIndexExternal)
						{
							// Header
							tmpStr = _T("\n[HKEY_CURRENT_USER\\Software\\liquidFOLDERS\\Stores\\");
							tmpStr += Store.StoreID;
							f.WriteString(tmpStr+_T("]\n"));

							// Name
							tmpStr = Store.StoreName;
							CEscape(tmpStr);
							f.WriteString(_T("\"Name\"=\"")+tmpStr+_T("\"\n"));

							// Mode
							tmpStr.Format(_T("\"Mode\"=dword:%.8x\n"), Store.Mode);
							f.WriteString(tmpStr);

							// AutoLocation
							tmpStr.Format(_T("\"AutoLocation\"=dword:%.8x\n"), Store.Flags & LFStoreFlagsAutoLocation);
							f.WriteString(tmpStr);

							if ((Store.Flags & LFStoreFlagsAutoLocation)==0)
							{
								// Path
								tmpStr = Store.DatPath;
								CEscape(tmpStr);
								f.WriteString(_T("\"Path\"=\"")+tmpStr+_T("\"\n"));
							}

							// GUID
							f.WriteString(_T("\"GUID\"=hex:")+MakeHex((BYTE*)&Store.UniqueID, sizeof(Store.UniqueID))+_T("\n"));

							// IndexVersion
							tmpStr.Format(_T("\"IndexVersion\"=dword:%.8x\n"), Store.IndexVersion);
							f.WriteString(tmpStr);

							// CreationTime
							f.WriteString(_T("\"CreationTime\"=hex:")+MakeHex((BYTE*)&Store.CreationTime, sizeof(Store.CreationTime))+_T("\n"));

							// FileTime
							f.WriteString(_T("\"FileTime\"=hex:")+MakeHex((BYTE*)&Store.FileTime, sizeof(Store.FileTime))+_T("\n"));

							// MaintenanceTime
							f.WriteString(_T("\"MaintenanceTime\"=hex:")+MakeHex((BYTE*)&Store.MaintenanceTime, sizeof(Store.MaintenanceTime))+_T("\n"));

							// SynchronizeTime
							f.WriteString(_T("\"SynchronizeTime\"=hex:")+MakeHex((BYTE*)&Store.SynchronizeTime, sizeof(Store.SynchronizeTime))+_T("\n"));
						}

					Ptr += LFKeySize;
				}
			}
			catch(CFileException ex)
			{
				LFErrorBox(this, LFDriveNotReady);
			}

			f.Close();
		}
	}

	free(pStoreIDs);
}

LRESULT LFStorePropertiesToolsPage::OnUpdateStore(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	const BOOL CanSynchronize = (p_Store->Mode & LFStoreModeBackendMask)>LFStoreModeBackendInternal;

	GetDlgItem(IDC_SYNCHRONIZED)->EnableWindow(CanSynchronize);
	GetDlgItem(IDC_RUNSYNCHRONIZE)->EnableWindow(CanSynchronize);

	if (p_StoreValid)
	{
		WCHAR tmpStr[256];
		CString tmpText;

		LFTimeToString(p_Store->MaintenanceTime, tmpStr, 256);
		tmpText.Format(m_MaskMaintenance, tmpStr);
		GetDlgItem(IDC_MAINTENANCE)->SetWindowText(tmpText);

		LFTimeToString(p_Store->SynchronizeTime, tmpStr, 256);
		tmpText.Format(m_MaskSynchronized, tmpStr);
		GetDlgItem(IDC_SYNCHRONIZED)->SetWindowText(tmpText);
	}

	return NULL;
}
