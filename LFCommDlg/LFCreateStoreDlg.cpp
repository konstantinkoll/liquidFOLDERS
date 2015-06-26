
// LFCreateStoreDlg.cpp: Implementierung der Klasse LFCreateStoreDlg
//

#include "stdafx.h"
#include "LFCommDlg.h"
#include "LFCreateStoreDlg.h"


// LFCreateStoreDlg
//

#define GetSelectedVolume() m_wndExplorerList.GetNextItem(-1, LVIS_SELECTED)

LFCreateStoreDlg::LFCreateStoreDlg(CWnd* pParentWnd)
	: LFDialog(IDD_CREATESTORE, pParentWnd)
{
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
		LFStoreDescriptor store;
		ZeroMemory(&store, sizeof(store));

		store.Mode = LFStoreModeBackendInternal;

		GetDlgItem(IDC_STORENAME)->GetWindowText(store.StoreName, 256);
		GetDlgItem(IDC_STORECOMMENT)->GetWindowText(store.StoreComment, 256);

		CHAR cVolume = '\0';
		if (!m_wndAutoPath.GetCheck())
		{
			INT Index = GetSelectedVolume();
			if (Index!=-1)
				cVolume = m_DriveLetters[Index];
		}

		if (cVolume)
		{
			store.Mode |= (LFGetSourceForVolume(cVolume)==LFTypeSourceUnknown) ? LFStoreModeIndexInternal : m_wndMakeSearchable.GetCheck() ? LFStoreModeIndexHybrid : LFStoreModeIndexExternal;
			swprintf_s(store.DatPath, MAX_PATH, L"%c:\\", cVolume);
		}
		else
		{
			store.Mode |= LFStoreModeIndexInternal;
			store.Flags |= LFStoreFlagAutoLocation;
		}

		CWaitCursor csr;
		LFErrorBox(LFCreateStore(&store), GetSafeHwnd());
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
	ON_BN_CLICKED(IDC_AUTOPATH, OnUpdate)
	ON_BN_CLICKED(IDC_VOLUMEPATH, OnUpdate)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_VOLUMELIST, OnItemChanged)
	ON_COMMAND(IDM_VOLUME_FORMAT, OnVolumeFormat)
	ON_COMMAND(IDM_VOLUME_EJECT, OnVolumeEject)
	ON_COMMAND(IDM_VOLUME_PROPERTIES, OnVolumeProperties)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_VOLUME_FORMAT, IDM_VOLUME_PROPERTIES, OnUpdateVolumeCommands)
	ON_REGISTERED_MESSAGE(LFGetApp()->p_MessageIDs->VolumesChanged, OnVolumesChanged)
	ON_REGISTERED_MESSAGE(LFGetApp()->p_MessageIDs->StoresChanged, OnVolumesChanged)
END_MESSAGE_MAP()

BOOL LFCreateStoreDlg::OnInitDialog()
{
	LFDialog::OnInitDialog();

	m_wndIcon.SetCoreIcon(2);

	m_wndAutoPath.SetCheck(TRUE);

	m_wndExplorerList.SetImageList(&LFGetApp()->m_SystemImageListSmall, LVSIL_SMALL);
	m_wndExplorerList.SetImageList(&LFGetApp()->m_SystemImageListExtraLarge, LVSIL_NORMAL);
	m_wndExplorerList.SetMenus(IDM_VOLUME);

	UpdateVolumes();

	return TRUE;
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

	m_wndIcon.SetCoreIcon(Source+1);
}

void LFCreateStoreDlg::OnItemChanged(NMHDR* pNMHDR, LRESULT* /*pResult*/)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	if ((pNMListView->uChanged & LVIF_STATE) && ((pNMListView->uOldState & LVIS_SELECTED) || (pNMListView->uNewState & LVIS_SELECTED)))
		OnUpdate();
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

LRESULT LFCreateStoreDlg::OnVolumesChanged(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	UpdateVolumes();

	return NULL;
}
