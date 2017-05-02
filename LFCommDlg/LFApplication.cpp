
// LFApplication.cpp: Definiert das Klassenverhalten f�r die Anwendung.
//

#include "stdafx.h"
#include "LFCommDlg.h"
#include <commoncontrols.h>
#include <io.h>
#include <mmsystem.h>


// LFApplication
//

#define GLOBALREGPATH     _T("SOFTWARE\\liquidFOLDERS\\")

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
	OSVersion = (osInfo.dwMajorVersion<6) ? OS_XP : ((osInfo.dwMajorVersion==6) && (osInfo.dwMinorVersion==0)) ? OS_Vista : OS_Seven;

	// GdiPlus
	m_SmoothingModeAntiAlias8x8 = OSVersion>=OS_Vista ? (SmoothingMode)(SmoothingModeAntiAlias+1) : SmoothingModeAntiAlias;

	// Clipboard
	CF_FILEDESCRIPTOR = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_FILEDESCRIPTOR);
	CF_FILECONTENTS = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_FILECONTENTS);
	CF_HLIQUID = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_LIQUIDFILES);

	// DLL-Hijacking verhindern
	SetDllDirectory(_T(""));

	// Messages
	p_MessageIDs = LFGetMessageIDs();
	m_TaskbarButtonCreated = RegisterWindowMessage(_T("TaskbarButtonCreated"));
	m_LicenseActivatedMsg = RegisterWindowMessage(_T("liquidFOLDERS.LicenseActivated"));
	m_SetProgressMsg = RegisterWindowMessage(_T("liquidFOLDERS.SetProgress"));
	m_WakeupMsg = RegisterWindowMessage(_T("liquidFOLDERS.NewWindow"));

	// Themes
	hModThemes = LoadLibrary(_T("UXTHEME.DLL"));
	if (hModThemes)
	{
		zSetWindowTheme = (PFNSETWINDOWTHEME)GetProcAddress(hModThemes, "SetWindowTheme");
		zOpenThemeData = (PFNOPENTHEMEDATA)GetProcAddress(hModThemes, "OpenThemeData");
		zCloseThemeData = (PFNCLOSETHEMEDATA)GetProcAddress(hModThemes, "CloseThemeData");
		zDrawThemeBackground = (PFNDRAWTHEMEBACKGROUND)GetProcAddress(hModThemes, "DrawThemeBackground");
		zGetThemePartSize = (PFNGETTHEMEPARTSIZE)GetProcAddress(hModThemes, "GetThemePartSize");
		zIsAppThemed = (PFNISAPPTHEMED)GetProcAddress(hModThemes, "IsAppThemed");

		m_ThemeLibLoaded = (zOpenThemeData && zCloseThemeData && zDrawThemeBackground && zGetThemePartSize && zIsAppThemed);
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
		zGetThemePartSize = NULL;
		zIsAppThemed = NULL;

		m_ThemeLibLoaded = FALSE;
	}

	// DWM
	hModDwm = LoadLibrary(_T("DWMAPI.DLL"));
	if (hModDwm)
	{
		zDwmIsCompositionEnabled = (PFNDWMISCOMPOSITIONENABLED)GetProcAddress(hModDwm, "DwmIsCompositionEnabled");
		zDwmSetWindowAttribute = (PFNDWMSETWINDOWATTRIBUTE)GetProcAddress(hModDwm, "DwmSetWindowAttribute");

		m_DwmLibLoaded = (zDwmIsCompositionEnabled && zDwmSetWindowAttribute);
		if (!m_DwmLibLoaded)
		{
			FreeLibrary(hModDwm);
			hModDwm = NULL;
		}
	}
	else
	{
		zDwmIsCompositionEnabled = NULL;
		zDwmSetWindowAttribute = NULL;

		m_DwmLibLoaded = FALSE;
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

	// User
	hModUser = LoadLibrary(_T("USER32.DLL"));
	if (hModUser)
	{
		zChangeWindowMessageFilter = (PFNCHANGEWINDOWMESSAGEFILTER)GetProcAddress(hModKernel, "ChangeWindowMessageFilter");

		m_UserLibLoaded = (zChangeWindowMessageFilter!=NULL);
		if (!m_UserLibLoaded)
		{
			FreeLibrary(hModUser);
			hModUser = NULL;
		}
	}
	else
	{
		zChangeWindowMessageFilter = NULL;

		m_UserLibLoaded = FALSE;
	}

	// System image lists
	IImageList* il;
	if (SUCCEEDED(SHGetImageList(SHIL_SYSSMALL, IID_IImageList, (void**)&il)))
		m_SystemImageListSmall.Attach((HIMAGELIST)il);

	if (SUCCEEDED(SHGetImageList(SHIL_EXTRALARGE, IID_IImageList, (void**)&il)))
		m_SystemImageListExtraLarge.Attach((HIMAGELIST)il);

	if (OSVersion>=OS_Vista)
		if (SUCCEEDED(SHGetImageList(SHIL_JUMBO, IID_IImageList, (void**)&il)))
			m_SystemImageListJumbo.Attach((HIMAGELIST)il);

	// Core image lists
	HMODULE hModIcons = GetModuleHandle(_T("LFCORE"));
	if (hModIcons)
	{
		ImageList_GetIconSize(m_SystemImageListExtraLarge, &m_ExtraLargeIconSize, &m_ExtraLargeIconSize);

		ExtractCoreIcons(hModIcons, GetSystemMetrics(SM_CYSMICON), &m_CoreImageListSmall);
		ExtractCoreIcons(hModIcons, m_ExtraLargeIconSize, &m_CoreImageListExtraLarge);
		ExtractCoreIcons(hModIcons, 128, &m_CoreImageListJumbo);
	}

	// Get attribute category names
	for (UINT a=0; a<LFAttrCategoryCount; a++)
		LFGetAttrCategoryName(m_AttrCategoryNames[a], 256, a);

	// Get sorted attribute list
	LFGetSortedAttributeList(m_SortedAttributeList);

	// Get attribute information
	for (UINT a=0; a<LFAttributeCount; a++)
		LFGetAttributeInfo(m_Attributes[a], a);

	// Get context information
	for (UINT a=0; a<LFContextCount; a++)
		LFGetContextInfo(m_Contexts[a], a);

	// Get Item category information
	for (UINT a=0; a<LFItemCategoryCount; a++)
		LFGetItemCategoryInfo(m_ItemCategories[a], a);

	// Get data source information
	for (UINT a=0; a<LFSourceCount; a++)
		for (UINT b=0; b<2; b++)
			LFGetSourceName(m_SourceNames[a][b], 256, a, b==1);
}

LFApplication::~LFApplication()
{
	if (hModThemes)
		FreeLibrary(hModThemes);

	if (hModDwm)
		FreeLibrary(hModDwm);

	if (hModShell)
		FreeLibrary(hModShell);

	if (hModKernel)
		FreeLibrary(hModKernel);

	if (hModUser)
		FreeLibrary(hModUser);

	if (hFontLetterGothic)
		RemoveFontMemResourceEx(hFontLetterGothic);
}


BOOL LFApplication::InitInstance()
{
	// GDI+ initalisieren
	GdiplusStartupInput gdiplusStartupInput;
	GdiplusStartup(&m_GdiPlusToken, &gdiplusStartupInput, NULL);

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

	// OLE initialisieren
	ENSURE(AfxOleInit());

	// Rating and priority bitmaps
	for (UINT a=0; a<=LFMaxRating; a++)
	{
		hRatingBitmaps[a] = LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_RATING0+a));
		hPriorityBitmaps[a] = LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_PRIORITY0+a));
	}

	// Eingebettete Schrift
	hFontLetterGothic = LoadFontFromResource(IDF_LETTERGOTHIC);

	// Fonts
	INT Size = 11;
	LOGFONT LogFont;
	if (SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &LogFont, 0))
		Size = max(abs(LogFont.lfHeight), 11);

	m_DefaultFont.CreateFont(-Size);
	m_ItalicFont.CreateFont(-Size, CLEARTYPE_QUALITY, FW_NORMAL, 1);
	m_SmallFont.CreateFont(-(Size*2/3+3), CLEARTYPE_QUALITY, FW_NORMAL, 0, _T("Segoe UI"));
	m_SmallBoldFont.CreateFont(-(Size*2/3+3), CLEARTYPE_QUALITY, FW_BOLD, 0, _T("Segoe UI"));
	m_LargeFont.CreateFont(-Size*7/6);
	m_CaptionFont.CreateFont(-Size*2, ANTIALIASED_QUALITY, FW_NORMAL, 0, _T("Letter Gothic"));
	m_UACFont.CreateFont(-Size*3/2);

	CFont* pDialogFont = CFont::FromHandle((HFONT)GetStockObject(DEFAULT_GUI_FONT));
	ASSERT_VALID(pDialogFont);

	pDialogFont->GetLogFont(&LogFont);
	LogFont.lfItalic = 0;
	LogFont.lfWeight = FW_NORMAL;

	m_DialogFont.CreateFontIndirect(&LogFont);

	LogFont.lfWeight = FW_BOLD;
	m_DialogBoldFont.CreateFontIndirect(&LogFont);

	LogFont.lfItalic = 1;
	LogFont.lfWeight = FW_NORMAL;
	m_DialogItalicFont.CreateFontIndirect(&LogFont);

	// liquidFOLDERS initalisieren
	LFInitialize();

	// SendTo-Link erzeugen
	WCHAR Path[MAX_PATH];
	if (SHGetSpecialFolderPath(NULL, Path, CSIDL_SENDTO, TRUE))
	{
		wcscat_s(Path, MAX_PATH, L"\\liquidFOLDERS.LFSendTo");

		HANDLE hFile = CreateFile(Path, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile!=INVALID_HANDLE_VALUE)
			CloseHandle(hFile);
	}

	// Registry
	SetRegistryKey(_T(""));

	// Tooltip
	m_wndTooltip.Create();

	return TRUE;
}

CWnd* LFApplication::OpenCommandLine(LPWSTR /*CmdLine*/)
{
	return NULL;
}

INT LFApplication::ExitInstance()
{
	m_wndTooltip.DestroyWindow();

	for (UINT a=0; a<m_ResourceCache.m_ItemCount; a++)
		delete m_ResourceCache[a].pImage;

	GdiplusShutdown(m_GdiPlusToken);

	for (UINT a=0; a<=LFMaxRating; a++)
	{
		DeleteObject(hRatingBitmaps[a]);
		DeleteObject(hPriorityBitmaps[a]);
	}

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

void LFApplication::SendMail(const CString& Subject) const
{
	CString URL = _T("mailto:support@liquidfolders.net");
	if (!Subject.IsEmpty())
		URL += _T("?subject=")+Subject;

	ShellExecute(m_pActiveWnd->GetSafeHwnd(), _T("open"), URL, NULL, NULL, SW_SHOWNORMAL);
}


INT LFApplication::GetGlobalInt(LPCTSTR lpszEntry, INT nDefault) const
{
	ENSURE(lpszEntry);

	INT nRet = nDefault;

	CSettingsStoreSP regSP;
	CSettingsStore& reg = regSP.Create(FALSE, TRUE);

	if (reg.Open(GLOBALREGPATH))
		reg.Read(lpszEntry, nRet);

	return nRet;
}

CString LFApplication::GetGlobalString(LPCTSTR lpszEntry, LPCTSTR lpszDefault) const
{
	ENSURE(lpszEntry);
	ENSURE(lpszDefault);

	CString strRet = lpszDefault;

	CSettingsStoreSP regSP;
	CSettingsStore& reg = regSP.Create(FALSE, TRUE);

	if (reg.Open(GLOBALREGPATH))
		reg.Read(lpszEntry, strRet);

	return strRet;
}

BOOL LFApplication::WriteGlobalInt(LPCTSTR lpszEntry, INT nValue) const
{
	ENSURE(lpszEntry);

	CSettingsStoreSP regSP;
	CSettingsStore& reg = regSP.Create(FALSE, FALSE);

	if (reg.CreateKey(GLOBALREGPATH))
		return reg.Write(lpszEntry, nValue);

	return FALSE;
}

BOOL LFApplication::WriteGlobalString(LPCTSTR lpszEntry, LPCTSTR lpszValue) const
{
	ENSURE(lpszEntry);
	ENSURE(lpszValue);

	CSettingsStoreSP regSP;
	CSettingsStore& reg = regSP.Create(FALSE, FALSE);

	if (reg.CreateKey(GLOBALREGPATH))
		return reg.Write(lpszEntry, lpszValue);

	return FALSE;
}

Bitmap* LFApplication::GetResourceImage(UINT nID) const
{
	Bitmap* pBitmap = NULL;

	HRSRC hResource = FindResource(AfxGetResourceHandle(), MAKEINTRESOURCE(nID), RT_RCDATA);
	if (hResource)
	{
		HGLOBAL hMemory = LoadResource(AfxGetResourceHandle(), hResource);
		if (hMemory)
		{
			LPVOID pResourceData = LockResource(hMemory);
			if (pResourceData)
			{
				DWORD Size = SizeofResource(AfxGetResourceHandle(), hResource);
				if (Size)
				{
					IStream* pStream = SHCreateMemStream((LPBYTE)pResourceData, Size);

					pBitmap = Gdiplus::Bitmap::FromStream(pStream);

					pStream->Release();
				}
			}
		}
	}

	return pBitmap;
}

Bitmap* LFApplication::GetCachedResourceImage(UINT nID)
{
	for (UINT a=0; a<m_ResourceCache.m_ItemCount; a++)
		if (m_ResourceCache[a].nID==nID)
			return m_ResourceCache[a].pImage;

	Bitmap* pBitmap = GetResourceImage(nID);;
	if (pBitmap)
	{
		ResourceCacheItem Item;
		Item.pImage = pBitmap;
		Item.nID = nID;

		m_ResourceCache.AddItem(Item);
	}

	return pBitmap;
}

HICON LFApplication::LoadDialogIcon(UINT nID)
{
	return (HICON)LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(nID), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_SHARED);
}

HANDLE LFApplication::LoadFontFromResource(UINT nID)
{
	HANDLE hFont = NULL;

	HRSRC hResource = FindResource(AfxGetResourceHandle(), MAKEINTRESOURCE(nID), RT_RCDATA);
	if (hResource)
	{
		HGLOBAL hMemory = LoadResource(AfxGetResourceHandle(), hResource);
		if (hMemory)
		{
			LPVOID pResourceData = LockResource(hMemory);
			if (pResourceData)
			{
				DWORD Size = SizeofResource(AfxGetResourceHandle(), hResource);
				if (Size)
				{
					DWORD nFonts;
					hFont = AddFontMemResourceEx(pResourceData, Size, NULL, &nFonts);
				}
			}
		}
	}

	return hFont;
}

void LFApplication::ExtractCoreIcons(HINSTANCE hModIcons, INT Size, CImageList* pImageList)
{
	ASSERT(pImageList);

	pImageList->Create(Size, Size, ILC_COLOR32 | ILC_MASK, IDI_LASTICON, 1);

	for (UINT a=1; a<=IDI_LASTICON; a++)
	{
		HICON hIcon = (HICON)LoadImage(hModIcons, MAKEINTRESOURCE(a), IMAGE_ICON, Size, Size, LR_DEFAULTCOLOR);
		pImageList->Add(hIcon);
		DestroyIcon(hIcon);
	}

	pImageList->SetOverlayImage(IDI_OVR_ERROR-1, 1);
	pImageList->SetOverlayImage(IDI_OVR_DEFAULT-1, 2);
	pImageList->SetOverlayImage(IDI_OVR_NEW-1, 3);
	pImageList->SetOverlayImage(IDI_OVR_EMPTY-1, 4);
}

void LFApplication::AttributeToString(CString& Name, CString& Value, LFItemDescriptor* pItemDescriptor, UINT Attr) const
{
	ASSERT(pItemDescriptor);

	Name.Empty();
	Value.Empty();

	if (m_Attributes[Attr].AttrProperties.Type==LFTypeRating)
		return;

	WCHAR tmpStr[256];
	LFAttributeToString(pItemDescriptor, Attr, tmpStr, 256);

	if (tmpStr[0])
	{
		// Decorate values
		if (m_Attributes[Attr].AttrProperties.Type==LFTypeIATACode)
		{
			WCHAR LocationName[256];
			LFIATAGetLocationNameForCode((LPCSTR)pItemDescriptor->AttributeValues[Attr], LocationName, 256);

			if (LocationName[0])
			{
				wcscat_s(tmpStr, 256, (GetThreadLocale() & 0x1FF)==LANG_ENGLISH ? L"�" : L" � ");
				wcscat_s(tmpStr, 256, LocationName);
			}
		}

		if (Attr==LFAttrDimension)
		{
			const SIZE_T Length = wcslen(tmpStr);
			swprintf_s(&tmpStr[Length], 256-Length, L" (%u�%u)", (UINT)*((UINT*)pItemDescriptor->AttributeValues[LFAttrWidth]), (UINT)*((UINT*)pItemDescriptor->AttributeValues[LFAttrHeight]));
		}

		// Copy to buffer
		if ((Attr!=LFAttrComments) && (Attr!=LFAttrFileFormat) && (Attr!=LFAttrDescription))
			Name = m_Attributes[Attr].Name;

		Value = tmpStr;
	}
}

void LFApplication::AppendAttribute(CString& Str, LFItemDescriptor* pItemDescriptor, UINT Attr) const
{
	ASSERT(pItemDescriptor);

	CString Name;
	CString Value;
	AttributeToString(Name, Value, pItemDescriptor, Attr);

	LFTooltip::AppendAttribute(Str, Name, Value);
}

CString LFApplication::GetHintForItem(LFItemDescriptor* pItemDescriptor, LPCWSTR pFormatName) const
{
	ASSERT(pItemDescriptor);
	ASSERT(m_SortedAttributeList[0]==LFAttrFileName);

	CString Hint;
	AppendAttribute(Hint, pItemDescriptor, LFAttrComments);
	AppendAttribute(Hint, pItemDescriptor, LFAttrDescription);

	// File format
	if (pFormatName)
		if (*pFormatName)
		{
			ASSERT((pItemDescriptor->Type & LFTypeMask)==LFTypeFile);

			CString tmpStr(pFormatName);

			if ((pItemDescriptor->Type & LFTypeSourceMask)>LFTypeSourceInternal)
			{
				WCHAR strSource[256] = L" ";
				wcscpy_s(&strSource[1], 255, m_SourceNames[pItemDescriptor->Type & LFTypeSourceMask][1]);
				strSource[1] = (WCHAR)towlower(strSource[1]);

				tmpStr.Append(strSource);
			}

			LFTooltip::AppendAttribute(Hint, _T(""), tmpStr);
		}

	// Other attributes
	for (UINT a=1; a<LFAttributeCount; a++)
	{
		const UINT Attr = m_SortedAttributeList[a];

		if (m_Attributes[Attr].AttrProperties.DefaultPriority==LFMaxAttributePriority)
			break;

		AppendAttribute(Hint, pItemDescriptor, Attr);
	}

	return Hint;
}

CString LFApplication::GetHintForStore(LFStoreDescriptor* pStoreDescriptor) const
{
	ASSERT(pStoreDescriptor);

	LFItemDescriptor* pItemDescriptor = LFAllocItemDescriptorEx(pStoreDescriptor);
	CString Hint = GetHintForItem(pItemDescriptor);
	LFFreeItemDescriptor(pItemDescriptor);

	return Hint;
}

void LFApplication::ShowTooltip(CWnd* pCallerWnd, CPoint point, const CString& Caption, const CString& Hint, HICON hIcon, HBITMAP hBitmap)
{
	ASSERT(IsWindow(m_wndTooltip));
	ASSERT(pCallerWnd);

	pCallerWnd->ClientToScreen(&point);
	m_wndTooltip.ShowTooltip(point, Caption, Hint, hIcon, hBitmap);
}

void LFApplication::ShowTooltip(CWnd* pCallerWnd, CPoint point, LFStoreDescriptor* pStoreDescriptor)
{
	LFItemDescriptor* pItemDescriptor = LFAllocItemDescriptorEx(pStoreDescriptor);

	ShowTooltip(pCallerWnd, point, pItemDescriptor->CoreAttributes.FileName, GetHintForItem(pItemDescriptor));

	LFFreeItemDescriptor(pItemDescriptor);
}


void LFApplication::ExecuteExplorerContextMenu(CHAR cVolume, LPCSTR Verb)
{
	// Sicherheitspr�fung
	if (strcmp(Verb, "format")==0)
		if (LFStoresOnVolume(cVolume))
		{
			CString Caption;
			Caption.Format(IDS_FORMAT_CAPTION, cVolume);

			LFMessageBox(m_pMainWnd, CString((LPCSTR)IDS_FORMAT_MSG), Caption, MB_ICONWARNING);

			return;
		}

	// Ausf�hren
	WCHAR Path[4] = L" :\\";
	Path[0] = cVolume;

	LPITEMIDLIST pidlFQ = SHSimpleIDListFromPath(Path);
	LPCITEMIDLIST pidlRel = NULL;

	IShellFolder* pParentFolder = NULL;
	if (FAILED(SHBindToParent(pidlFQ, IID_IShellFolder, (void**)&pParentFolder, &pidlRel)))
		return;

	IContextMenu* pcm = NULL;
	if (SUCCEEDED(pParentFolder->GetUIObjectOf(m_pMainWnd->GetSafeHwnd(), 1, &pidlRel, IID_IContextMenu, NULL, (void**)&pcm)))
	{
		HMENU hPopup = CreatePopupMenu();
		if (hPopup)
		{
			UINT uFlags = CMF_NORMAL | CMF_EXPLORE;
			if (SUCCEEDED(pcm->QueryContextMenu(hPopup, 0, 1, 0x6FFF, uFlags)))
			{
				CWaitCursor csr;

				CMINVOKECOMMANDINFO cmi;
				cmi.cbSize = sizeof(CMINVOKECOMMANDINFO);
				cmi.fMask = 0;
				cmi.hwnd = m_pMainWnd->GetSafeHwnd();
				cmi.lpVerb = Verb;
				cmi.lpParameters = NULL;
				cmi.lpDirectory = NULL;
				cmi.nShow = SW_SHOWNORMAL;
				cmi.dwHotKey = 0;
				cmi.hIcon = NULL;

				pcm->InvokeCommand(&cmi);
			}
		}
	}
}


void LFApplication::PlayRegSound(const CString& Identifier)
{
	CString strKey;
	strKey.Format(_T("AppEvents\\Schemes\\%s\\.Current"), Identifier);

	CSettingsStoreSP regSP;
	CSettingsStore& reg = regSP.Create(FALSE, TRUE);

	if (reg.Open(strKey))
	{
		CString strFile;

		if (reg.Read(_T(""), strFile))
			if (!strFile.IsEmpty())
				PlaySound(strFile, NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT | SND_NOWAIT);
	}
}

void LFApplication::PlayAsteriskSound()
{
	PlayRegSound(_T("Apps\\.Default\\SystemAsterisk"));
}

void LFApplication::PlayDefaultSound()
{
	PlayRegSound(_T("Apps\\.Default\\.Default"));
}

void LFApplication::PlayErrorSound()
{
	PlayRegSound(_T("Apps\\.Default\\SystemHand"));
}

void LFApplication::PlayNavigateSound()
{
	PlayRegSound(_T("Apps\\Explorer\\Navigating"));
}

void LFApplication::PlayNotificationSound()
{
	PlayRegSound(_T("Apps\\Explorer\\SecurityBand"));
}

void LFApplication::PlayQuestionSound()
{
	PlayRegSound(_T("Apps\\.Default\\SystemQuestion"));
}

void LFApplication::PlayTrashSound()
{
	PlayRegSound(_T("Apps\\Explorer\\EmptyRecycleBin"));
}

void LFApplication::PlayWarningSound()
{
	PlayRegSound(_T("Apps\\.Default\\SystemExclamation"));
}


void LFApplication::GetUpdateSettings(BOOL& EnableAutoUpdate, INT& Interval) const
{
	EnableAutoUpdate = GetGlobalInt(_T("EnableAutoUpdate"), 1)!=0;
	Interval = GetGlobalInt(_T("UpdateCheckInterval"), 0);
}

void LFApplication::SetUpdateSettings(BOOL EnableAutoUpdate, INT Interval) const
{
	WriteGlobalInt(_T("EnableAutoUpdate"), EnableAutoUpdate);
	WriteGlobalInt(_T("UpdateCheckInterval"), Interval);
}

BOOL LFApplication::IsUpdateCheckDue() const
{
	BOOL EnableAutoUpdate;
	INT Interval;
	GetUpdateSettings(EnableAutoUpdate, Interval);

	if (EnableAutoUpdate && (Interval>=0) && (Interval<=2))
	{
		FILETIME ft;
		GetSystemTimeAsFileTime(&ft);

		ULARGE_INTEGER LastUpdateCheck;
		LastUpdateCheck.HighPart = GetGlobalInt(_T("LastUpdateCheckHigh"), 0);
		LastUpdateCheck.LowPart = GetGlobalInt(_T("LastUpdateCheckLow"), 0);

		ULARGE_INTEGER Now;
		Now.HighPart = ft.dwHighDateTime;
		Now.LowPart = ft.dwLowDateTime;

#define SECOND (10000000ull)
#define MINUTE (60ull*SECOND)
#define HOUR   (60ull*MINUTE)
#define DAY    (24ull*HOUR)

		ULARGE_INTEGER NextUpdateCheck = LastUpdateCheck;
		NextUpdateCheck.QuadPart += 10ull*SECOND;

		switch (Interval)
		{
		case 0:
			NextUpdateCheck.QuadPart += DAY;
			break;

		case 1:
			NextUpdateCheck.QuadPart += 7ull*DAY;
			break;

		case 2:
			NextUpdateCheck.QuadPart += 30ull*DAY;
			break;
		}

		if ((Now.QuadPart>=NextUpdateCheck.QuadPart) || (Now.QuadPart<LastUpdateCheck.QuadPart))
		{
			WriteGlobalInt(_T("LastUpdateCheckHigh"), Now.HighPart);
			WriteGlobalInt(_T("LastUpdateCheckLow"), Now.LowPart);

			return TRUE;
		}
	}

	return FALSE;
}

void LFApplication::GetBinary(LPCTSTR lpszEntry, void* pData, UINT Size)
{
	UINT Bytes;
	LPBYTE pBuffer = NULL;
	CWinAppEx::GetBinary(lpszEntry, &pBuffer, &Bytes);

	if (pBuffer)
	{
		memcpy_s(pData, Size, pBuffer, min(Size, Bytes));
		free(pBuffer);
	}
}


BEGIN_MESSAGE_MAP(LFApplication, CWinAppEx)
	ON_COMMAND(IDM_BACKSTAGE_PURCHASE, OnBackstagePurchase)
	ON_COMMAND(IDM_BACKSTAGE_ENTERLICENSEKEY, OnBackstageEnterLicenseKey)
	ON_COMMAND(IDM_BACKSTAGE_SUPPORT, OnBackstageSupport)
	ON_COMMAND(IDM_BACKSTAGE_ABOUT, OnBackstageAbout)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_BACKSTAGE_PURCHASE, IDM_BACKSTAGE_ABOUT, OnUpdateBackstageCommands)
END_MESSAGE_MAP()

void LFApplication::OnBackstagePurchase()
{
	CString URL((LPCSTR)IDS_PURCHASEURL);

	ShellExecute(m_pActiveWnd->GetSafeHwnd(), _T("open"), URL, NULL, NULL, SW_SHOWNORMAL);
}

void LFApplication::OnBackstageEnterLicenseKey()
{
	LFLicenseDlg dlg(m_pActiveWnd);
	dlg.DoModal();
}

void LFApplication::OnBackstageSupport()
{
	SendMail();
}

void LFApplication::OnBackstageAbout()
{
	LFAboutDlg dlg(m_pActiveWnd);
	dlg.DoModal();
}

void LFApplication::OnUpdateBackstageCommands(CCmdUI* pCmdUI)
{
	switch (pCmdUI->m_nID)
	{
	case IDM_BACKSTAGE_PURCHASE:
	case IDM_BACKSTAGE_ENTERLICENSEKEY:
		pCmdUI->Enable(!LFIsLicensed());
		break;

	default:
		pCmdUI->Enable(TRUE);
	}
}
