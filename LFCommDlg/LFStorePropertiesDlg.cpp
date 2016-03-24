
// LFStorePropertiesDlg.cpp: Implementierung der Klasse LFStorePropertiesDlg
//

#include "stdafx.h"
#include "LFCommDlg.h"


CString MakeHex(BYTE* x, UINT bCount)
{
	CString tmpStr;

	for (UINT a=0; a<bCount; a++)
	{
		CString Digit;
		Digit.Format(_T("%.2x"), x[a]);
		tmpStr += Digit;

		if (a<bCount-1)
			tmpStr += _T(",");
	}

	return tmpStr;
}

void CEscape(CString &str)
{
	for (INT a=str.GetLength()-1; a>=0; a--)
		if ((str[a]==L'\\') || (str[a]==L'\"'))
			str.Insert(a, L'\\');
}


// LFStorePropertiesDlg
//

UINT LFStorePropertiesDlg::m_LastTab = 0;

LFStorePropertiesDlg::LFStorePropertiesDlg(const CHAR* pStoreID, CWnd* pParentWnd)
	: LFTabbedDialog(0, pParentWnd, &m_LastTab)
{
	if (LFGetStoreSettings(pStoreID, &m_Store)==LFOk)
	{
		m_StoreUniqueID = m_Store.UniqueID;
		m_StoreValid = TRUE;
	}
	else
	{
		ZeroMemory(&m_StoreUniqueID, sizeof(m_StoreUniqueID));
		m_StoreValid = FALSE;
	}
}

void LFStorePropertiesDlg::DoDataExchange(CDataExchange* pDX)
{
	LFTabbedDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_STOREICON, m_wndIcon);
	DDX_Control(pDX, IDC_STORENAME, m_wndStoreName);
	DDX_Control(pDX, IDC_COMMENTS, m_wndStoreComment);
	DDX_Control(pDX, IDC_MAKEDEFAULT, m_wndMakeDefault);
	DDX_Control(pDX, IDC_MAKESEARCHABLE, m_wndMakeSearchable);

	if (pDX->m_bSaveAndValidate)
	{
		CWaitCursor csr;

		WCHAR StoreName[256];
		m_wndStoreName.GetWindowText(StoreName, 256);

		WCHAR Comment[256];
		m_wndStoreComment.GetWindowText(Comment, 256);

		// Save settings, they get overwritten in the dialog box when updating the attributes
		const BOOL SetDefault = (m_wndMakeDefault.IsWindowEnabled() && m_wndMakeDefault.GetCheck());
		const BOOL MakeSearchable = m_wndMakeSearchable.GetCheck();

		UINT Result = LFSetStoreAttributes(m_Store.StoreID, StoreName, Comment);
		if (Result!=LFOk)
		{
			LFErrorBox(this, Result);
		}
		else
		{
			if (SetDefault)
				LFErrorBox(this, LFSetDefaultStore(m_Store.StoreID));

			ASSERT(m_wndMakeSearchable.IsWindowVisible()==((m_Store.Mode & LFStoreModeIndexMask)>=LFStoreModeIndexHybrid));

			if (m_wndMakeSearchable.IsWindowVisible())
				LFErrorBox(this, LFMakeStoreSearchable(m_Store.StoreID, MakeSearchable));
		}
	}
}

BOOL LFStorePropertiesDlg::InitSidebar(LPSIZE pszTabArea)
{
	if (!LFTabbedDialog::InitSidebar(pszTabArea))
		return FALSE;

	AddTab(IDD_STOREPROPERTIES_GENERAL, pszTabArea);
	AddTab(IDD_STOREPROPERTIES_TOOLS, pszTabArea);
	AddTab(IDD_STOREPROPERTIES_INDEX, pszTabArea);

	return TRUE;
}

BOOL LFStorePropertiesDlg::InitDialog()
{
	BOOL Result =LFTabbedDialog::InitDialog();

	// Caption
	CString Caption;
	Caption.Format(IDS_STOREPROPERTIES, m_Store.StoreName);

	SetWindowText(Caption);

	// Masks
	GetDlgItem(IDC_MAINTENANCE)->GetWindowText(m_MaskMaintenance);
	GetDlgItem(IDC_SYNCHRONIZED)->GetWindowText(m_MaskSynchronized);

	// Store
	m_wndIcon.SetCoreIcon(LFGetStoreIcon(&m_Store));

	if ((m_Store.Mode & LFStoreModeIndexMask)==LFStoreModeIndexInternal)
	{
		m_wndMakeSearchable.ShowWindow(SW_HIDE);
	}
	else
	{
		m_wndMakeSearchable.SetCheck((m_Store.Mode & LFStoreModeIndexMask)==LFStoreModeIndexHybrid);
	}

	OnUpdateStore(NULL, NULL);

	return Result;
}

BEGIN_MESSAGE_MAP(LFStorePropertiesDlg, LFTabbedDialog)
	ON_BN_CLICKED(IDC_RUNSYNCHRONIZE, OnRunSynchronize)
	ON_BN_CLICKED(IDC_RUNMAINTENANCE, OnRunMaintenance)
	ON_BN_CLICKED(IDC_RUNBACKUP, OnRunBackup)

	ON_REGISTERED_MESSAGE(LFGetApp()->p_MessageIDs->StoresChanged, OnUpdateStore)
	ON_REGISTERED_MESSAGE(LFGetApp()->p_MessageIDs->StoreAttributesChanged, OnUpdateStore)
	ON_REGISTERED_MESSAGE(LFGetApp()->p_MessageIDs->StatisticsChanged, OnUpdateStore)
END_MESSAGE_MAP()

void LFStorePropertiesDlg::OnRunSynchronize()
{
	LFRunSynchronization(m_Store.StoreID, this);
}

void LFStorePropertiesDlg::OnRunMaintenance()
{
	LFRunMaintenance(this);
}

void LFStorePropertiesDlg::OnRunBackup()
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


LRESULT LFStorePropertiesDlg::OnUpdateStore(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	GetDlgItem(IDOK)->EnableWindow(m_StoreValid=(LFGetStoreSettingsEx(m_StoreUniqueID, &m_Store)==LFOk));

	m_wndStoreName.EnableWindow(m_StoreValid);
	m_wndStoreComment.EnableWindow(m_StoreValid);
	m_wndMakeDefault.EnableWindow(m_StoreValid);
	m_wndMakeSearchable.EnableWindow(m_StoreValid);

	const BOOL CanSynchronize = (m_Store.Mode & LFStoreModeBackendMask)>LFStoreModeBackendInternal;
	GetDlgItem(IDC_SYNCHRONIZED)->EnableWindow(CanSynchronize);
	GetDlgItem(IDC_RUNSYNCHRONIZE)->EnableWindow(CanSynchronize);

	if (m_StoreValid)
	{
		WCHAR tmpStr[256];
		CString tmpText;

		// Tab 0
		if (m_wndStoreName.LineLength()==0)
			m_wndStoreName.SetWindowText(m_Store.StoreName);

		if (m_wndStoreComment.LineLength()==0)
			m_wndStoreComment.SetWindowText(m_Store.Comments);

		LFCombineFileCountSize(m_Store.FileCount[LFContextAllFiles], m_Store.FileSize[LFContextAllFiles], tmpStr, 256);
		GetDlgItem(IDC_CONTENTS)->SetWindowText(tmpStr);

		LFTimeToString(m_Store.CreationTime, tmpStr, 256);
		GetDlgItem(IDC_CREATED)->SetWindowText(tmpStr);

		LFTimeToString(m_Store.FileTime, tmpStr, 256);
		GetDlgItem(IDC_UPDATED)->SetWindowText(tmpStr);

		GetDlgItem(IDC_LASTSEENCAPTION)->EnableWindow((m_Store.Mode & LFStoreModeIndexMask)!=LFStoreModeIndexInternal);
		GetDlgItem(IDC_LASTSEEN)->SetWindowText(m_Store.LastSeen);

		CHAR StoreID[LFKeySize];
		if (LFGetDefaultStore(StoreID)==LFOk)
			m_wndMakeDefault.SetCheck(strcmp(m_Store.StoreID, StoreID)==0);

		// Tab 1
		LFTimeToString(m_Store.MaintenanceTime, tmpStr, 256);
		tmpText.Format(m_MaskMaintenance, tmpStr);
		GetDlgItem(IDC_MAINTENANCE)->SetWindowText(tmpText);

		LFTimeToString(m_Store.SynchronizeTime, tmpStr, 256);
		tmpText.Format(m_MaskSynchronized, tmpStr);
		GetDlgItem(IDC_SYNCHRONIZED)->SetWindowText(tmpText);

		// Tab 2
		GetDlgItem(IDC_DATPATH)->SetWindowText(m_Store.DatPath);

		OLECHAR szGUID[MAX_PATH];
		StringFromGUID2(m_Store.UniqueID, szGUID, MAX_PATH);
		GetDlgItem(IDC_GUID)->SetWindowText(szGUID);

		GetDlgItem(IDC_IDXPATHMAIN)->SetWindowText(m_Store.IdxPathMain);
		GetDlgItem(IDC_IDXPATHAUXCAPTION)->EnableWindow(m_Store.IdxPathAux[0]!=L'\0');
		GetDlgItem(IDC_IDXPATHAUX)->SetWindowText(m_Store.IdxPathAux);

		LFUINTToString(m_Store.IndexVersion, tmpStr, 256);
		GetDlgItem(IDC_IDXVERSION)->SetWindowText(tmpStr);
	}

	return NULL;
}
