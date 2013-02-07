
// LFStorePropertiesToolsPage.cpp: Implementierung der Klasse LFStorePropertiesToolsPage
//

#include "stdafx.h"
#include "LFStorePropertiesToolsPage.h"
#include "Resource.h"


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

extern AFX_EXTENSION_MODULE LFCommDlgDLL;
extern LFMessageIDs* MessageIDs;

LFStorePropertiesToolsPage::LFStorePropertiesToolsPage(LFStoreDescriptor* pStore, BOOL* pStoreValid)
	: CPropertyPage(IDD_STOREPROPERTIES_TOOLS)
{
	ASSERT(pStore);
	ASSERT(pValid);

	p_Store = pStore;
	p_StoreValid = pStoreValid;
}

void LFStorePropertiesToolsPage::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_ICONMAINTENANCE, m_IconMaintenance);
}


BEGIN_MESSAGE_MAP(LFStorePropertiesToolsPage, CPropertyPage)
	ON_BN_CLICKED(IDC_RUNMAINTENANCE, OnRunMaintenance)
	ON_BN_CLICKED(IDC_RUNBACKUP, OnRunBackup)
	ON_REGISTERED_MESSAGE(MessageIDs->StoresChanged, OnUpdateStore)
	ON_REGISTERED_MESSAGE(MessageIDs->StoreAttributesChanged, OnUpdateStore)
END_MESSAGE_MAP()

BOOL LFStorePropertiesToolsPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	m_IconMaintenance.SetSmallIcon(LFCommDlgDLL.hResource, IDD_STOREMAINTENANCE);

	GetDlgItem(IDC_MAINTENANCE)->GetWindowText(m_Mask);

	// Store
	SendNotifyMessage(MessageIDs->StoresChanged, LFMSGF_IntStores | LFMSGF_ExtHybStores, NULL);

	return TRUE;  // TRUE zur�ckgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

void LFStorePropertiesToolsPage::OnRunMaintenance()
{
	LFRunMaintenance(this);
}

void LFStorePropertiesToolsPage::OnRunBackup()
{
	CHAR* Keys;
	UINT StoreCount;
	UINT res = LFGetStores(&Keys, &StoreCount);
	if (res!=LFOk)
	{
		LFErrorBox(res, GetSafeHwnd());
		return;
	}

	CString tmpStr;
	ENSURE(tmpStr.LoadString(IDS_REGFILEFILTER));
	tmpStr += _T(" (*.reg)|*.reg||");

	CFileDialog dlg(FALSE, _T(".reg"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, tmpStr, this);

	if (dlg.DoModal()==IDOK)
	{
		CWaitCursor csr;

		CStdioFile f;
		if (!f.Open(dlg.GetPathName(), CFile::modeCreate | CFile::modeWrite))
		{
			LFErrorBox(LFDriveNotReady, GetSafeHwnd());
		}
		else
		{
			try
			{
				f.WriteString(_T("Windows Registry Editor Version 5.00\n"));

				CHAR* Ptr = Keys;
				for (UINT a=0; a<StoreCount; a++)
				{
					LFStoreDescriptor s;
					if (LFGetStoreSettings(Ptr, &s)==LFOk)
						if (s.StoreMode<=LFItemCategoryHybridStores)
						{
							// Header
							tmpStr = _T("\n[HKEY_CURRENT_USER\\Software\\liquidFOLDERS\\Stores\\");
							tmpStr += s.StoreID;
							f.WriteString(tmpStr+_T("]\n"));

							// Name
							tmpStr = s.StoreName;
							CEscape(tmpStr);
							f.WriteString(_T("\"Name\"=\"")+tmpStr+_T("\"\n"));

							// Mode
							tmpStr.Format(_T("\"Mode\"=dword:%.8x\n"), s.StoreMode);
							f.WriteString(tmpStr);

							// AutoLocation
							tmpStr.Format(_T("\"AutoLocation\"=dword:%.8x\n"), s.AutoLocation);
							f.WriteString(tmpStr);

							if (!s.AutoLocation)
							{
								// Path
								tmpStr = s.DatPath;
								CEscape(tmpStr);
								f.WriteString(_T("\"Path\"=\"")+tmpStr+_T("\"\n"));
							}

							// GUID
							f.WriteString(_T("\"GUID\"=hex:")+MakeHex((BYTE*)&s.guid, sizeof(s.guid))+_T("\n"));

							// IndexVersion
							tmpStr.Format(_T("\"IndexVersion\"=dword:%.8x\n"), s.IndexVersion);
							f.WriteString(tmpStr);

							// CreationTime
							f.WriteString(_T("\"CreationTime\"=hex:")+MakeHex((BYTE*)&s.CreationTime, sizeof(s.CreationTime))+_T("\n"));

							// FileTime
							f.WriteString(_T("\"FileTime\"=hex:")+MakeHex((BYTE*)&s.FileTime, sizeof(s.FileTime))+_T("\n"));

							// MaintenanceTime
							f.WriteString(_T("\"MaintenanceTime\"=hex:")+MakeHex((BYTE*)&s.MaintenanceTime, sizeof(s.MaintenanceTime))+_T("\n"));
						}

					Ptr += LFKeySize;
				}
			}
			catch(CFileException ex)
			{
				LFErrorBox(LFDriveNotReady, GetSafeHwnd());
			}

			f.Close();
		}
	}

	free(Keys);
}

LRESULT LFStorePropertiesToolsPage::OnUpdateStore(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	if (p_StoreValid)
	{
		WCHAR tmpStr[256];
		LFTimeToString(p_Store->MaintenanceTime, tmpStr, 256);

		CString tmpText;
		tmpText.Format(m_Mask, tmpStr);
		GetDlgItem(IDC_MAINTENANCE)->SetWindowText(tmpText);
	}

	return NULL;
}
