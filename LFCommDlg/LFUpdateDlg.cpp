
// LFUpdateDlg.cpp: Implementierung der Klasse LFUpdateDlg
//

#include "stdafx.h"
#include "LFCommDlg.h"
#include <wininet.h>


// Use a GUID to uniquely identify the tray icon: {7091D760-A474-4c14-86B0-2BBB1B842092}
static const GUID TrayIcon = { 0x7091D760, 0xA474, 0x4C14, { 0x86, 0xB0, 0x2B, 0xBB, 0x1B, 0x84, 0x20, 0x92 } };


// LFUpdateDlg
//

#define WM_TRAYMENU     WM_USER+6
#define MARGIN          4

LFUpdateDlg::LFUpdateDlg(const CString& Version, const CString& MSN, DWORD Features, CWnd* pParentWnd)
	: LFDialog(IDD_UPDATE, pParentWnd, TRUE)
{
	m_NotificationWindow = (pParentWnd==NULL);
	m_CaptionTop = m_IconTop = m_FeaturesTop = m_FeaturesLeft = m_FeatureItemHeight = 0;
	m_Connected = TRUE;

	p_Logo = LFGetApp()->GetCachedResourceImage(IDB_LIQUIDFOLDERS_64);

	m_Version = Version;
	m_MSN = MSN;
	m_Features = Features;

	ENSURE(m_AppName.LoadString(IDR_APPLICATION));
}

void LFUpdateDlg::DoDataExchange(CDataExchange* pDX)
{
	LFDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_VERSIONINFO, m_wndVersionInfo);
	DDX_Control(pDX, IDC_IGNOREUPDATE, m_wndIgnoreUpdate);
}

void LFUpdateDlg::PaintOnBackground(CDC& dc, Graphics& g, const CRect& rectLayout)
{
	LFDialog::PaintOnBackground(dc, g, rectLayout);

	// Logo
	g.DrawImage(p_Logo, rectLayout.left+9, rectLayout.top+m_IconTop);

	// Caption
	BOOL Themed = IsCtrlThemed();

	CRect rectText(rectLayout);
	rectText.left = rectLayout.left+80;
	rectText.top = rectLayout.top+m_CaptionTop;

	CFont* pOldFont = dc.SelectObject(&m_CaptionFont);

	dc.SetTextColor(Themed ? 0xCC3300 : GetSysColor(COLOR_WINDOWTEXT));
	dc.DrawText(m_AppName, rectText, DT_SINGLELINE | DT_LEFT | DT_NOPREFIX | DT_END_ELLIPSIS);

	// Features
	rectText.SetRect(rectLayout.left+m_FeaturesLeft, rectLayout.top+m_FeaturesTop, rectLayout.right-m_FeaturesLeft, rectLayout.top+m_FeaturesTop+m_FeatureItemHeight);
	dc.SelectStockObject(DEFAULT_GUI_FONT);

	for (UINT a=0; a<=31; a++)
		if (m_Features & (1<<a))
		{
			const INT Index = a ? a+1 : LFGetApp()->OSVersion==OS_Vista ? 1 : 0;
			m_UpdateIcons.Draw(dc, rectText.left, rectText.top, Index);

			CRect rectLine(rectText);
			rectLine.left += m_FeatureItemHeight+MARGIN+MARGIN/2;

			CString Text((LPCSTR)IDS_UPDATE_FIRST+a);

			dc.SetTextColor(a<3 ? 0x0000FF : Themed ? 0x000000 : GetSysColor(COLOR_WINDOWTEXT));
			dc.DrawText(Text, rectLine, DT_NOPREFIX | DT_SINGLELINE | DT_LEFT | DT_VCENTER | DT_END_ELLIPSIS);

			rectText.OffsetRect(0, m_FeatureItemHeight+MARGIN);
		}

	dc.SelectObject(pOldFont);
}

void LFUpdateDlg::UpdateDownloadButton()
{
	GetDlgItem(IDOK)->EnableWindow(m_Connected && !m_wndIgnoreUpdate.GetCheck());
}

void LFUpdateDlg::CheckInternetConnection()
{
	DWORD Flags;
	m_Connected = InternetGetConnectedState(&Flags, 0);

	UpdateDownloadButton();
}

void LFUpdateDlg::UpdatePosition()
{
	if (!m_NotificationWindow)
		return;

	MONITORINFO MonitorInfo;
	MonitorInfo.cbSize = sizeof(MONITORINFO);

	CRect rectScreen;
	if (GetMonitorInfo(MonitorFromPoint(CPoint(0, 0), MONITOR_DEFAULTTONEAREST), &MonitorInfo))
	{
		rectScreen = MonitorInfo.rcWork;
	}
	else
	{
		SystemParametersInfo(SPI_GETWORKAREA, 0, &rectScreen, 0);
	}

	CRect rectWindow;
	GetWindowRect(rectWindow);

	SetWindowPos(&wndTopMost, rectScreen.right-rectWindow.Width()-1, rectScreen.bottom-rectWindow.Height()-1, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOCOPYBITS);
}

BOOL LFUpdateDlg::AddTrayIcon()
{
	INT Size = GetSystemMetrics(SM_CXSMICON);

	NOTIFYICONDATA nid;
	ZeroMemory(&nid, sizeof(nid));
	nid.cbSize = sizeof(nid);
	nid.hWnd = GetSafeHwnd();
	nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE | NIF_GUID | NIF_SHOWTIP;
	nid.guidItem = TrayIcon;
	nid.uVersion = NOTIFYICON_VERSION_4;
	nid.uCallbackMessage = WM_TRAYMENU;
	nid.hIcon = (HICON)LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_APPLICATION), IMAGE_ICON, Size, Size, LR_DEFAULTCOLOR);
	GetWindowText(nid.szTip, 128);
	Shell_NotifyIcon(NIM_ADD, &nid);

	return Shell_NotifyIcon(NIM_SETVERSION, &nid);
}

BOOL LFUpdateDlg::RemoveTrayIcon()
{
	NOTIFYICONDATA nid;
	ZeroMemory(&nid, sizeof(nid));
	nid.cbSize = sizeof(nid);
	nid.hWnd = GetSafeHwnd();
	nid.uFlags = NIF_GUID;
	nid.guidItem = TrayIcon;

	return Shell_NotifyIcon(NIM_DELETE, &nid);
}

void LFUpdateDlg::ShowMenu()
{
	CMenu Menu;
	Menu.LoadMenu(IDM_UPDATE);
	ASSERT_VALID(&Menu);

	CMenu* pPopup = Menu.GetSubMenu(0);
	ASSERT_VALID(pPopup);

	MENUITEMINFO mii;
	mii.cbSize = sizeof(mii);
	mii.fMask = MIIM_BITMAP;
	mii.hbmpItem = HBMMENU_POPUP_RESTORE;

	pPopup->SetMenuItemInfo(0, &mii, TRUE);
	pPopup->SetDefaultItem(0, TRUE);

	CPoint pt;
	GetCursorPos(&pt);

	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, this);
}

BOOL LFUpdateDlg::InitDialog()
{
	AddBottomRightControl(IDC_HIDE);
	AddBottomRightControl(&m_wndIgnoreUpdate);

	if (m_NotificationWindow)
		LFGetApp()->AddFrame(this);

	INT DynamicHeight = 0;

	// Version
	CRect rectWnd;
	m_wndVersionInfo.GetWindowRect(rectWnd);
	ScreenToClient(rectWnd);

	CString Caption;
	m_wndVersionInfo.GetWindowText(Caption);

	CString Text;
	Text.Format(Caption, m_Version, m_MSN);

	m_wndVersionInfo.SetWindowText(Text);

	// Hintergrund
	const INT Height = rectWnd.Height();
	const INT LineGap = Height/6;
	const INT HeightCaption = 4*LineGap;
	const INT HeightVersion = 2*LineGap;

	CRect rectLayout;
	GetLayoutRect(rectLayout);

	m_CaptionFont.CreateFont(HeightCaption, ANTIALIASED_QUALITY, FW_NORMAL, 0, _T("Letter Gothic"));
	m_VersionFont.CreateFont(HeightVersion);
	m_wndVersionInfo.SetFont(&m_VersionFont);

	m_CaptionTop = rectWnd.top-rectLayout.top+(rectWnd.Height()-HeightCaption-HeightVersion)/2-4;
	m_IconTop = rectWnd.top-rectLayout.top+(rectWnd.Height()-62)/2-3;

	rectWnd.left = rectLayout.left+82;
	rectWnd.top = rectLayout.top+m_CaptionTop+HeightCaption;
	m_wndVersionInfo.SetWindowPos(NULL, rectWnd.left, rectWnd.top, rectWnd.Width(), rectWnd.Height(), SWP_NOACTIVATE | SWP_NOZORDER);

	// Feature-Liste
	GetDlgItem(IDC_IGNOREUPDATE)->GetWindowRect(rectWnd);
	ScreenToClient(rectWnd);

	if (m_Features)
	{
		m_FeaturesTop = rectWnd.top-rectLayout.top;
		m_FeaturesLeft = rectWnd.left-rectLayout.left;

		UINT Count = 0;
		for (DWORD Features=m_Features; Features; Features>>=1)
			if (Features & 1)
				Count++;

		m_FeatureItemHeight = LFGetApp()->m_DialogFont.GetFontHeight()>14 ? 32 : Count<=3 ? 32 : 16;
		m_UpdateIcons.Load(m_FeatureItemHeight==32 ? IDB_UPDATEICONS_32 : IDB_UPDATEICONS_16, m_FeatureItemHeight);

		DynamicHeight += Count*(m_FeatureItemHeight+MARGIN)+m_FeaturesLeft;
	}

	// Größe anpassen
	if (!m_NotificationWindow)
		DynamicHeight -= rectWnd.Height()+rectWnd.left-2;

	if (DynamicHeight)
	{
		GetWindowRect(rectWnd);
		SetWindowPos(NULL, 0, 0, rectWnd.Width(), rectWnd.Height()+DynamicHeight, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
	}

	// Audio
	LFGetApp()->PlayNotificationSound();

	// Stil
	if (m_NotificationWindow)
	{
		UpdatePosition();
	}
	else
	{
		GetDlgItem(IDC_IGNOREUPDATE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_HIDE)->ShowWindow(SW_HIDE);
	}

	// Internet
	CheckInternetConnection();
	SetTimer(1, 1000, NULL);

	return TRUE;
}

void LFUpdateDlg::EndDialog(INT nResult)
{
	if (m_NotificationWindow)
	{
		if (m_wndIgnoreUpdate.GetCheck())
			LFGetApp()->WriteGlobalString(_T("IgnoreUpdateMSN"), m_MSN);

		DestroyWindow();
	}
	else
	{
		LFDialog::EndDialog(nResult);
	}
}


BEGIN_MESSAGE_MAP(LFUpdateDlg, LFDialog)
	ON_WM_DESTROY()
	ON_WM_NCDESTROY()
	ON_WM_TIMER()
	ON_WM_THEMECHANGED()
	ON_WM_DWMCOMPOSITIONCHANGED()
	ON_MESSAGE(WM_DISPLAYCHANGE, OnDisplayChange)
	ON_BN_CLICKED(IDC_IGNOREUPDATE, OnIgnoreUpdate)
	ON_NOTIFY(NM_CLICK, IDC_HIDE, OnHide)
	ON_BN_CLICKED(IDOK, OnDownload)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_MESSAGE(WM_TRAYMENU, OnTrayMenu)
	ON_COMMAND(IDM_UPDATE_RESTORE, OnRestore)
	ON_REGISTERED_MESSAGE(LFGetApp()->m_WakeupMsg, OnWakeup)
	ON_WM_COPYDATA()
END_MESSAGE_MAP()

void LFUpdateDlg::OnDestroy()
{
	KillTimer(1);
	RemoveTrayIcon();

	LFDialog::OnDestroy();

	if (m_NotificationWindow)
	{
		LFGetApp()->m_pUpdateNotification = NULL;
		LFGetApp()->KillFrame(this);
	}
}

void LFUpdateDlg::PostNcDestroy()
{
	if (m_NotificationWindow)
		delete this;
}

void LFUpdateDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent==1)
		CheckInternetConnection();

	LFDialog::OnTimer(nIDEvent);

	// Eat bogus WM_TIMER messages
	MSG msg;
	while (PeekMessage(&msg, m_hWnd, WM_TIMER, WM_TIMER, PM_REMOVE));
}

LRESULT LFUpdateDlg::OnDisplayChange(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	UpdatePosition();

	return NULL;
}

void LFUpdateDlg::OnIgnoreUpdate()
{
	UpdateDownloadButton();
}

void LFUpdateDlg::OnHide(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	OnCancel();

	*pResult = 0;
}

void LFUpdateDlg::OnDownload()
{
	CString URL((LPCSTR)IDS_UPDATEURL);

	ShellExecute(GetSafeHwnd(), _T("open"), URL, NULL, NULL, SW_SHOWNORMAL);

	EndDialog(IDOK);
}

void LFUpdateDlg::OnCancel()
{
	if (((CButton*)GetDlgItem(IDC_IGNOREUPDATE))->GetCheck() || !m_NotificationWindow)
	{
		EndDialog(IDCANCEL);
	}
	else
		if (AddTrayIcon())
		{
			ShowWindow(SW_HIDE);
			m_wndShadow.Update(this);
		}
}

LRESULT LFUpdateDlg::OnTrayMenu(WPARAM /*wParam*/, LPARAM lParam)
{
	switch (LOWORD(lParam))
	{
	case WM_LBUTTONUP:
	case WM_LBUTTONDBLCLK:
		OnRestore();

		break;

	case WM_RBUTTONUP:
		SetForegroundWindow();
		ShowMenu();

		break;
	}

	return NULL;
}

void LFUpdateDlg::OnRestore()
{
	RemoveTrayIcon();
	ShowWindow(SW_SHOW);
}
