
// LFApplication.cpp: Definiert das Klassenverhalten für die Anwendung.
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

extern AFX_EXTENSION_MODULE LFCommDlgDLL;

BEGIN_MESSAGE_MAP(LFApplication, CWinAppEx)
	ON_COMMAND(ID_APP_SUPPORT, OnAppSupport)
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

LFApplication::LFApplication(GUID& AppID)
{
	// ID
	m_AppID = AppID;

	// Update notification
	m_pUpdateNotification = NULL;

	// Version
	OSVERSIONINFO osInfo;
	ZeroMemory(&osInfo, sizeof(OSVERSIONINFO));
	osInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osInfo);
	OSVersion = (osInfo.dwMajorVersion<6) ? OS_XP : ((osInfo.dwMajorVersion==6) && (osInfo.dwMinorVersion==0)) ? OS_Vista : ((osInfo.dwMajorVersion==6) && (osInfo.dwMinorVersion==1)) ? OS_Seven : OS_Eight;

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

	// Shell
	hModShell = LoadLibrary(_T("SHELL32.DLL"));
	if (hModShell)
	{
		zSetCurrentProcessExplicitAppUserModelID = (PFNSETCURRENTPROCESSEXPLICITAPPUSERMODELID)GetProcAddress(hModShell, "SetCurrentProcessExplicitAppUserModelID");

		m_ShellLibLoaded = (zSetCurrentProcessExplicitAppUserModelID!=NULL);
		if (!m_ShellLibLoaded)
		{
			FreeLibrary(hModShell);
			hModShell = NULL;
		}
	}
	else
	{
		zSetCurrentProcessExplicitAppUserModelID = NULL;

		m_ShellLibLoaded = FALSE;
	}

	// Kernel
	hModKernel = LoadLibrary(_T("KERNEL32.DLL"));
	if (hModKernel)
	{
		zRegisterApplicationRestart = (PFNREGISTERAPPLICATIONRESTART)GetProcAddress(hModKernel, "RegisterApplicationRestart");

		m_KernelLibLoaded = (zRegisterApplicationRestart!=NULL);
		if (!m_KernelLibLoaded)
		{
			FreeLibrary(hModKernel);
			hModKernel = NULL;
		}
	}
	else
	{
		zRegisterApplicationRestart = NULL;

		m_KernelLibLoaded = FALSE;
	}

	// Eingebettete Schrift
	hFontLetterGothic = LoadFontFromResource(IDF_LETTERGOTHIC, LFCommDlgDLL.hResource);

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

	// Get attribute information
	for (UINT a=0; a<LFAttributeCount; a++)
		m_Attributes[a] = LFGetAttributeInfo(a);

	for (UINT a=0; a<LFAttrCategoryCount; a++)
		m_AttrCategories[a] = LFGetAttrCategoryName(a);

	// Get data source information
	for (UINT a=0; a<LFSourceCount; a++)
		for (UINT b=0; b<2; b++)
			m_SourceNames[a][b] = LFGetSourceName(a, b==1);

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
	if (hModShell)
		FreeLibrary(hModShell);
	if (hModKernel)
		FreeLibrary(hModKernel);
	if (hFontLetterGothic)
		RemoveFontMemResourceEx(hFontLetterGothic);

	for (UINT a=0; a<=LFMaxRating; a++)
	{
		DeleteObject(m_RatingBitmaps[a]);
		DeleteObject(m_PriorityBitmaps[a]);
	}
	for (UINT a=0; a<LFAttributeCount; a++)
		LFFreeAttributeDescriptor(m_Attributes[a]);
	for (UINT a=0; a<LFAttrCategoryCount; a++)
		delete m_AttrCategories[a];
	for (UINT a=0; a<LFSourceCount; a++)
		for (UINT b=0; b<2; b++)
			delete m_SourceNames[a][b];
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

	// InitCommonControlsEx() ist für Windows XP erforderlich, wenn ein Anwendungsmanifest
	// die Verwendung von ComCtl32.dll Version 6 oder höher zum Aktivieren
	// von visuellen Stilen angibt. Ansonsten treten beim Erstellen von Fenstern Fehler auf.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Legen Sie dies fest, um alle allgemeinen Steuerelementklassen einzubeziehen,
	// die Sie in Ihrer Anwendung verwenden möchten.
	InitCtrls.dwICC = ICC_WIN95_CLASSES | ICC_DATE_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	if (!CWinAppEx::InitInstance())
		return FALSE;

	// OLE Initialisieren
	ENSURE(AfxOleInit());

	// Dialog classes
	WNDCLASS wc;
	GetClassInfo(LFCommDlgDLL.hModule, _T("#32770"), &wc);
	wc.lpszClassName = _T("UpdateDlg");
	AfxRegisterClass(&wc);

	// liquidFOLDERS initalisieren
	LFInitialize();

	// SendTo-Link erzeugen
	LFCreateSendTo();

	SetRegistryKey(_T("liquidFOLDERS"));

	ResetNagCounter;

	// Beim ersten Mal Standard-Store erzeugen
	if ((LFGetStoreCount()==0) && (GetGlobalInt(_T("FirstRun"), 1)!=0))
	{
		WriteGlobalInt(_T("FirstRun"), 0);

		// Lokalen Store erstellen
		LFStoreDescriptor store;
		ZeroMemory(&store, sizeof(store));

		store.Flags = LFStoreFlagAutoLocation;
		store.IndexMode = LFStoreIndexModeInternal;

		LFErrorBox(LFCreateStore(&store));
	}

	return TRUE;
}

CWnd* LFApplication::OpenCommandLine(WCHAR* /*CmdLine*/)
{
	return NULL;
}

INT LFApplication::ExitInstance()
{
	for (POSITION p=m_ResourceCache.GetHeadPosition(); p; )
		delete m_ResourceCache.GetNext(p).pImage;

	GdiplusShutdown(m_gdiplusToken);

	if (hModThemes)
		FreeLibrary(hModThemes);
	if (hModAero)
		FreeLibrary(hModAero);

	return CWinAppEx::ExitInstance();
}

void LFApplication::AddFrame(CWnd* pFrame)
{
	m_pMainFrames.AddTail(pFrame);
	m_pMainWnd = pFrame;
	m_pActiveWnd = NULL;
}

void LFApplication::KillFrame(CWnd* pVictim)
{
	for (POSITION p=m_pMainFrames.GetHeadPosition(); p; )
	{
		POSITION pl = p;
		CWnd* pFrame = m_pMainFrames.GetNext(p);
		if (pFrame==pVictim)
		{
			m_pMainFrames.RemoveAt(pl);
		}
		else
		{
			m_pMainWnd = pFrame;
		}
	}
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

CGdiPlusBitmap* LFApplication::GetCachedResourceImage(UINT nID, LPCTSTR pType, HMODULE hInst)
{
	for (POSITION p=m_ResourceCache.GetHeadPosition(); p; )
	{
		ResourceCacheItem item = m_ResourceCache.GetNext(p);

		if (item.nResID==nID)
			return item.pImage;
	}

	ResourceCacheItem item;
	item.pImage = new CGdiPlusBitmapResource(nID, pType, hInst);
	item.nResID = nID;

	m_ResourceCache.AddTail(item);
	return item.pImage;
}

HANDLE LFApplication::LoadFontFromResource(UINT nID, HMODULE hInst)
{
	HRSRC hResource = FindResource(hInst, MAKEINTRESOURCE(nID), L"TTF");
	if (!hResource)
		return NULL;

	HGLOBAL hMemory = LoadResource(hInst, hResource);
	if (!hMemory)
		return NULL;

	LPVOID pResourceData = LockResource(hMemory);
	if (!pResourceData)
		return NULL;

	DWORD Size = SizeofResource(hInst, hResource);
	if (!Size)
		return NULL;

	DWORD nFonts;
	HANDLE res = AddFontMemResourceEx(pResourceData, Size, NULL, &nFonts);

	UnlockResource(hMemory);
	return res;
}

void LFApplication::ExtractCoreIcons(HINSTANCE hModIcons, INT size, CImageList* li)
{
	li->Create(size, size, ILC_COLOR32 | ILC_MASK, IDI_LastIcon, 1);

	for (UINT a=1; a<=IDI_LastIcon; a++)
	{
		HICON ic = (HICON)LoadImage(hModIcons, MAKEINTRESOURCE(a), IMAGE_ICON, size, size, LR_DEFAULTCOLOR);
		li->Add(ic);
		DestroyIcon(ic);
	}

	li->SetOverlayImage(IDI_OVR_Default-1, 1);
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
	PlayRegSound(_T("Apps\\.Default\\.Default"));
}

void LFApplication::PlayNavigateSound()
{
	PlayRegSound(_T("Apps\\Explorer\\Navigating"));
}

void LFApplication::PlayWarningSound()
{
	PlayRegSound(_T("Apps\\Explorer\\SecurityBand"));
}

void LFApplication::PlayTrashSound()
{
	PlayRegSound(_T("Apps\\Explorer\\EmptyRecycleBin"));
}

void LFApplication::GetUpdateSettings(BOOL* EnableAutoUpdate, INT* Interval)
{
	if (EnableAutoUpdate)
		*EnableAutoUpdate = GetGlobalInt(_T("EnableAutoUpdate"), 1)!=0;
	if (Interval)
		*Interval = GetGlobalInt(_T("UpdateCheckInterval"), 0);
}

void LFApplication::SetUpdateSettings(BOOL EnableAutoUpdate, INT Interval)
{
	WriteGlobalInt(_T("EnableAutoUpdate"), EnableAutoUpdate);
	WriteGlobalInt(_T("UpdateCheckInterval"), Interval);
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

		ULARGE_INTEGER LastUpdateCheck;
		LastUpdateCheck.HighPart = GetGlobalInt(_T("LastCheckUpdateHigh"), 0);
		LastUpdateCheck.LowPart = GetGlobalInt(_T("LastCheckUpdateLow"), 0);

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
			LastUpdateCheck.QuadPart += DAY;
			break;
		case 1:
			LastUpdateCheck.QuadPart += 7*DAY;
			break;
		case 2:
			LastUpdateCheck.QuadPart += 30*DAY;
			break;
		}
		LastUpdateCheck.QuadPart += 10*SECOND;

		if (Now.QuadPart>=LastUpdateCheck.QuadPart)
		{
			WriteGlobalInt(_T("LastUpdateCheckHigh"), Now.HighPart);
			WriteGlobalInt(_T("LastUpdateCheckLow"), Now.LowPart);

			return TRUE;
		}
	}

	return FALSE;
}

void LFApplication::GetBinary(LPCTSTR lpszEntry, void* pData, UINT size)
{
	UINT sz;
	LPBYTE buf = NULL;
	CWinAppEx::GetBinary(lpszEntry, &buf, &sz);
	if (buf)
	{
		if (sz<size)
			size = sz;
		memcpy_s(pData, size, buf, size);
		free(buf);
	}
}
