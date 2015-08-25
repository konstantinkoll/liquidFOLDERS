
// LFCreateStoreDlg.cpp: Implementierung der Klasse LFCreateStoreDlg
//

#include "stdafx.h"
#include "LFCommDlg.h"
#include "LFCreateStoreDlg.h"


// LFCreateStoreDlg
//

#define WM_VOLUMECHANGE     WM_USER+5
#define GetSelectedVolume() m_wndExplorerList.GetNextItem(-1, LVIS_SELECTED)

LFCreateStoreDlg::LFCreateStoreDlg(CWnd* pParentWnd)
	: LFDialog(IDD_CREATESTORE, pParentWnd)
{
	m_SHChangeNotifyRegister = 0;
}

void LFCreateStoreDlg::DoDataExchange(CDataExchange* pDX)
{
	LFDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_STOREICON, m_wndIcon);
	DDX_Control(pDX, IDC_AUTOPATH, m_wndAutoPath);
	DDX_Control(pDX, IDC_MAKESEARCHABLE, m_wndMakeSearchable);
	DDX_Control(pDX, IDC_VOLUMELIST, m_wndExplorerList);

	if (pDX->m_bSaveAndValidate)
	{
		LFStoreDescriptor Store;
		ZeroMemory(&Store, sizeof(Store));

		Store.Mode = LFStoreModeBackendInternal;

		GetDlgItem(IDC_STORENAME)->GetWindowText(Store.StoreName, 256);
		GetDlgItem(IDC_STORECOMMENT)->GetWindowText(Store.StoreComment, 256);

		CHAR cVolume = '\0';
		if (!m_wndAutoPath.GetCheck())
		{
			INT Index = GetSelectedVolume();
			if (Index!=-1)
				cVolume = m_DriveLetters[Index];
		}

		if (cVolume)
		{
			Store.Mode |= (LFGetSourceForVolume(cVolume)==LFTypeSourceUnknown) ? LFStoreModeIndexInternal : m_wndMakeSearchable.GetCheck() ? LFStoreModeIndexHybrid : LFStoreModeIndexExternal;
			swprintf_s(Store.DatPath, MAX_PATH, L"%c:\\", cVolume);
		}
		else
		{
			Store.Mode |= LFStoreModeIndexInternal;
			Store.Flags |= LFStoreFlagAutoLocation;
		}

		CWaitCursor csr;
		LFErrorBox(this, LFCreateStore(&Store));
	}
}

void LFCreateStoreDlg::UpdateVolumes()
{
	DWORD VolumesOnSystem = LFGetLogicalVolumes();

	m_wndExplorerList.DeleteAllItems();

	LVITEM lvi;
	ZeroMemory(&lvi, sizeof(lvi));
	lvi.mask = LVIF_TEXT | LVIF_IMAGE;

	for (CHAR cVolume='A'; cVolume<='Z'; cVolume++, VolumesOnSystem>>=1)
	{
		if ((VolumesOnSystem & 1)==0)
			continue;

		WCHAR szVolumeRoot[] = L" :\\";
		szVolumeRoot[0] = cVolume;

		SHFILEINFO sfi;
		if (SHGetFileInfo(szVolumeRoot, 0, &sfi, sizeof(SHFILEINFO), SHGFI_DISPLAYNAME | SHGFI_ATTRIBUTES | SHGFI_SYSICONINDEX))
		{
			if (!sfi.dwAttributes)
				continue;

			lvi.pszText = sfi.szDisplayName;
			lvi.iImage = sfi.iIcon;
			m_wndExplorerList.InsertItem(&lvi);

			m_DriveLetters[lvi.iItem++] = cVolume;
		}
	}

	OnUpdate();
}


BEGIN_MESSAGE_MAP(LFCreateStoreDlg, LFDialog)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_AUTOPATH, OnUpdate)
	ON_BN_CLICKED(IDC_VOLUMEPATH, OnUpdate)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_VOLUMELIST, OnItemChanged)
	ON_NOTIFY(REQUEST_TOOLTIP_DATA, IDC_VOLUMELIST, OnRequestTooltipData)
	ON_MESSAGE(WM_VOLUMECHANGE, OnVolumeChange)

	ON_COMMAND(IDM_VOLUME_FORMAT, OnVolumeFormat)
	ON_COMMAND(IDM_VOLUME_EJECT, OnVolumeEject)
	ON_COMMAND(IDM_VOLUME_PROPERTIES, OnVolumeProperties)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_VOLUME_FORMAT, IDM_VOLUME_PROPERTIES, OnUpdateVolumeCommands)

END_MESSAGE_MAP()

BOOL LFCreateStoreDlg::OnInitDialog()
{
	LFDialog::OnInitDialog();

	m_wndAutoPath.SetCheck(TRUE);

	m_wndExplorerList.SetImageList(&LFGetApp()->m_SystemImageListSmall, LVSIL_SMALL);
	m_wndExplorerList.SetImageList(&LFGetApp()->m_SystemImageListExtraLarge, LVSIL_NORMAL);
	m_wndExplorerList.SetMenus(IDM_VOLUME);
	m_wndExplorerList.SetView(LV_VIEW_ICON);
	m_wndExplorerList.SetItemsPerRow(3);

	UpdateVolumes();

	// Benachrichtigung, wenn sich Laufwerke ändern
	SHChangeNotifyEntry shCNE = { NULL, TRUE };
	m_SHChangeNotifyRegister = SHChangeNotifyRegister(GetSafeHwnd(), SHCNRF_ShellLevel | SHCNRF_NewDelivery,
		SHCNE_DRIVEADD | SHCNE_DRIVEREMOVED | SHCNE_MEDIAINSERTED | SHCNE_MEDIAREMOVED,
		WM_VOLUMECHANGE, 1, &shCNE);

	return TRUE;
}

void LFCreateStoreDlg::OnDestroy()
{
	if (m_SHChangeNotifyRegister)
		VERIFY(SHChangeNotifyDeregister(m_SHChangeNotifyRegister));

	LFDialog::OnDestroy();
}

void LFCreateStoreDlg::OnUpdate()
{
	UINT Source = LFTypeSourceUnknown;

	if (!m_wndAutoPath.GetCheck())
	{
		m_wndExplorerList.EnableWindow(TRUE);

		INT Index = GetSelectedVolume();
		if (Index!=-1)
			Source = LFGetSourceForVolume(m_DriveLetters[Index]);
	}
	else
	{
		m_wndExplorerList.EnableWindow(FALSE);
	}

	if (Source==LFTypeSourceUnknown)
		Source = LFTypeSourceInternal;

	m_wndMakeSearchable.EnableWindow(Source!=LFTypeSourceInternal);

	m_wndIcon.SetCoreIcon(Source);
}

void LFCreateStoreDlg::OnItemChanged(NMHDR* pNMHDR, LRESULT* /*pResult*/)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	if ((pNMListView->uChanged & LVIF_STATE) && ((pNMListView->uOldState & LVIS_SELECTED) || (pNMListView->uNewState & LVIS_SELECTED)))
		OnUpdate();
}

void LFCreateStoreDlg::OnRequestTooltipData(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_TOOLTIPDATA* pTooltipData = (NM_TOOLTIPDATA*)pNMHDR;

	if (pTooltipData->Item!=-1)
	{
		WCHAR szVolumeRoot[] = L" :\\";
		szVolumeRoot[0] = m_DriveLetters[pTooltipData->Item];

		SHFILEINFO sfi;
		if (SHGetFileInfo(szVolumeRoot, 0, &sfi, sizeof(SHFILEINFO), SHGFI_SYSICONINDEX | SHGFI_DISPLAYNAME | SHGFI_TYPENAME | SHGFI_ATTRIBUTES))
		{
			wcscpy_s(pTooltipData->Text, 4096, sfi.szTypeName);
			pTooltipData->hIcon = LFGetApp()->m_SystemImageListExtraLarge.ExtractIcon(sfi.iIcon);

			pTooltipData->Show = TRUE;
		}
	}

	*pResult = 0;
}

LRESULT LFCreateStoreDlg::OnVolumeChange(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	UpdateVolumes();

	return NULL;
}


void LFCreateStoreDlg::OnVolumeFormat()
{
	INT Index = GetSelectedVolume();
	if (Index!=-1)
		LFGetApp()->ExecuteExplorerContextMenu(m_DriveLetters[Index], "format");
}

void LFCreateStoreDlg::OnVolumeEject()
{
	INT Index = GetSelectedVolume();
	if (Index!=-1)
		LFGetApp()->ExecuteExplorerContextMenu(m_DriveLetters[Index], "eject");
}

void LFCreateStoreDlg::OnVolumeProperties()
{
	INT Index = GetSelectedVolume();
	if (Index!=-1)
		LFGetApp()->ExecuteExplorerContextMenu(m_DriveLetters[Index], "properties");
}

void LFCreateStoreDlg::OnUpdateVolumeCommands(CCmdUI* pCmdUI)
{
	BOOL b = FALSE;

	INT Index = GetSelectedVolume();
	if (Index!=-1)
	{
		switch (pCmdUI->m_nID)
		{
		case IDM_VOLUME_FORMAT:
		case IDM_VOLUME_EJECT:
			b = LFGetSourceForVolume(m_DriveLetters[Index])!=LFTypeSourceUnknown;
			break;

		case IDM_VOLUME_PROPERTIES:
			b = TRUE;
			break;
		}
	}

	pCmdUI->Enable(b);
}
