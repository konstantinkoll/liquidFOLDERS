
// LFStorePropertiesDlg.cpp: Implementierung der Klasse LFStorePropertiesDlg
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CUsageList
//

CIcons CUsageList::m_ContextIcons;
CString CUsageList::m_OtherFiles;

CUsageList::CUsageList()
	: CFrontstageItemView(FRONTSTAGE_ENABLESCROLLING, sizeof(UsageItemData))
{
	if (m_OtherFiles.IsEmpty())
		ENSURE(m_OtherFiles.LoadString(IDS_OTHERFILES));

	// Item
	SetItemHeight(m_ContextIcons.LoadForSize(IDB_CONTEXTS_16, max(32, 2*m_DefaultFontHeight)), 2);
}

INT CUsageList::CompareUsage(UsageItemData* pData1, UsageItemData* pData2, const SortParameters& /*Parameters*/)
{
	const INT Result =(INT)((pData2->FileSize-pData1->FileSize)>>32);

	return Result ? Result : wcscmp(LFGetApp()->m_Contexts[pData1->Context].Name, LFGetApp()->m_Contexts[pData2->Context].Name);
}

void CUsageList::AddContext(const LFStatistics& Statistics, UINT Context)
{
	UsageItemData Data;

	Data.Context = Context;
	Data.FileCount = Statistics.FileCount[Context];
	Data.FileSize = Statistics.FileSize[Context];

	AddItem(&Data);
}

void CUsageList::SetUsage(LFStatistics Statistics)
{
	ASSERT(LFContextAllFiles==0);

	// Add contexts
	SetItemCount(LFLastPersistentContext+1, FALSE);

	for (UINT a=1; a<=LFLastPersistentContext; a++)
		if (Statistics.FileCount[a])
		{
			if (a!=LFContextFilters)
			{
				Statistics.FileCount[0] -= Statistics.FileCount[a];
				Statistics.FileSize[0] -= Statistics.FileSize[a];
			}

			AddContext(Statistics, a);
		}

	if (Statistics.FileSize[0])
		AddContext(Statistics, 0);

	LastItem();
	SortItems((PFNCOMPARE)CompareUsage);

	AdjustLayout();
}

void CUsageList::AdjustLayout()
{
	AdjustLayoutColumns();
}

void CUsageList::DrawItem(CDC& dc, Graphics& /*g*/, LPCRECT rectItem, INT Index, BOOL Themed)
{
	const UsageItemData* pData = (UsageItemData*)GetItemData(Index);
	const LFContextDescriptor* pContextDescriptor = &LFGetApp()->m_Contexts[pData->Context];

	CRect rect(rectItem);
	rect.right -= ITEMVIEWPADDING;

	// Occupied storage
	WCHAR tmpStr[256];
	LFSizeToString(pData->FileSize, tmpStr, 256);

	dc.DrawText(tmpStr, -1, rect, DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
	rect.right -= LFGetApp()->m_LargeFont.GetTextExtent(tmpStr).cx+(BACKSTAGEBORDER-ITEMVIEWPADDING);

	SetDarkTextColor(dc, Index, Themed);
	DrawTile(dc, rect, m_ContextIcons, pData->Context,
		GetLightTextColor(dc, Index, Themed), 2,
		(pData->Context==LFContextAllFiles) ? m_OtherFiles : pContextDescriptor->Name, pContextDescriptor->Comment);
}


// LFStorePropertiesDlg
//

UINT LFStorePropertiesDlg::m_LastTab = 0;

LFStorePropertiesDlg::LFStorePropertiesDlg(const ABSOLUTESTOREID& StoreID, CWnd* pParentWnd)
	: LFTabbedDialog(0, pParentWnd, &m_LastTab)
{
	if (LFGetStoreSettings(StoreID, m_StoreDescriptor)==LFOk)
	{
		m_StoreUniqueID = m_StoreDescriptor.UniqueID;
	}
	else
	{
		ZeroMemory(&m_StoreUniqueID, sizeof(m_StoreUniqueID));
	}

	m_StoreDescriptorValid = FALSE;
}

void LFStorePropertiesDlg::DoDataExchange(CDataExchange* pDX)
{
	LFTabbedDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_STOREICON, m_wndStoreIcon);
	DDX_Control(pDX, IDC_STORENAME, m_wndStoreName);
	DDX_Control(pDX, IDC_COMMENTS, m_wndStoreComment);
	DDX_Control(pDX, IDC_MAKEDEFAULT, m_wndMakeDefault);
	DDX_Control(pDX, IDC_MAKESEARCHABLE, m_wndMakeSearchable);

	DDX_Control(pDX, IDC_SYNCHRONIZEICON, m_wndSynchronizeIcon);
	DDX_Control(pDX, IDC_MAINTENANCEICON, m_wndMaintenanceIcon);
	DDX_Control(pDX, IDC_BACKUPICON, m_wndBackupIcon);

	if (pDX->m_bSaveAndValidate)
	{
		CWaitCursor WaitCursor;

		WCHAR StoreName[256];
		m_wndStoreName.GetWindowText(StoreName, 256);

		WCHAR Comment[256];
		m_wndStoreComment.GetWindowText(Comment, 256);

		// Save settings, they get overwritten in the dialog box when updating the attributes
		const BOOL SetDefault = (m_wndMakeDefault.IsWindowEnabled() && m_wndMakeDefault.GetCheck());
		const BOOL MakeSearchable = m_wndMakeSearchable.GetCheck();

		UINT Result = LFSetStoreAttributes(m_StoreDescriptor.StoreID, StoreName, Comment);
		if (Result!=LFOk)
		{
			LFErrorBox(this, Result);
		}
		else
		{
			if (SetDefault)
				LFErrorBox(this, LFSetDefaultStore(m_StoreDescriptor.StoreID));

			ASSERT(m_wndMakeSearchable.IsWindowVisible()==((m_StoreDescriptor.Mode & LFStoreModeIndexMask)>=LFStoreModeIndexHybrid));

			if (m_wndMakeSearchable.IsWindowVisible())
				LFErrorBox(this, LFMakeStoreSearchable(m_StoreDescriptor.StoreID, MakeSearchable));
		}
	}
}

void LFStorePropertiesDlg::AdjustLayout(const CRect& rectLayout, UINT nFlags)
{
	LFTabbedDialog::AdjustLayout(rectLayout, nFlags);

	UINT ExplorerHeight = 0;
	if (IsWindow(m_wndUsageHeader))
	{
		ExplorerHeight = m_wndUsageHeader.GetPreferredHeight();
		m_wndUsageHeader.SetWindowPos(NULL, rectLayout.left, rectLayout.top, rectLayout.Width(), ExplorerHeight, nFlags);
	}

	if (IsWindow(m_wndUsageList))
		m_wndUsageList.SetWindowPos(NULL, rectLayout.left, rectLayout.top+ExplorerHeight, rectLayout.Width(), m_BottomDivider-rectLayout.top-ExplorerHeight, nFlags);
}

BOOL LFStorePropertiesDlg::InitSidebar(LPSIZE pszTabArea)
{
	if (!LFTabbedDialog::InitSidebar(pszTabArea))
		return FALSE;

	AddTab(IDD_STOREPROPERTIES_GENERAL, pszTabArea);
	AddTab(IDD_STOREPROPERTIES_USAGE, pszTabArea);
	AddTab(IDD_STOREPROPERTIES_TOOLS, pszTabArea);
	AddTab(IDD_STOREPROPERTIES_INDEX, pszTabArea);

	return TRUE;
}

BOOL LFStorePropertiesDlg::InitDialog()
{
	// Useage
	m_wndUsageHeader.Create(this, IDC_USAGEHEADER);
	AddControl(m_wndUsageHeader, 1);

	m_wndUsageList.Create(this, IDC_USAGELIST);
	AddControl(m_wndUsageList, 1);

	// Caption
	m_DialogCaption.Format(IDS_STOREPROPERTIES, m_StoreDescriptor.StoreName);

	// Icons
	m_wndSynchronizeIcon.SetTaskIcon(AfxGetResourceHandle(), IDI_STORESYNCHRONIZE);
	m_wndMaintenanceIcon.SetTaskIcon(AfxGetResourceHandle(), IDI_STOREMAINTENANCE);
	m_wndBackupIcon.SetTaskIcon(AfxGetResourceHandle(), IDI_STOREBACKUP);

	// Masks
	GetDlgItem(IDC_CONTENTS)->GetWindowText(m_MaskContents);
	GetDlgItem(IDC_MAINTENANCE)->GetWindowText(m_MaskMaintenance);
	GetDlgItem(IDC_SYNCHRONIZED)->GetWindowText(m_MaskSynchronized);

	// Store
	m_wndStoreIcon.SetCoreIcon(LFGetStoreIcon(&m_StoreDescriptor));

	if ((m_StoreDescriptor.Mode & LFStoreModeIndexMask)==LFStoreModeIndexInternal)
	{
		m_wndMakeSearchable.ShowWindow(SW_HIDE);
	}
	else
	{
		m_wndMakeSearchable.SetCheck((m_StoreDescriptor.Mode & LFStoreModeIndexMask)==LFStoreModeIndexHybrid);
	}

	// Store
	OnUpdateStore(NULL, NULL);

	return LFTabbedDialog::InitDialog();
}

CString LFStorePropertiesDlg::MakeHex(LPBYTE x, UINT bCount)
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

void LFStorePropertiesDlg::CEscape(CString &strEscape)
{
	for (INT a=strEscape.GetLength()-1; a>=0; a--)
		if ((strEscape[a]==L'\\') || (strEscape[a]==L'\"'))
			strEscape.Insert(a, L'\\');
}


BEGIN_MESSAGE_MAP(LFStorePropertiesDlg, LFTabbedDialog)
	ON_NOTIFY(NM_CLICK, IDC_CONTENTS, OnShowUsageTab)
	ON_BN_CLICKED(IDC_RUNSYNCHRONIZE, OnRunSynchronize)
	ON_BN_CLICKED(IDC_RUNMAINTENANCE, OnRunMaintenance)
	ON_BN_CLICKED(IDC_RUNBACKUP, OnRunBackup)

	ON_REGISTERED_MESSAGE(LFGetApp()->p_MessageIDs->StoresChanged, OnUpdateStore)
	ON_REGISTERED_MESSAGE(LFGetApp()->p_MessageIDs->StoreAttributesChanged, OnUpdateStore)
	ON_REGISTERED_MESSAGE(LFGetApp()->p_MessageIDs->StatisticsChanged, OnUpdateStore)
END_MESSAGE_MAP()

void LFStorePropertiesDlg::OnShowUsageTab(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	SelectTab(1);

	*pResult = 0;
}

void LFStorePropertiesDlg::OnRunSynchronize()
{
	LFRunSynchronizeStores(m_StoreDescriptor.StoreID, this);
}

void LFStorePropertiesDlg::OnRunMaintenance()
{
	LFRunStoreMaintenance(this);
}

void LFStorePropertiesDlg::OnRunBackup()
{
	// Get stores
	UINT Result;
	LPCABSOLUTESTOREID lpcStoreIDs;
	UINT Count;
	if ((Result=LFGetAllStores(lpcStoreIDs, Count))!=LFOk)
	{
		LFErrorBox(this, Result);
		return;
	}

	// Open file
	CString tmpStr((LPCSTR)IDS_REGFILEFILTER);
	tmpStr += _T(" (*.reg)|*.reg||");

	CFileDialog dlg(FALSE, _T(".reg"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, tmpStr, this);
	if (dlg.DoModal()==IDOK)
	{
		CWaitCursor WaitCursor;

		CStdioFile File;
		if (!File.Open(dlg.GetPathName(), CFile::modeCreate | CFile::modeWrite))
		{
			LFErrorBox(this, LFDriveNotReady);
		}
		else
		{
			try
			{
				File.WriteString(_T("Windows Registry Editor Version 5.00\n"));

				// Iterate stores
				for (UINT a=0; a<Count; a++)
				{
					LFStoreDescriptor StoreDescriptor;
					if (LFGetStoreSettings(lpcStoreIDs[a], StoreDescriptor)==LFOk)
						if ((StoreDescriptor.Mode & LFStoreModeIndexMask)!=LFStoreModeIndexExternal)
						{
							// Header
							tmpStr = _T("\n[HKEY_CURRENT_USER\\Software\\liquidFOLDERS\\Stores\\");
							tmpStr += StoreDescriptor.StoreID;
							File.WriteString(tmpStr+_T("]\n"));

							// Name
							tmpStr = StoreDescriptor.StoreName;
							CEscape(tmpStr);
							File.WriteString(_T("\"Name\"=\"")+tmpStr+_T("\"\n"));

							// Mode
							tmpStr.Format(_T("\"Mode\"=dword:%.8x\n"), StoreDescriptor.Mode);
							File.WriteString(tmpStr);

							// AutoLocation
							tmpStr.Format(_T("\"AutoLocation\"=dword:%.8x\n"), StoreDescriptor.Flags & LFStoreFlagsAutoLocation);
							File.WriteString(tmpStr);

							if ((StoreDescriptor.Flags & LFStoreFlagsAutoLocation)==0)
							{
								// Path
								tmpStr = StoreDescriptor.DatPath;
								CEscape(tmpStr);
								File.WriteString(_T("\"Path\"=\"")+tmpStr+_T("\"\n"));
							}

							// GUID
							File.WriteString(_T("\"GUID\"=hex:")+MakeHex((LPBYTE)&StoreDescriptor.UniqueID, sizeof(StoreDescriptor.UniqueID))+_T("\n"));

							// IndexVersion
							tmpStr.Format(_T("\"IndexVersion\"=dword:%.8x\n"), StoreDescriptor.IndexVersion);
							File.WriteString(tmpStr);

							// CreationTime
							File.WriteString(_T("\"CreationTime\"=hex:")+MakeHex((LPBYTE)&StoreDescriptor.CreationTime, sizeof(StoreDescriptor.CreationTime))+_T("\n"));

							// FileTime
							File.WriteString(_T("\"FileTime\"=hex:")+MakeHex((LPBYTE)&StoreDescriptor.FileTime, sizeof(StoreDescriptor.FileTime))+_T("\n"));

							// MaintenanceTime
							File.WriteString(_T("\"MaintenanceTime\"=hex:")+MakeHex((LPBYTE)&StoreDescriptor.MaintenanceTime, sizeof(StoreDescriptor.MaintenanceTime))+_T("\n"));

							// SynchronizeTime
							File.WriteString(_T("\"SynchronizeTime\"=hex:")+MakeHex((LPBYTE)&StoreDescriptor.SynchronizeTime, sizeof(StoreDescriptor.SynchronizeTime))+_T("\n"));
						}
				}
			}
			catch(CFileException ex)
			{
				LFErrorBox(this, LFDriveNotReady);
			}

			File.Close();
		}
	}

	free(lpcStoreIDs);
}


LRESULT LFStorePropertiesDlg::OnUpdateStore(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	// Get store data
	if ((m_StoreDescriptorValid=((LFGetStoreSettingsEx(m_StoreUniqueID, m_StoreDescriptor)==LFOk)))==TRUE)
		m_StoreIcon = LFGetStoreIcon(&m_StoreDescriptor, &m_StoreType);

	// Basic settings
	m_wndStoreName.EnableWindow(m_StoreDescriptorValid);
	m_wndStoreComment.EnableWindow(m_StoreDescriptorValid);
	m_wndMakeDefault.EnableWindow(m_StoreDescriptorValid);
	m_wndMakeSearchable.EnableWindow(m_StoreDescriptorValid);

	const BOOL Editable = m_StoreDescriptorValid && (m_StoreType & LFTypeWriteable);
	GetDlgItem(IDOK)->EnableWindow(Editable);

	// Usage
	m_wndUsageHeader.SetHeader(m_StoreDescriptor.StoreName, LFGetApp()->GetFreeBytesAvailable(m_StoreDescriptor));
	m_wndUsageList.SetUsage(m_StoreDescriptor);

	// Synchronize
	const BOOL CanSynchronize = m_StoreType & LFTypeSynchronizeAllowed;
	GetDlgItem(IDC_SYNCHRONIZED)->EnableWindow(CanSynchronize);
	GetDlgItem(IDC_RUNSYNCHRONIZE)->EnableWindow(CanSynchronize && Editable);

	// Update properties
	if (m_StoreDescriptorValid)
	{
		WCHAR tmpStr[256];
		CString tmpText;

		// Tab 0
		if (m_wndStoreName.LineLength()==0)
			m_wndStoreName.SetWindowText(m_StoreDescriptor.StoreName);

		if (m_wndStoreComment.LineLength()==0)
			m_wndStoreComment.SetWindowText(m_StoreDescriptor.Comments);

		if (m_StoreDescriptor.Flags & LFStoreFlagsMaintained)
		{
			GetDlgItem(IDC_CONTENTSLABEL)->EnableWindow(TRUE);

			LFGetFileSummary(tmpStr, 256, m_StoreDescriptor.Statistics.FileCount[LFContextAllFiles], m_StoreDescriptor.Statistics.FileSize[LFContextAllFiles]);
			tmpText.Format(m_MaskContents, tmpStr);
			GetDlgItem(IDC_CONTENTS)->SetWindowText(tmpText);
		}
		else
		{
			GetDlgItem(IDC_CONTENTSLABEL)->EnableWindow(FALSE);
			GetDlgItem(IDC_CONTENTS)->SetWindowText(_T(""));
		}

		LFTimeToString(m_StoreDescriptor.CreationTime, tmpStr, 256);
		GetDlgItem(IDC_CREATED)->SetWindowText(tmpStr);

		LFTimeToString(m_StoreDescriptor.FileTime, tmpStr, 256);
		GetDlgItem(IDC_UPDATED)->SetWindowText(tmpStr);

		GetDlgItem(IDC_LASTSEENLABEL)->EnableWindow((m_StoreDescriptor.Mode & LFStoreModeIndexMask)!=LFStoreModeIndexInternal);
		GetDlgItem(IDC_LASTSEEN)->SetWindowText(m_StoreDescriptor.LastSeen);

		ABSOLUTESTOREID StoreID;
		if (LFGetDefaultStore(StoreID)==LFOk)
			m_wndMakeDefault.SetCheck(m_StoreDescriptor.StoreID==StoreID);

		// Tab 1
		LFTimeToString(m_StoreDescriptor.MaintenanceTime, tmpStr, 256);
		tmpText.Format(m_MaskMaintenance, tmpStr);
		GetDlgItem(IDC_MAINTENANCE)->SetWindowText(tmpText);

		LFTimeToString(m_StoreDescriptor.SynchronizeTime, tmpStr, 256);
		tmpText.Format(m_MaskSynchronized, tmpStr);
		GetDlgItem(IDC_SYNCHRONIZED)->SetWindowText(tmpText);

		// Tab 2
		GetDlgItem(IDC_DATPATH)->SetWindowText(m_StoreDescriptor.DatPath);

		OLECHAR szGUID[MAX_PATH];
		StringFromGUID2(m_StoreDescriptor.UniqueID, szGUID, MAX_PATH);
		GetDlgItem(IDC_GUID)->SetWindowText(szGUID);

		GetDlgItem(IDC_IDXPATHMAIN)->SetWindowText(m_StoreDescriptor.IdxPathMain);
		GetDlgItem(IDC_IDXPATHAUXCAPTION)->EnableWindow(m_StoreDescriptor.IdxPathAux[0]!=L'\0');
		GetDlgItem(IDC_IDXPATHAUX)->SetWindowText(m_StoreDescriptor.IdxPathAux);

		LFUINTToString(m_StoreDescriptor.IndexVersion, tmpStr, 256);
		GetDlgItem(IDC_IDXVERSION)->SetWindowText(tmpStr);
	}

	return NULL;
}
