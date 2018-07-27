
// LFCreateStoreDlg.cpp: Implementierung der Klasse LFCreateStoreDlg
//

#include "stdafx.h"
#include "LFCommDlg.h"
#include "LFCreateStoreDlg.h"


// CVolumeList
//

CVolumeList::CVolumeList()
	: CFrontstageItemView(FRONTSTAGE_ENABLESCROLLING | FRONTSTAGE_ENABLEFOCUSITEM, sizeof(VolumeItemData))
{
	WNDCLASS wndcls;
	ZeroMemory(&wndcls, sizeof(wndcls));
	wndcls.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	wndcls.lpfnWndProc = ::DefWindowProc;
	wndcls.hCursor = LFGetApp()->LoadStandardCursor(IDC_ARROW);
	wndcls.lpszClassName = L"CVolumeList";

	if (!(::GetClassInfo(AfxGetInstanceHandle(), L"CVolumeList", &wndcls)))
	{
		wndcls.hInstance = AfxGetInstanceHandle();

		if (!AfxRegisterClass(&wndcls))
			AfxThrowResourceException();
	}
}

void CVolumeList::ShowTooltip(const CPoint& point)
{
	ASSERT(m_HoverItem>=0);

	const VolumeItemData* pData = GetVolumeItemData(m_HoverItem);

	WCHAR szVolumeRoot[] = L" :\\";
	szVolumeRoot[0] = pData->cVolume;

	SHFILEINFO sfi;
	if (SHGetFileInfo(szVolumeRoot, 0, &sfi, sizeof(SHFILEINFO), SHGFI_SYSICONINDEX | SHGFI_DISPLAYNAME | SHGFI_TYPENAME | SHGFI_ATTRIBUTES))
		LFGetApp()->ShowTooltip(this, point, pData->DisplayName, sfi.szTypeName,
			LFGetApp()->m_SystemImageListExtraLarge.ExtractIcon(sfi.iIcon), NULL);
}

BOOL CVolumeList::GetContextMenu(CMenu& Menu, INT Index)
{
	if (Index>=0)
		Menu.LoadMenu(IDM_VOLUME);

	return FALSE;
}

void CVolumeList::AddVolume(CHAR cVolume, LPCWSTR DisplayName, INT iIcon)
{
	VolumeItemData Data;

	Data.cVolume = cVolume;
	wcsncpy_s(Data.DisplayName, 256, DisplayName, _TRUNCATE);
	Data.iIcon = iIcon;

	AddItem(&Data);
}

void CVolumeList::SetVolumes(UINT Mask)
{
	ASSERT(LFContextAllFiles==0);

	// Add volumes
	SetItemCount(26, FALSE);

	DWORD VolumesOnSystem = LFGetLogicalVolumes(Mask);

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

			AddVolume(cVolume, sfi.szDisplayName, sfi.iIcon);
		}
	}

	LastItem();

	AdjustLayout();
}

CHAR CVolumeList::GetSelectedVolume() const
{
	const INT Index = GetSelectedItem();

	return (Index<0) ? '\0' : GetVolumeItemData(Index)->cVolume;
}

void CVolumeList::AdjustLayout()
{
	AdjustLayoutSingleRow(3);
}

void CVolumeList::DrawItem(CDC& dc, Graphics& /*g*/, LPCRECT rectItem, INT Index, BOOL /*Themed*/)
{
	const VolumeItemData* pData = GetVolumeItemData(Index);

	CRect rect(rectItem);
	rect.DeflateRect(ITEMVIEWPADDING, ITEMVIEWPADDING);

	// Text
	CFont* pOldFont = dc.SelectObject(&LFGetApp()->m_DialogFont);
	dc.DrawText(pData->DisplayName, -1, rect, DT_END_ELLIPSIS | DT_NOPREFIX | DT_CENTER | DT_BOTTOM | DT_SINGLELINE);
	dc.SelectObject(pOldFont);

	rect.bottom -= LFGetApp()->m_DialogFont.GetFontHeight()+ITEMVIEWPADDING;

	// Icon
	LFGetApp()->m_SystemImageListExtraLarge.DrawEx(&dc, pData->iIcon,
		CPoint((rect.left+rect.right-LFGetApp()->m_ExtraLargeIconSize)/2, (rect.top+rect.bottom-LFGetApp()->m_ExtraLargeIconSize)/2),
		CSize(LFGetApp()->m_ExtraLargeIconSize, LFGetApp()->m_ExtraLargeIconSize), CLR_NONE, CLR_NONE,
		IsWindowEnabled() ? ILD_TRANSPARENT : ILD_BLEND50);
}


// LFCreateStoreDlg
//

#define WM_VOLUMECHANGE     WM_USER+4

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
	DDX_Control(pDX, IDC_VOLUMELIST, m_wndVolumeList);

	if (pDX->m_bSaveAndValidate)
	{
		GetDlgItem(IDC_STORENAME)->GetWindowText(m_StoreName, 256);

		WCHAR Comments[256];
		GetDlgItem(IDC_COMMENTS)->GetWindowText(Comments, 256);

		CWaitCursor WaitCursor;
		m_Result = LFCreateStoreLiquidfolders(m_StoreName, Comments,
			m_wndAutoPath.GetCheck() ? '\0' : GetSelectedVolume(),
			m_wndMakeSearchable.GetCheck());
	}
}

void LFCreateStoreDlg::UpdateVolumes()
{
	m_wndVolumeList.SetVolumes();

	OnUpdateControls();
}


BOOL LFCreateStoreDlg::InitDialog()
{
	m_wndAutoPath.SetCheck(TRUE);

	UpdateVolumes();

	// Shell notifications
	const SHChangeNotifyEntry shCNE = { NULL, TRUE };

	m_SHChangeNotifyRegister = SHChangeNotifyRegister(GetSafeHwnd(), SHCNRF_ShellLevel | SHCNRF_NewDelivery,
		SHCNE_DRIVEADD | SHCNE_DRIVEREMOVED | SHCNE_MEDIAINSERTED | SHCNE_MEDIAREMOVED,
		WM_VOLUMECHANGE, 1, &shCNE);

	return TRUE;
}


BEGIN_MESSAGE_MAP(LFCreateStoreDlg, LFDialog)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_AUTOPATH, OnUpdateControls)
	ON_BN_CLICKED(IDC_VOLUMEPATH, OnUpdateControls)
	ON_NOTIFY(IVN_SELECTIONCHANGED, IDC_VOLUMELIST, OnSelectionChanged)
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

void LFCreateStoreDlg::OnUpdateControls()
{
	UINT Source = LFTypeSourceInternal;

	// Volume list
	if (!m_wndAutoPath.GetCheck())
	{
		m_wndVolumeList.EnableWindow(TRUE);

		if (const CHAR cVolume = GetSelectedVolume())
			Source = LFGetSourceForVolume(cVolume);
	}
	else
	{
		m_wndVolumeList.EnableWindow(FALSE);
	}

	// Make searchable
	m_wndMakeSearchable.EnableWindow(Source!=LFTypeSourceInternal);

	// Icon
	m_wndIcon.SetCoreIcon(Source);
}

void LFCreateStoreDlg::OnSelectionChanged(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	OnUpdateControls();

	*pResult = 0;
}

LRESULT LFCreateStoreDlg::OnVolumeChange(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	UpdateVolumes();

	return NULL;
}


void LFCreateStoreDlg::OnVolumeFormat()
{
	if (const CHAR cVolume = GetSelectedVolume())
		LFGetApp()->ExecuteExplorerContextMenu(cVolume, "format");
}

void LFCreateStoreDlg::OnVolumeEject()
{
	if (const CHAR cVolume = GetSelectedVolume())
		LFGetApp()->ExecuteExplorerContextMenu(cVolume, "eject");
}

void LFCreateStoreDlg::OnVolumeProperties()
{
	if (const CHAR cVolume = GetSelectedVolume())
		LFGetApp()->ExecuteExplorerContextMenu(cVolume, "properties");
}

void LFCreateStoreDlg::OnUpdateVolumeCommands(CCmdUI* pCmdUI)
{
	BOOL bEnable = FALSE;

	if (const CHAR cVolume = GetSelectedVolume())
	{
		const UINT Source = LFGetSourceForVolume(cVolume);

		switch (pCmdUI->m_nID)
		{
		case IDM_VOLUME_FORMAT:
		case IDM_VOLUME_EJECT:
			bEnable = (Source>LFTypeSourceWindows) && (Source<LFTypeSourceNethood);
			break;

		case IDM_VOLUME_PROPERTIES:
			bEnable = TRUE;
			break;
		}
	}

	pCmdUI->Enable(bEnable);
}
