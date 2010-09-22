
// LFApplication.cpp: Definiert das Klassenverhalten für die Anwendung.
//

#include "stdafx.h"
#include "afxwinappex.h"
#include "LFCore.h"
#include "LFCommDlg.h"
#include <io.h>
#include <mmsystem.h>


// LFApplication
//

BEGIN_MESSAGE_MAP(LFApplication, CWinAppEx)
	ON_COMMAND(ID_APP_HELP, OnAppHelp)
	ON_COMMAND(ID_APP_PURCHASE, OnAppPurchase)
	ON_COMMAND(ID_APP_ENTERLICENSEKEY, OnAppEnterLicenseKey)
	ON_COMMAND(ID_APP_SUPPORT, OnAppSupport)
	ON_COMMAND(ID_APP_NEWFILEDROP, OnAppNewFileDrop)
	ON_COMMAND(ID_APP_NEWMIGRATE, OnAppNewMigrate)
	ON_COMMAND(ID_APP_NEWSTOREMANAGER, OnAppNewStoreManager)
	ON_COMMAND(ID_APP_PROMPT, OnAppPrompt)
	ON_UPDATE_COMMAND_UI_RANGE(ID_APP_HELP, ID_APP_ENTERLICENSEKEY, OnUpdateAppCommands)
END_MESSAGE_MAP()

extern AFX_EXTENSION_MODULE LFCommDlgDLL;

void PlayRegSound(CString Identifier)
{
	CString strFile;
	CString strKey = _T("AppEvents\\Schemes\\");
	strKey += Identifier;
	strKey += _T("\\.current");

	CSettingsStoreSP regSP;
	CSettingsStore& reg = regSP.Create(FALSE, TRUE);

	if (reg.Open(strKey))
		if (reg.Read(_T(""), strFile))
			if (!strFile.IsEmpty())
				PlaySound(strFile, NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT | SND_NOWAIT);
}


// LFApplication-Erstellung

LFApplication::LFApplication(UINT _HasGUI)
{
	HasGUI = _HasGUI;

	// Version
	OSVERSIONINFO osInfo;
	ZeroMemory(&osInfo, sizeof(OSVERSIONINFO));
	osInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osInfo);
	OSVersion = (osInfo.dwMajorVersion<6) ? OS_XP : ((osInfo.dwMajorVersion==6) && (osInfo.dwMinorVersion==0)) ? OS_Vista : OS_Seven;

	// DLL-Hijacking verhindern
	SetDllDirectory(_T(""));

	// Nachrichten
	MessageIDs = LFGetMessageIDs();

	// Themes
	hModThemes = LoadLibrary(_T("UXTHEME.DLL"));
	if (hModThemes)
	{
		zSetWindowTheme = (PFNSETWINDOWTHEME)GetProcAddress(hModThemes, "SetWindowTheme");
		zOpenThemeData = (PFNOPENTHEMEDATA)GetProcAddress(hModThemes, "OpenThemeData");
		zCloseThemeData = (PFNCLOSETHEMEDATA)GetProcAddress(hModThemes, "CloseThemeData");
		zDrawThemeBackground = (PFNDRAWTHEMEBACKGROUND)GetProcAddress(hModThemes, "DrawThemeBackground");
		zDrawThemeText = (PFNDRAWTHEMETEXT)GetProcAddress(hModThemes, "DrawThemeText");
		zDrawThemeTextEx = (PFNDRAWTHEMETEXTEX)GetProcAddress(hModThemes, "DrawThemeTextEx");
		zGetThemeSysFont = (PFNGETTHEMESYSFONT)GetProcAddress(hModThemes, "GetThemeSysFont");
		zGetThemeSysColor = (PFNGETTHEMESYSCOLOR)GetProcAddress(hModThemes, "GetThemeSysColor");
		zGetThemePartSize = (PFNGETTHEMEPARTSIZE)GetProcAddress(hModThemes, "GetThemePartSize");
		zSetWindowThemeAttribute = (PFNSETWINDOWTHEMEATTRIBUTE)GetProcAddress(hModThemes, "SetWindowThemeAttribute");
		zIsThemeActive = (PFNISTHEMEACTIVE)GetProcAddress(hModThemes, "IsThemeActive");

		m_ThemeLibLoaded = (zOpenThemeData && zCloseThemeData && zDrawThemeBackground && zDrawThemeText && zGetThemeSysFont && zGetThemeSysColor && zGetThemePartSize && zIsThemeActive);
		if (m_ThemeLibLoaded)
		{
			FreeLibrary(hModThemes);
			hModThemes = NULL;
		}
	}
	else
	{
		zSetWindowTheme = NULL;
		zOpenThemeData = NULL;
		zCloseThemeData = NULL;
		zDrawThemeBackground = NULL;
		zDrawThemeText = NULL;
		zDrawThemeTextEx = NULL;
		zGetThemeSysFont = NULL;
		zGetThemeSysColor = NULL;
		zGetThemePartSize = NULL;
		zSetWindowThemeAttribute = NULL;
		zIsThemeActive = NULL;

		m_ThemeLibLoaded = FALSE;
	}

	// Aero
	hModAero = LoadLibrary(_T("DWMAPI.DLL"));
	if (hModAero)
	{
		zDwmIsCompositionEnabled = (PFNDWMISCOMPOSITIONENABLED)GetProcAddress(hModAero, "DwmIsCompositionEnabled");
		zDwmExtendFrameIntoClientArea = (PFNDWMEXTENDFRAMEINTOCLIENTAREA)GetProcAddress(hModAero, "DwmExtendFrameIntoClientArea");
		zDwmDefWindowProc = (PFNDWMDEFWINDOWPROC)GetProcAddress(hModAero, "DwmDefWindowProc");

		m_AeroLibLoaded = (zDwmIsCompositionEnabled && zDwmExtendFrameIntoClientArea && zDwmDefWindowProc);
		if (!m_AeroLibLoaded)
		{
			FreeLibrary(hModAero);
			hModAero = NULL;
		}
	}
	else
	{
		zDwmIsCompositionEnabled = NULL;
		zDwmExtendFrameIntoClientArea = NULL;
		zDwmDefWindowProc = NULL;

		m_AeroLibLoaded = FALSE;
	}

	// Anwendungspfad
	TCHAR szPathName[MAX_PATH];
	GetModuleFileName(NULL, szPathName, MAX_PATH);
	LPTSTR pszFileName = _tcsrchr(szPathName, '\\')+1;
	*pszFileName = '\0';
	path = szPathName;

	// Rating and Priority bitmaps
	for (UINT a=0; a<=LFMaxRating; a++)
	{
		m_RatingBitmaps[a] = LoadBitmap(LFCommDlgDLL.hResource, MAKEINTRESOURCE(IDB_RATING0+a));
		m_PriorityBitmaps[a] = LoadBitmap(LFCommDlgDLL.hResource, MAKEINTRESOURCE(IDB_PRIORITY0+a));
	}

	// Fonts
	CString face = GetDefaultFontFace();
	const int base = 12;

	m_Fonts[FALSE][FALSE].CreateFont(-base, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
		face);
	m_Fonts[TRUE][FALSE].CreateFont(-(base+2), 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
		face);
	m_Fonts[FALSE][TRUE].CreateFont(-(base+2), 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
		face);
	m_Fonts[TRUE][TRUE].CreateFont(-(base+4), 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
		face);

	int sz = 8;
	LOGFONT lf;
	if (SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &lf, 0))
		sz = abs(lf.lfHeight);

	m_DefaultFont.CreateFont(-sz, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
		face);
	m_ItalicFont.CreateFont(-sz, 0, 0, 0, FW_NORMAL, 1, 0, 0, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
		face);
	m_SmallFont.CreateFont(-(sz*3/4), 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
		face);
	m_CaptionFont.CreateFont(-18, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
		face);

	// System image lists
	SHFILEINFO shfi;
	m_SystemImageListSmall.Attach((HIMAGELIST)SHGetFileInfo(_T(""), NULL, &shfi, sizeof(shfi), SHGFI_SYSICONINDEX | SHGFI_SMALLICON));
	m_SystemImageListLarge.Attach((HIMAGELIST)SHGetFileInfo(_T(""), NULL, &shfi, sizeof(shfi), SHGFI_SYSICONINDEX | SHGFI_LARGEICON));

	// Core image lists
	HINSTANCE hModIcons = LoadLibrary(_T("LFCORE.DLL"));
	if (hModIcons)
	{
		int cx = GetSystemMetrics(SM_CXSMICON);
		int cy = GetSystemMetrics(SM_CYSMICON);
		ImageList_GetIconSize(m_SystemImageListSmall, &cx, &cy);
		ExtractCoreIcons(hModIcons, cy, &m_CoreImageListSmall);

		cx = GetSystemMetrics(SM_CXICON);
		cy = GetSystemMetrics(SM_CYICON);
		ImageList_GetIconSize(m_SystemImageListLarge, &cx, &cy);
		ExtractCoreIcons(hModIcons, cy, &m_CoreImageListLarge);

		FreeLibrary(hModIcons);
	}

	// Get attribute information
	for (UINT a=0; a<LFAttrCategoryCount; a++)
		m_AttrCategories[a] = LFGetAttrCategoryName(a);

	for (UINT a=0; a<LFAttributeCount; a++)
		m_Attributes[a] = LFGetAttributeInfo(a);

	// Get context information
	for (UINT a=0; a<LFContextCount; a++)
		m_Contexts[a] = LFGetContextInfo(a);

	// Get item category information
	for (UINT a=0; a<LFItemCategoryCount; a++)
		m_ItemCategories[a] = LFGetItemCategoryInfo(a);
}

LFApplication::~LFApplication()
{
	if (hModThemes)
	{
		FreeLibrary(hModThemes);
		hModThemes = NULL;
	}

	for (UINT a=0; a<=LFMaxRating; a++)
	{
		DeleteObject(m_RatingBitmaps[a]);
		DeleteObject(m_PriorityBitmaps[a]);
	}
	for (UINT a=0; a<LFAttributeCount; a++)
		LFFreeAttributeDescriptor(m_Attributes[a]);
	for (UINT a=0; a<LFAttrCategoryCount; a++)
		delete m_AttrCategories[a];
	for (UINT a=0; a<LFContextCount; a++)
		LFFreeContextDescriptor(m_Contexts[a]);
	for (UINT a=0; a<LFItemCategoryCount; a++)
		LFFreeItemCategoryDescriptor(m_ItemCategories[a]);
}


// LFApplication-Initialisierung

BOOL LFApplication::InitInstance()
{
	// GDI+ initalisieren
	GdiplusStartupInput gdiplusStartupInput;
	GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);

	// Nachrichten registrieren
	p_MessageIDs = LFGetMessageIDs();

	// SendTo-Link erzeugen
	LFCreateSendTo();

	// InitCommonControlsEx() ist für Windows XP erforderlich, wenn ein Anwendungsmanifest
	// die Verwendung von ComCtl32.dll Version 6 oder höher zum Aktivieren
	// von visuellen Stilen angibt. Ansonsten treten beim Erstellen von Fenstern Fehler auf.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Legen Sie dies fest, um alle allgemeinen Steuerelementklassen einzubeziehen,
	// die Sie in Ihrer Anwendung verwenden möchten.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();

	InitTooltipManager();
	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL, RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

	InitShellManager();
	if (HasGUI==HasGUI_None)
		return TRUE;

	// OLE Initialisieren
	ENSURE(AfxOleInit());

	SetRegistryKey(_T("liquidFOLDERS"));
	LoadStdProfileSettings();

	if (HasGUI==HasGUI_Ribbon)
	{
		InitContextMenuManager();
		InitKeyboardManager();
	}

	m_nAppLook = GetGlobalInt(_T("ApplicationLook"), ID_VIEW_APPLOOK_OFF_2007_BLUE);
	SetApplicationLook(m_nAppLook);

	// Watchdog starten
	#ifndef _DEBUG
	HANDLE mutex = CreateMutex(NULL, FALSE, _T(LFCM_Watchdog));
	if (mutex && GetLastError()==ERROR_SUCCESS)
	{
		ReleaseMutex(mutex);
		CloseHandle(mutex);

		if (_access(path+"LFWatchdog.exe", 0)==0)
			ShellExecute(NULL, _T("open"), path+"LFWatchdog.exe", NULL, NULL, SW_SHOW);
	}
	#endif

	// Beim ersten Mal Welcome-Dialog anzeigen
	if ((LFGetStoreCount()==0) && (GetGlobalInt(_T("FirstRun"), 1)!=0))
	{
		WriteGlobalInt(_T("FirstRun"), 0);

		LFWelcomeDlg* dlg = new LFWelcomeDlg();
		dlg->DoModal();
		delete dlg;
	}

	return TRUE;
}

int LFApplication::ExitInstance()
{
	CWinAppEx::ExitInstance();
	GdiplusShutdown(m_gdiplusToken);

	if (hModThemes)
		FreeLibrary(hModThemes);
	if (hModAero)
		FreeLibrary(hModAero);

	return 0;
}

CString LFApplication::GetDefaultFontFace()
{
	return (OSVersion==OS_XP ? _T("Arial") : _T("Segoe UI"));
}

void LFApplication::SendMail(CString Subject)
{
	CString URL = _T("mailto:support@liquidfolders.net");
	if (!Subject.IsEmpty())
		URL += _T("?subject=")+Subject;
	ShellExecute(m_pActiveWnd->GetSafeHwnd(), _T("open"), URL, NULL, NULL, SW_SHOW);
}

void LFApplication::OnUpdateAppCommands(CCmdUI* pCmdUI)
{
	switch (pCmdUI->m_nID)
	{
	case ID_APP_NEWFILEDROP:
		pCmdUI->Enable(_access(path+"FileDrop.exe", 0)==0);
		break;
	case ID_APP_NEWMIGRATE:
		pCmdUI->Enable(_access(path+"Migrate.exe", 0)==0);
		break;
	case ID_APP_NEWSTOREMANAGER:
		pCmdUI->Enable(_access(path+"StoreManager.exe", 0)==0);
		break;
	case ID_APP_PURCHASE:
	case ID_APP_ENTERLICENSEKEY:
		pCmdUI->Enable(!LFIsLicensed());
		break;
	default:
		pCmdUI->Enable(TRUE);
	}
}

void LFApplication::OnAppHelp()
{
	CString url;
	ENSURE(url.LoadString(IDS_HELPURL));
	ShellExecute(m_pActiveWnd->GetSafeHwnd(), _T("open"), url, NULL, NULL, SW_SHOW);
}

void LFApplication::OnAppPurchase()
{
	CString url;
	ENSURE(url.LoadString(IDS_PURCHASEURL));
	ShellExecute(m_pActiveWnd->GetSafeHwnd(), _T("open"), url, NULL, NULL, SW_SHOW);
}

void LFApplication::OnAppEnterLicenseKey()
{
	LFLicenseDlg dlg(NULL);
	dlg.DoModal();
}

void LFApplication::OnAppSupport()
{
	SendMail();
}

void LFApplication::OnAppNewFileDrop()
{
	ShellExecute(m_pActiveWnd->GetSafeHwnd(), _T("open"), path+"FileDrop.exe", NULL, NULL, SW_SHOW);
}

void LFApplication::OnAppNewMigrate()
{
	ShellExecute(m_pActiveWnd->GetSafeHwnd(), _T("open"), path+"Migrate.exe", NULL, NULL, SW_SHOW);
}

void LFApplication::OnAppNewStoreManager()
{
	ShellExecute(m_pActiveWnd->GetSafeHwnd(), _T("open"), path+"StoreManager.exe", NULL, NULL, SW_SHOW);
}

void LFApplication::OnAppPrompt()
{
	ShellExecute(m_pActiveWnd->GetSafeHwnd(), _T("open"), _T("cmd.exe"), NULL, NULL, SW_SHOW);
}

void LFApplication::SetApplicationLook(UINT nID)
{
	if (nID!=m_nAppLook)
	{
		m_nAppLook = nID;
		WriteGlobalInt(_T("ApplicationLook"), nID);
	}

	switch (m_nAppLook)
	{
	case ID_VIEW_APPLOOK_OFF_2007_BLACK:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2007));
		CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_ObsidianBlack);
		break;
	case ID_VIEW_APPLOOK_OFF_2007_SILVER:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2007));
		CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Silver);
		break;
	case ID_VIEW_APPLOOK_OFF_2007_AQUA:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2007));
		CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Aqua);
		break;
#if (_MFC_VER>=0x1000)
	case ID_VIEW_APPLOOK_WINDOWS_7:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows7));
		break;
#endif
	default:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2007));
		CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_LunaBlue);
		break;
	}

	CDockingManager::SetDockingMode(DT_IMMEDIATE);
}

CString LFApplication::GetGlobalRegPath()
{
	CString strReg = _T("SOFTWARE\\");

	CString strRegKey = m_pszRegistryKey;
	if (!strRegKey.IsEmpty())
	{
		strReg += strRegKey;
		strReg += _T("\\");
	}

	return strReg;
}

int LFApplication::GetGlobalInt(LPCTSTR lpszEntry, int nDefault)
{
	ENSURE(lpszEntry);

	int nRet = nDefault;

	CSettingsStoreSP regSP;
	CSettingsStore& reg = regSP.Create(FALSE, TRUE);

	if (reg.Open(GetGlobalRegPath()))
		reg.Read(lpszEntry, nRet);

	return nRet;
}

CString LFApplication::GetGlobalString(LPCTSTR lpszEntry, LPCTSTR lpszDefault)
{
	ENSURE(lpszEntry);
	ENSURE(lpszDefault);

	CString strRet = lpszDefault;

	CSettingsStoreSP regSP;
	CSettingsStore& reg = regSP.Create(FALSE, TRUE);

	if (reg.Open(GetGlobalRegPath()))
		reg.Read(lpszEntry, strRet);

	return strRet;
}

BOOL LFApplication::WriteGlobalInt(LPCTSTR lpszEntry, int nValue)
{
	ENSURE(lpszEntry);

	CSettingsStoreSP regSP;
	CSettingsStore& reg = regSP.Create(FALSE, FALSE);

	if (reg.CreateKey(GetGlobalRegPath()))
		return reg.Write(lpszEntry, nValue);

	return FALSE;
}

BOOL LFApplication::WriteGlobalString(LPCTSTR lpszEntry, LPCTSTR lpszValue)
{
	ENSURE(lpszEntry);
	ENSURE(lpszValue);

	CSettingsStoreSP regSP;
	CSettingsStore& reg = regSP.Create(FALSE, FALSE);

	if (reg.CreateKey(GetGlobalRegPath()))
		return reg.Write(lpszEntry, lpszValue);

	return FALSE;
}

void LFApplication::ExtractCoreIcons(HINSTANCE hModIcons, int size, CImageList* li)
{
	li->Create(size, size, ILC_COLOR32, IDI_LastIcon, 1);

	for (UINT a=1; a<=IDI_LastIcon; a++)
	{
		HICON ic = (HICON)LoadImage(hModIcons, MAKEINTRESOURCE(a), IMAGE_ICON, size, size, LR_DEFAULTCOLOR);
		li->Add(ic);
		DestroyIcon(ic);
	}
}

UINT LFApplication::DeleteStore(LFItemDescriptor* store, CWnd* pParentWnd, CWnd* pOwnerWnd)
{
	if (!LFAskDeleteStore(store, pParentWnd ? pParentWnd->GetSafeHwnd() : NULL))
		return LFCancel;

	// Dialogbox nur zeigen, wenn der Store gemountet ist
	if (!(store->Type & LFTypeNotMounted))
	{
		LFStoreDeleteDlg dlg(pParentWnd, store->CoreAttributes.FileName);
		if (dlg.DoModal()!=IDOK)
			return LFCancel;
	}

	return LFDeleteStore(store->StoreID, pOwnerWnd ? pOwnerWnd->GetSafeHwnd() : NULL);
}

UINT LFApplication::DeleteStore(LFStoreDescriptor* store, CWnd* pParentWnd, CWnd* pOwnerWnd)
{
	if (!LFAskDeleteStore(store, pParentWnd ? pParentWnd->GetSafeHwnd() : NULL))
		return LFCancel;

	// Dialogbox nur zeigen, wenn der Store gemountet ist
	if (store->DatPath[0]!='\0')
	{
		LFStoreDeleteDlg dlg(pParentWnd, store->StoreName);
		if (dlg.DoModal()!=IDOK)
			return LFCancel;
	}

	return LFDeleteStore(store->StoreID, pOwnerWnd ? pOwnerWnd->GetSafeHwnd() : NULL);
}

void LFApplication::PlayNavigateSound()
{
	PlayRegSound(L"Apps\\Explorer\\Navigating");
}

void LFApplication::PlayWarningSound()
{
	PlayRegSound(L"Apps\\Explorer\\SecurityBand");
}

void LFApplication::PlayTrashSound()
{
	PlayRegSound(L"Apps\\Explorer\\EmptyRecycleBin");
}

BOOL LFApplication::HideFileExt()
{
	CSettingsStoreSP regSP;
	CSettingsStore& reg = regSP.Create(FALSE, TRUE);

	if (reg.Open(_T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced")))
	{
		DWORD hide;
		if (reg.Read(_T("HideFileExt"), hide))
			return hide;
	}

	return FALSE;
}

CString LFApplication::GetCommandName(UINT nID, BOOL bInsertSpace)
{
	CString tmpStr = _T("?");
	tmpStr.LoadString(nID);

	int pos = tmpStr.Find(L'\n');
	if (pos!=-1)
		tmpStr.Delete(0, pos+1);

	pos = tmpStr.Find(_T(" ("));
	if (pos!=-1)
		tmpStr.Delete(pos, tmpStr.GetLength()-pos);

	if (bInsertSpace)
	{
		pos = tmpStr.Find(L'-');
		if ((pos!=-1) && (tmpStr.Find(L' ')==-1))
			tmpStr.Insert(pos+1, L' ');
	}

	return tmpStr;
}