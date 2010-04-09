
// LFApplication.cpp: Definiert das Klassenverhalten für die Anwendung.
//

#include "stdafx.h"
#include "afxwinappex.h"
#include "LFCore.h"
#include "LFCommDlg.h"
#include <io.h>
#include <mmsystem.h>


// LFApplication

BEGIN_MESSAGE_MAP(LFApplication, CWinAppEx)
	ON_COMMAND(ID_APP_HELP, OnAppHelp)
	ON_COMMAND(ID_APP_PURCHASE, OnAppPurchase)
	ON_COMMAND(ID_APP_ENTERLICENSEKEY, OnAppEnterLicenseKey)
	ON_COMMAND(ID_APP_SUPPORT, OnAppSupport)
	ON_COMMAND(ID_APP_NEWFILEDROP, OnAppNewFileDrop)
	ON_COMMAND(ID_APP_NEWMIGRATE, OnAppNewMigrate)
	ON_COMMAND(ID_APP_NEWSTOREMANAGER, OnAppNewStoreManager)
	ON_COMMAND(ID_APP_PROMPT, OnAppPrompt)
END_MESSAGE_MAP()

extern AFX_EXTENSION_MODULE LFCommDlgDLL;

void PlayRegSound(CString Identifier)
{
	CString strFile;
	CString strKey = L"AppEvents\\Schemes\\";
	strKey += Identifier;
	strKey += L"\\.current";

	CSettingsStoreSP regSP;
	CSettingsStore& reg = regSP.Create(FALSE, TRUE);

	if (reg.Open(strKey))
		if (reg.Read(L"", strFile))
			if (!strFile.IsEmpty())
				PlaySound(strFile, NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT | SND_NOWAIT);
}


// LFApplication-Erstellung

LFApplication::LFApplication()
{
	zSetWindowTheme = NULL;

	// Version
	ZeroMemory(&osInfo, sizeof(OSVERSIONINFO));
	osInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osInfo);

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
		zGetThemeSysFont = (PFNGETTHEMESYSFONT)GetProcAddress(hModThemes, "GetThemeSysFont");
		zGetThemeSysColor = (PFNGETTHEMESYSCOLOR)GetProcAddress(hModThemes, "GetThemeSysColor");

		m_ThemeLibLoaded = (zOpenThemeData && zCloseThemeData && zDrawThemeBackground && zDrawThemeText && zGetThemeSysFont && zGetThemeSysColor);
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
		zGetThemeSysFont = NULL;
		zGetThemeSysColor = NULL;

		m_ThemeLibLoaded = FALSE;
	}

	// Anwendungspfad
	TCHAR szPathName[MAX_PATH];
	::GetModuleFileName(NULL, szPathName, MAX_PATH);
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

	m_Fonts[FALSE][FALSE].CreateFont(-base, 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
		face);
	m_Fonts[TRUE][FALSE].CreateFont(-(base+2), 0, 0, 0, FW_BOLD, 0, 0, 0, ANSI_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
		face);
	m_Fonts[FALSE][TRUE].CreateFont(-(base+2), 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
		face);
	m_Fonts[TRUE][TRUE].CreateFont(-(base+4), 0, 0, 0, FW_BOLD, 0, 0, 0, ANSI_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
		face);

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


// Versionsinfo

OSVERSIONINFO osInfo;


// LFApplication-Initialisierung

BOOL LFApplication::InitInstance()
{
	// GDI+ initalisieren
	GdiplusStartupInput gdiplusStartupInput;
	GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);

	// Nachrichten registrieren
	p_MessageIDs = LFGetMessageIDs();

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

	SetRegistryKey(_T("liquidFOLDERS"));
	LoadStdProfileSettings();

	InitContextMenuManager();
	InitShellManager();

	InitKeyboardManager();

	InitTooltipManager();
	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL, RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2007));
	CDockingManager::SetDockingMode(DT_IMMEDIATE);
	m_nAppLook = GetGlobalInt(_T("ApplicationLook"), ID_VIEW_APPLOOK_OFF_2007_BLUE);
	SetApplicationLook(m_nAppLook);

	// Watchdog starten
	HANDLE mutex = CreateMutex(NULL, FALSE, _T(LFCM_Watchdog));
	if (mutex && GetLastError()==ERROR_SUCCESS)
	{
		ReleaseMutex(mutex);
		CloseHandle(mutex);

		if (_access(path+"LFWatchdog.exe", 0)==0)
			ShellExecute(NULL, _T("open"), path+"LFWatchdog.exe", NULL, NULL, SW_SHOW);
	}

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
	return 0;
}

CString LFApplication::GetDefaultFontFace()
{
	return (osInfo.dwMajorVersion==5 ? L"Arial" : L"Segoe UI");
}

void LFApplication::GetBackgroundColors(UINT Background, COLORREF* back, COLORREF* text, COLORREF* highlight)
{
	switch (Background)
	{
	case ChildBackground_Ribbon:
		switch (CMFCVisualManagerOffice2007::GetStyle())
		{
		case CMFCVisualManagerOffice2007::Office2007_Silver:
			*back = (COLORREF)0xDDD4D0;
			if (text)
				*text = (COLORREF)0x000000;
			if (highlight)
				*highlight = (COLORREF)0x5C534C;
			break;
		case CMFCVisualManagerOffice2007::Office2007_ObsidianBlack:
			*back = (COLORREF)0x535353;
			if (text)
				*text = (COLORREF)0xFFFFFF;
			if (highlight)
				*highlight = (COLORREF)0xFFFFFF;
			break;
		case CMFCVisualManagerOffice2007::Office2007_Aqua:
			*back = (COLORREF)0xD9CAC4;
			if (text)
				*text = (COLORREF)0x000000;
			if (highlight)
				*highlight = (COLORREF)0x6E1500;
			break;
		default:
			*back = (COLORREF)0xFFDBBF;
			if (text)
				*text = (COLORREF)0x000000;
			if (highlight)
				*highlight = (COLORREF)0x8B4215;
		}
		break;
	case ChildBackground_Black:
		*back = (COLORREF)0x000000;
		if (text)
			*text = (COLORREF)0xFFFFFF;
		if (highlight)
			*highlight = (COLORREF)0xCCFFFF;
		break;
	case ChildBackground_White:
		*back = (COLORREF)0xFFFFFF;
		if (text)
			*text = (COLORREF)0x000000;
		if (highlight)
			*highlight = (COLORREF)0x800000;
		break;
	default:
		*back = GetSysColor(COLOR_WINDOW);
		if (text)
			*text = GetSysColor(COLOR_WINDOWTEXT);
		if (highlight)
			*highlight = GetSysColor(COLOR_HOTLIGHT);
	}
}

CString LFApplication::GetCommandName(UINT nID)
{
	CString tmpStr = _T("?");
	tmpStr.LoadString(nID);

	int pos = tmpStr.Find('\n');
	if (pos!=-1)
		tmpStr.Delete(0, pos+1);
	pos = tmpStr.Find(_T(" ("));
	if (pos!=-1)
		tmpStr.Delete(pos, tmpStr.GetLength()-pos);

	return tmpStr;
}

CMFCRibbonButton* LFApplication::CommandButton(UINT nID, int nSmallImageIndex, int nLargeImageIndex, BOOL bAlwaysShowDescription)
{
	return new CMFCRibbonButton(nID, GetCommandName(nID), nSmallImageIndex, nLargeImageIndex, bAlwaysShowDescription);
}

CMFCRibbonCheckBox* LFApplication::CommandCheckBox(UINT nID)
{
	return new CMFCRibbonCheckBox(nID, GetCommandName(nID));
}

void LFApplication::SendMail(CString Subject)
{
	CString URL("mailto:support@liquidFOLDERS.net");
	if (Subject!="")
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
		CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_ObsidianBlack);
		break;

	case ID_VIEW_APPLOOK_OFF_2007_SILVER:
		CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Silver);
		break;

	case ID_VIEW_APPLOOK_OFF_2007_AQUA:
		CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Aqua);
		break;

	default:
		CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_LunaBlue);
		break;
	}
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
		HICON ic = (HICON)LoadImage(hModIcons, MAKEINTRESOURCE(a), IMAGE_ICON, size, size, LR_LOADTRANSPARENT);
		li->Add(ic);
		DestroyIcon(ic);
	}
}

UINT LFApplication::DeleteStore(LFItemDescriptor* store, CWnd* pParentWnd)
{
	HWND hWnd = (pParentWnd ? pParentWnd->GetSafeHwnd() : NULL);

	if (!LFAskDeleteStore(store, hWnd))
		return LFCancel;

	// Dialogbox nur zeigen, wenn der Store gemountet ist
	if (!(store->Type & LFTypeNotMounted))
	{
		LFStoreDeleteDlg dlg(pParentWnd, store->CoreAttributes.FileName);
		if (dlg.DoModal()!=IDOK)
			return LFCancel;
	}

	return LFDeleteStore((char*)store->CoreAttributes.StoreID, hWnd);
}

void LFApplication::PlayNavigateSound()
{
	PlayRegSound(L"Apps\\Explorer\\Navigating");
}

void LFApplication::PlayWarningSound()
{
	PlayRegSound(L"Apps\\Explorer\\SecurityBand");
}
