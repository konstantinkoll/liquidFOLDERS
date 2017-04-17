
// LFCreateStoreDlg.cpp: Implementierung der Klasse LFCreateStoreDlg
//

#include "stdafx.h"
#include "LFCommDlg.h"
#include "LFCreateStoreDlg.h"


// LFCreateStoreDlg
//

#define WM_VOLUMECHANGE         WM_USER+5
#define GetSelectedVolume()     m_wndExplorerList.GetNextItem(-1, LVIS_SELECTED)

LFCreateStoreDlg::LFCreateStoreDlg(CWnd* pParentWnd)
	: LFDialog(IDD_CREATESTORE, pParentWnd)
{
	m_Result = LFCancel;
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
		GetDlgItem(IDC_STORENAME)->GetWindowText(m_StoreName, 256);

		WCHAR Comments[256];
		GetDlgItem(IDC_COMMENTS)->GetWindowText(Comments, 256);
		
		CHAR cVolume = '\0';
		if (!m_wndAutoPath.GetCheck())
		{
			INT Index = GetSelectedVolume();
			if (Index!=-1)
				cVolume = m_DriveLetters[Index];
		}

		CWaitCursor csr;
		m_Result = LFCreateStoreLiquidfolders(m_StoreName, Comments, cVolume, m_wndMakeSearchable.GetCheck());
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


BOOL LFCreateStoreDlg::InitDialog()
{
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

void LFCreateStoreDlg::OnDestroy()
{
	if (m_SHChangeNotifyRegister)
		VERIFY(SHChangeNotifyDeregister(m_SHChangeNotifyRegister));

	LFDialog::OnDestroy();
}

void LFCreateStoreDlg::OnUpdate()
{
	UINT Source = LFTypeSourceInternal;

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

	*pResult = FALSE;
	if (pTooltipData->Item!=-1)
	{
		WCHAR szVolumeRoot[] = L" :\\";
		szVolumeRoot[0] = m_DriveLetters[pTooltipData->Item];

		SHFILEINFO sfi;
		if (SHGetFileInfo(szVolumeRoot, 0, &sfi, sizeof(SHFILEINFO), SHGFI_SYSICONINDEX | SHGFI_DISPLAYNAME | SHGFI_TYPENAME | SHGFI_ATTRIBUTES))
		{
			wcscpy_s(pTooltipData->Hint, 4096, sfi.szTypeName);
			pTooltipData->hIcon = LFGetApp()->m_SystemImageListExtraLarge.ExtractIcon(sfi.iIcon);

			*pResult = TRUE;
		}
	}
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
	BOOL bEnable = FALSE;

	INT Index = GetSelectedVolume();
	if (Index!=-1)
	{
		switch (pCmdUI->m_nID)
		{
		case IDM_VOLUME_FORMAT:
		case IDM_VOLUME_EJECT:
			bEnable = LFGetSourceForVolume(m_DriveLetters[Index])>LFTypeSourceInternal;
			break;

		case IDM_VOLUME_PROPERTIES:
			bEnable = TRUE;
			break;
		}
	}

	pCmdUI->Enable(bEnable);
}
