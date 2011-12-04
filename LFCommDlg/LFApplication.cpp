
// LFApplication.cpp: Definiert das Klassenverhalten f�r die Anwendung.
//

#include "stdafx.h"
#include "afxwinappex.h"
#include "LFCore.h"
#include "LFCommDlg.h"
#include <commoncontrols.h>
#include <io.h>
#include <mmsystem.h>


// Thread workers
//

struct WorkerParameters
{
	LFWorkerParameters Hdr;
	CHAR StoreID[LFKeySize];
	HWND hWndSource;
	UINT Result;
};

DWORD WINAPI WorkerDeleteStore(void* lParam)
{
	LF_WORKERTHREAD_START(lParam);

	wp->Result = LFDeleteStore(wp->StoreID, wp->hWndSource, &p);

	LF_WORKERTHREAD_FINISH();
}


// LFApplication
//

#define ResetNagCounter     m_NagCounter = 0;

BEGIN_MESSAGE_MAP(LFApplication, CWinAppEx)
	ON_COMMAND(ID_APP_SUPPORT, OnAppSupport)
	ON_COMMAND(ID_APP_NEWFILEDROP, OnAppNewFileDrop)
	ON_COMMAND(ID_APP_NEWMIGRATE, OnAppNewMigrate)
	ON_COMMAND(ID_APP_NEWSTOREMANAGER, OnAppNewStoreManager)
	ON_COMMAND(ID_APP_PURCHASE, OnAppPurchase)
	ON_COMMAND(ID_APP_ENTERLICENSEKEY, OnAppEnterLicenseKey)
	ON_UPDATE_COMMAND_UI_RANGE(ID_APP_SUPPORT, ID_APP_ENTERLICENSEKEY, OnUpdateAppCommands)
	ON_UPDATE_COMMAND_UI(ID_APP_ABOUT, OnUpdateAppCommands)
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

LFApplication::LFApplication(BOOL HasGUI)
{
	m_HasGUI = HasGUI;

	// Version
	OSVERSIONINFO osInfo;
	ZeroMemory(&osInfo, sizeof(OSVERSIONINFO));
	osInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osInfo);
	OSVersion = (osInfo.dwMajorVersion<6) ? OS_XP : ((osInfo.dwMajorVersion==6) && (osInfo.dwMinorVersion==0)) ? OS_Vista : OS_Seven;

	// Clipboard
	CF_FILEDESCRIPTOR = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_FILEDESCRIPTOR);
	CF_FILECONTENTS = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_FILECONTENTS);
	CF_HLIQUID = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_LIQUIDFILES);

	// DLL-Hijacking verhindern
	SetDllDirectory(_T(""));

	// Nachrichten
	p_MessageIDs = LFGetMessageIDs();

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
		zIsAppThemed = (PFNISAPPTHEMED)GetProcAddress(hModThemes, "IsAppThemed");

		m_ThemeLibLoaded = (zOpenThemeData && zCloseThemeData && zDrawThemeBackground && zDrawThemeText && zGetThemeSysFont && zGetThemeSysColor && zGetThemePartSize && zIsAppThemed);
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
		zIsAppThemed = NULL;

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
	m_Path = szPathName;

	// Rating and Priority bitmaps
	for (UINT a=0; a<=LFMaxRating; a++)
	{
		m_RatingBitmaps[a] = LoadBitmap(LFCommDlgDLL.hResource, MAKEINTRESOURCE(IDB_RATING0+a));
		m_PriorityBitmaps[a] = LoadBitmap(LFCommDlgDLL.hResource, MAKEINTRESOURCE(IDB_PRIORITY0+a));
	}

	// Fonts
	CString face = GetDefaultFontFace();

	INT sz = 8;
	LOGFONT lf;
	if (SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &lf, 0))
		sz = abs(lf.lfHeight);

	m_DefaultFont.CreateFont(-sz, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
		face);
	m_BoldFont.CreateFont(-sz, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
		face);
	m_ItalicFont.CreateFont(-sz, 0, 0, 0, FW_NORMAL, 1, 0, 0, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
		face);
	m_SmallFont.CreateFont(-(sz-2), 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
		(sz<=11) ? _T("Tahoma") : face);
	m_LargeFont.CreateFont(-(sz+2), 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
		face);
	m_CaptionFont.CreateFont(-(sz+5), 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
		face);

	// System image lists
	IImageList* il;
	if (SUCCEEDED(SHGetImageList(SHIL_SYSSMALL, IID_IImageList, (void**)&il)))
		m_SystemImageListSmall.Attach((HIMAGELIST)il);
	if (SUCCEEDED(SHGetImageList(SHIL_LARGE, IID_IImageList, (void**)&il)))
		m_SystemImageListLarge.Attach((HIMAGELIST)il);
	if (SUCCEEDED(SHGetImageList(SHIL_EXTRALARGE, IID_IImageList, (void**)&il)))
		m_SystemImageListExtraLarge.Attach((HIMAGELIST)il);
	if (OSVersion>=OS_Vista)
		if (SUCCEEDED(SHGetImageList(SHIL_JUMBO, IID_IImageList, (void**)&il)))
			m_SystemImageListJumbo.Attach((HIMAGELIST)il);

	// Core image lists
	HMODULE hModIcons = GetModuleHandle(_T("LFCORE"));
	if (hModIcons)
	{
		INT cx = GetSystemMetrics(SM_CXSMICON);
		INT cy = GetSystemMetrics(SM_CYSMICON);
		ImageList_GetIconSize(m_SystemImageListSmall, &cx, &cy);
		ExtractCoreIcons(hModIcons, cy, &m_CoreImageListSmall);

		cx = GetSystemMetrics(SM_CXICON);
		cy = GetSystemMetrics(SM_CYICON);
		ImageList_GetIconSize(m_SystemImageListLarge, &cx, &cy);
		ExtractCoreIcons(hModIcons, cy, &m_CoreImageListLarge);

		cx = cy = 48;
		ImageList_GetIconSize(m_SystemImageListExtraLarge, &cx, &cy);
		ExtractCoreIcons(hModIcons, cy, &m_CoreImageListExtraLarge);

		ExtractCoreIcons(hModIcons, 128, &m_CoreImageListJumbo);
	}

	// liquidFOLDERS initalisieren
	LFInitialize();

	// Get attribute information
	for (UINT a=0; a<LFAttributeCount; a++)
		m_Attributes[a] = LFGetAttributeInfo(a);

	for (UINT a=0; a<LFAttrCategoryCount; a++)
		m_AttrCategories[a] = LFGetAttrCategoryName(a);

	// Get domain information
	for (UINT a=0; a<LFDomainCount; a++)
		m_Domains[a] = LFGetDomainInfo(a);

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
		FreeLibrary(hModThemes);
	if (hModAero)
		FreeLibrary(hModAero);

	for (UINT a=0; a<=LFMaxRating; a++)
	{
		DeleteObject(m_RatingBitmaps[a]);
		DeleteObject(m_PriorityBitmaps[a]);
	}
	for (UINT a=0; a<LFAttributeCount; a++)
		LFFreeAttributeDescriptor(m_Attributes[a]);
	for (UINT a=0; a<LFAttrCategoryCount; a++)
		delete m_AttrCategories[a];
	for (UINT a=0; a<LFDomainCount; a++)
		LFFreeDomainDescriptor(m_Domains[a]);
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

	// InitCommonControlsEx() ist f�r Windows XP erforderlich, wenn ein Anwendungsmanifest
	// die Verwendung von ComCtl32.dll Version 6 oder h�her zum Aktivieren
	// von visuellen Stilen angibt. Ansonsten treten beim Erstellen von Fenstern Fehler auf.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Legen Sie dies fest, um alle allgemeinen Steuerelementklassen einzubeziehen,
	// die Sie in Ihrer Anwendung verwenden m�chten.
	InitCtrls.dwICC = ICC_WIN95_CLASSES | ICC_DATE_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	if (!CWinAppEx::InitInstance())
		return FALSE;

	// OLE Initialisieren
	ENSURE(AfxOleInit());

	// SendTo-Link erzeugen
	LFCreateSendTo();

	SetRegistryKey(_T("liquidFOLDERS"));

	if (!m_HasGUI)
		return TRUE;

	// Watchdog starten
	#ifndef _DEBUG
	HANDLE hMutex = CreateMutex(NULL, FALSE, _T(LFCM_Watchdog));
	if (hMutex && GetLastError()==ERROR_SUCCESS)
	{
		ReleaseMutex(hMutex);
		CloseHandle(hMutex);

		if (_waccess(m_Path+_T("LFWatchdog.exe"), 0)==0)
			ShellExecute(NULL, _T("open"), m_Path+_T("LFWatchdog.exe"), NULL, NULL, SW_SHOW);
	}
	#endif

	// Falls abgelaufen, Fenster anzeigen
	ResetNagCounter;
	if (!LFIsLicensed())
		ShowNagScreen(NAG_NOTLICENSED | NAG_FORCE);

	// Beim ersten Mal Welcome-Dialog anzeigen
	if ((LFGetStoreCount()==0) && (GetGlobalInt(_T("FirstRun"), 1)!=0))
	{
		WriteGlobalInt(_T("FirstRun"), 0);

		LFWelcomeDlg dlg;
		dlg.DoModal();
	}

	return TRUE;
}

INT LFApplication::ExitInstance()
{
	GdiplusShutdown(m_gdiplusToken);

	if (hModThemes)
		FreeLibrary(hModThemes);
	if (hModAero)
		FreeLibrary(hModAero);

	return CWinAppEx::ExitInstance();
}

BOOL LFApplication::ShowNagScreen(UINT Level, CWnd* pWndParent, BOOL Abort)
{
	if ((Level & NAG_EXPIRED) ? LFIsSharewareExpired() : !LFIsLicensed())
		if ((Level & NAG_FORCE) || (++m_NagCounter)>5)
		{
			CString tmpStr;
			ENSURE(tmpStr.LoadString(IDS_NOLICENSE));

			MessageBox(pWndParent ? pWndParent->GetSafeHwnd() : GetForegroundWindow(), tmpStr, _T("liquidFOLDERS"), Abort ? (MB_OK | MB_ICONSTOP) : (MB_OK | MB_ICONINFORMATION));
			ResetNagCounter;

			return TRUE;
		}

	return FALSE;
}

CString LFApplication::GetDefaultFontFace()
{
	LOGFONT lf;
	SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &lf, 0);

	return lf.lfFaceName;
}

void LFApplication::SendMail(CString Subject)
{
	CString URL = _T("mailto:support@liquidfolders.net");
	if (!Subject.IsEmpty())
		URL += _T("?subject=")+Subject;

	ShellExecute(m_pActiveWnd->GetSafeHwnd(), _T("open"), URL, NULL, NULL, SW_SHOW);
}


void LFApplication::OnAppSupport()
{
	SendMail();
}

void LFApplication::OnAppNewFileDrop()
{
	ShellExecute(m_pActiveWnd->GetSafeHwnd(), _T("open"), m_Path+_T("FileDrop.exe"), NULL, NULL, SW_SHOW);
}

void LFApplication::OnAppNewMigrate()
{
	ShellExecute(m_pActiveWnd->GetSafeHwnd(), _T("open"), m_Path+_T("Migrate.exe"), NULL, NULL, SW_SHOW);
}

void LFApplication::OnAppNewStoreManager()
{
	ShellExecute(m_pActiveWnd->GetSafeHwnd(), _T("open"), m_Path+_T("StoreManager.exe"), NULL, NULL, SW_SHOW);
}

void LFApplication::OnAppPurchase()
{
	CString url;
	ENSURE(url.LoadString(IDS_PURCHASEURL));

	ShellExecute(m_pActiveWnd->GetSafeHwnd(), _T("open"), url, NULL, NULL, SW_SHOW);
}

void LFApplication::OnAppEnterLicenseKey()
{
	LFLicenseDlg dlg(m_pActiveWnd);
	dlg.DoModal();
}

void LFApplication::OnUpdateAppCommands(CCmdUI* pCmdUI)
{
	switch (pCmdUI->m_nID)
	{
	case ID_APP_NEWFILEDROP:
		pCmdUI->Enable(_waccess(m_Path+_T("FileDrop.exe"), 0)==0);
		break;
	case ID_APP_NEWMIGRATE:
		pCmdUI->Enable(_waccess(m_Path+_T("Migrate.exe"), 0)==0);
		break;
	case ID_APP_NEWSTOREMANAGER:
		pCmdUI->Enable(_waccess(m_Path+_T("StoreManager.exe"), 0)==0);
		break;
	case ID_APP_PURCHASE:
	case ID_APP_ENTERLICENSEKEY:
		pCmdUI->Enable(!LFIsLicensed());
		break;
	default:
		pCmdUI->Enable(TRUE);
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

INT LFApplication::GetGlobalInt(LPCTSTR lpszEntry, INT nDefault)
{
	ENSURE(lpszEntry);

	INT nRet = nDefault;

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

BOOL LFApplication::WriteGlobalInt(LPCTSTR lpszEntry, INT nValue)
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

void LFApplication::ExtractCoreIcons(HINSTANCE hModIcons, INT size, CImageList* li)
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

	WorkerParameters wp;
	ZeroMemory(&wp, sizeof(wp));
	strcpy_s(wp.StoreID, LFKeySize, store->StoreID);
	wp.hWndSource = pOwnerWnd ? pOwnerWnd->GetSafeHwnd() : NULL;

	LFDoWithProgress(WorkerDeleteStore, &wp.Hdr, pParentWnd);

	return wp.Result;
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

void LFApplication::PlayStandardSound()
{
	PlayRegSound(L"Apps\\.Default\\.Default");
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

BOOL LFApplication::HideEmptyDrives()
{
	CSettingsStoreSP regSP;
	CSettingsStore& reg = regSP.Create(FALSE, TRUE);

	if (reg.Open(_T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced")))
	{
		DWORD hide;
		if (reg.Read(_T("HideDrivesWithNoMedia"), hide))
			return hide;
	}

	return FALSE;
}

void LFApplication::GetUpdateSettings(BOOL* EnableAutoUpdate, INT* Interval)
{
	if (EnableAutoUpdate)
		*EnableAutoUpdate = GetGlobalInt(_T("EnableAutoUpdate"), 1)!=0;
	if (Interval)
		*Interval = GetGlobalInt(_T("UpdateInterval"), 0);
}

void LFApplication::SetUpdateSettings(BOOL EnableAutoUpdate, INT Interval)
{
	WriteGlobalInt(_T("EnableAutoUpdate"), EnableAutoUpdate);
	WriteGlobalInt(_T("UpdateInterval"), Interval);
}

BOOL LFApplication::IsUpdateCheckDue()
{
	BOOL EnableAutoUpdate;
	INT Interval;
	GetUpdateSettings(&EnableAutoUpdate, &Interval);

	if ((EnableAutoUpdate) && (Interval>=0) && (Interval<=2))
	{
		FILETIME ft;
		GetSystemTimeAsFileTime(&ft);

		ULARGE_INTEGER LastUpdate;
		LastUpdate.HighPart = GetGlobalInt(_T("LastUpdateHigh"), 0);
		LastUpdate.LowPart = GetGlobalInt(_T("LastUpdateLow"), 0);

		ULARGE_INTEGER Now;
		Now.HighPart = ft.dwHighDateTime;
		Now.LowPart = ft.dwLowDateTime;

#define SECOND ((ULONGLONG)10000000)
#define MINUTE (60*SECOND)
#define HOUR   (60*MINUTE)
#define DAY    (24*HOUR)

		switch (Interval)
		{
		case 0:
			LastUpdate.QuadPart += DAY;
			break;
		case 1:
			LastUpdate.QuadPart += 7*DAY;
			break;
		case 2:
			LastUpdate.QuadPart += 30*DAY;
			break;
		}
		LastUpdate.QuadPart += 10*SECOND;

		if (Now.QuadPart>=LastUpdate.QuadPart)
		{
			WriteGlobalInt(_T("LastUpdateHigh"), Now.HighPart);
			WriteGlobalInt(_T("LastUpdateLow"), Now.LowPart);

			return TRUE;
		}
	}

	return FALSE;
}
