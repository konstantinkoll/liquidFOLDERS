
// LFApplication.cpp: Definiert das Klassenverhalten für die Anwendung.
//

#include "stdafx.h"
#include "LFCommDlg.h"
#include <commoncontrols.h>
#include <io.h>
#include <mmsystem.h>
#include <winhttp.h>


// LFApplication
//

#define GLOBALREGPATH     _T("SOFTWARE\\liquidFOLDERS\\")

const INT LFApplication::m_ColorDotSizes[4] = { 14, 16, 20, 25 };

LFApplication::LFApplication(const GUID& AppID)
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

	OSVersion = (osInfo.dwMajorVersion<6) ? OS_XP : (osInfo.dwMajorVersion==6) ? (osInfo.dwMinorVersion==0) ? OS_Vista : (osInfo.dwMinorVersion==1) ? OS_Seven : OS_Eight : OS_Ten;

	// GdiPlus
	m_SmoothingModeAntiAlias8x8 = (OSVersion>=OS_Vista) ? (SmoothingMode)(SmoothingModeAntiAlias+1) : SmoothingModeAntiAlias;

	// Clipboard
	CF_FILEDESCRIPTOR = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_FILEDESCRIPTOR);
	CF_FILECONTENTS = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_FILECONTENTS);
	CF_LIQUIDFILES = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_LIQUIDFILES);

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

		if (!(m_ThemeLibLoaded=(zOpenThemeData && zCloseThemeData && zDrawThemeBackground && zGetThemePartSize && zIsAppThemed)))
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

		if (!(m_DwmLibLoaded=(zDwmIsCompositionEnabled && zDwmSetWindowAttribute)))
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

		if ((m_KernelLibLoaded=(zRegisterApplicationRestart!=NULL))==FALSE)
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

	// Shell
	hModShell = LoadLibrary(_T("SHELL32.DLL"));
	if (hModShell)
	{
		zGetPropertyStoreForWindow = (PFNGETPROPERTYSTOREFORWINDOW)GetProcAddress(hModShell, "SHGetPropertyStoreForWindow");
		zSetCurrentProcessExplicitAppUserModelID = (PFNSETCURRENTPROCESSEXPLICITAPPUSERMODELID)GetProcAddress(hModShell, "SetCurrentProcessExplicitAppUserModelID");

		if (!(m_ShellLibLoaded=(zGetPropertyStoreForWindow && zSetCurrentProcessExplicitAppUserModelID)))
		{
			FreeLibrary(hModShell);
			hModShell = NULL;
		}
	}
	else
	{
		zChangeWindowMessageFilter = NULL;
		zSetCurrentProcessExplicitAppUserModelID = NULL;

		m_ShellLibLoaded = FALSE;
	}

	// User
	hModUser = LoadLibrary(_T("USER32.DLL"));
	if (hModUser)
	{
		zChangeWindowMessageFilter = (PFNCHANGEWINDOWMESSAGEFILTER)GetProcAddress(hModKernel, "ChangeWindowMessageFilter");

		if ((m_UserLibLoaded=(zChangeWindowMessageFilter!=NULL))==FALSE)
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
	IImageList* pImageList;
	if (SUCCEEDED(SHGetImageList(SHIL_SYSSMALL, IID_IImageList, (void**)&pImageList)))
		m_SystemImageListSmall.Attach((HIMAGELIST)pImageList);

	if (SUCCEEDED(SHGetImageList(SHIL_EXTRALARGE, IID_IImageList, (void**)&pImageList)))
		m_SystemImageListExtraLarge.Attach((HIMAGELIST)pImageList);

	if (OSVersion>=OS_Vista)
		if (SUCCEEDED(SHGetImageList(SHIL_JUMBO, IID_IImageList, (void**)&pImageList)))
			m_SystemImageListJumbo.Attach((HIMAGELIST)pImageList);

	// Core image lists
	const HMODULE hModIcons = GetModuleHandle(_T("LFCORE"));
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

	// Tooltip
	p_WndTooltipOwner = NULL;
}

LFApplication::~LFApplication()
{
	if (hModThemes)
		FreeLibrary(hModThemes);

	if (hModDwm)
		FreeLibrary(hModDwm);

	if (hModKernel)
		FreeLibrary(hModKernel);

	if (hModUser)
		FreeLibrary(hModUser);

	if (hModShell)
		FreeLibrary(hModShell);

	if (hFontDinMittelschrift)
		RemoveFontMemResourceEx(hFontDinMittelschrift);
}


BOOL LFApplication::InitInstance()
{
	// GDI+ initalisieren
	GdiplusStartupInput gdiplusStartupInput;
	GdiplusStartup(&m_GdiPlusToken, &gdiplusStartupInput, NULL);

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

	// OLE initialisieren
	ENSURE(AfxOleInit());

	// Rating and priority bitmaps
	for (UINT a=0; a<=LFMaxRating; a++)
	{
		hRatingBitmaps[a] = LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_RATING0+a));
		hPriorityBitmaps[a] = LoadBitmap(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_PRIORITY0+a));
	}

	// Eingebettete Schrift
	hFontDinMittelschrift = LoadFontFromResource(IDF_DINMITTELSCHRIFT);

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
	m_CaptionFont.CreateFont(-Size*2+1, ANTIALIASED_QUALITY, FW_NORMAL, 0, _T("DIN Mittelschrift"));
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

BOOL LFApplication::OpenCommandLine(LPWSTR /*CmdLine*/)
{
	return FALSE;
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


// Frame handling

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


// Dialog wrapper

void LFApplication::SendMail(const CString& Subject) const
{
	CString URL = _T("mailto:support@liquidfolders.net");
	if (!Subject.IsEmpty())
		URL += _T("?subject=")+Subject;

	ShellExecute(m_pActiveWnd->GetSafeHwnd(), _T("open"), URL, NULL, NULL, SW_SHOWNORMAL);
}


// Registry access

void LFApplication::GetBinary(LPCTSTR lpszEntry, LPVOID pData, UINT Size)
{
	UINT Bytes;
	LPBYTE pBuffer = NULL;
	CWinAppEx::GetBinary(lpszEntry, &pBuffer, &Bytes);

	if (pBuffer)
	{
		memcpy(pData, pBuffer, min(Size, Bytes));
		free(pBuffer);
	}
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


// Resource access

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

void LFApplication::LoadColorDots(CIcons& Icons, INT Size)
{
	const UINT Level = (Size>=27) ? 3 : (Size>=22) ? 2 : (Size>=18) ? 1 : 0;

	Icons.Load(IDB_COLORDOTS_14+Level, m_ColorDotSizes[Level]);
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


// Tooltips

void LFApplication::AttributeToString(CString& Name, CString& Value, const LFItemDescriptor* pItemDescriptor, UINT Attr) const
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
				wcscat_s(tmpStr, 256, (GetThreadLocale() & 0x1FF)==LANG_ENGLISH ? L"—" : L" – ");
				wcscat_s(tmpStr, 256, LocationName);
			}
		}

		if (Attr==LFAttrDimension)
			if (!LFIsNullAttribute(pItemDescriptor, LFAttrWidth) && !LFIsNullAttribute(pItemDescriptor, LFAttrHeight))
			{
				const SIZE_T Length = wcslen(tmpStr);
				swprintf_s(&tmpStr[Length], 256-Length, L" (%u×%u)", (UINT)*((UINT*)pItemDescriptor->AttributeValues[LFAttrWidth]), (UINT)*((UINT*)pItemDescriptor->AttributeValues[LFAttrHeight]));
			}

		// Copy to buffer
		if ((Attr!=LFAttrComments) && (Attr!=LFAttrFileFormat))
			Name = GetAttributeName(Attr, LFGetUserContextID(pItemDescriptor));

		Value = tmpStr;
	}
}

CString LFApplication::GetFreeBytesAvailable(INT64 FreeBytesAvailable)
{
	WCHAR tmpBuf[256];
	LFSizeToString(FreeBytesAvailable, tmpBuf, 256);

	CString tmpStr;
	tmpStr.Format(IDS_FREEBYTESAVAILABLE, tmpBuf);

	return tmpStr;
}

void LFApplication::AppendAttribute(CString& Str, const LFItemDescriptor* pItemDescriptor, UINT Attr) const
{
	ASSERT(pItemDescriptor);

	CString Name;
	CString Value;
	AttributeToString(Name, Value, pItemDescriptor, Attr);

	LFTooltip::AppendAttribute(Str, Name, Value);
}

CString LFApplication::GetHintForItem(const LFItemDescriptor* pItemDescriptor, LPCWSTR pFormatName) const
{
	ASSERT(pItemDescriptor);
	ASSERT(m_SortedAttributeList[0]==LFAttrFileName);

	CString Hint;
	AppendAttribute(Hint, pItemDescriptor, LFAttrComments);

	LFTooltip::AppendAttribute(Hint, _T(""), pItemDescriptor->Description);

	// File format
	if (pFormatName)
		if (*pFormatName)
		{
			ASSERT(LFIsFile(pItemDescriptor));

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

	// Free bytes available
	if (LFIsStore(pItemDescriptor))
		LFTooltip::AppendAttribute(Hint, _T(""), GetFreeBytesAvailable(pItemDescriptor->StoreDescriptor));

	// Other attributes
	for (UINT a=1; a<LFAttributeCount; a++)
	{
		const UINT Attr = m_SortedAttributeList[a];

		if (m_Attributes[Attr].AttrProperties.DefaultPriority==LFMinAttributePriority)
			break;

		AppendAttribute(Hint, pItemDescriptor, Attr);
	}

	return Hint;
}

CString LFApplication::GetHintForStore(const LFStoreDescriptor& StoreDescriptor) const
{
	LFItemDescriptor* pItemDescriptor = LFAllocItemDescriptorEx(StoreDescriptor);

	CString Hint = GetHintForItem(pItemDescriptor);

	LFFreeItemDescriptor(pItemDescriptor);

	return Hint;
}

void LFApplication::ShowTooltip(const CWnd* pWndOwner, CPoint point, const CString& Caption, const CString& Hint, HICON hIcon, HBITMAP hBitmap)
{
	ASSERT(IsWindow(m_wndTooltip));
	ASSERT(pWndOwner);

	(p_WndTooltipOwner=pWndOwner)->ClientToScreen(&point);
	m_wndTooltip.ShowTooltip(point, Caption, Hint, hIcon, hBitmap);
}

void LFApplication::HideTooltip(const CWnd* pWndOwner)
{
	if (!pWndOwner || (pWndOwner==p_WndTooltipOwner))
	{
		if (m_wndTooltip.IsWindowVisible())
			m_wndTooltip.HideTooltip();

		p_WndTooltipOwner = NULL;
	}
}


// Attributes

LPCWSTR LFApplication::GetAttributeName(UINT Attr, UINT Context) const
{
	ASSERT(Attr<LFAttributeCount);
	ASSERT(Context<LFContextCount);

	for (UINT a=0; a<LFAttributeContextRecordCount; a++)
		if (m_Attributes[Attr].ContextRecords[a].ContextSet & (1ull<<Context))
			return m_Attributes[Attr].ContextRecords[a].Name;

	ASSERT(FALSE);
	return m_Attributes[Attr].ContextRecords[0].Name;
}

INT LFApplication::GetAttributeIcon(UINT Attr, UINT Context) const
{
	ASSERT(Attr<LFAttributeCount);
	ASSERT(Context<LFContextCount);

	for (UINT a=0; a<LFAttributeContextRecordCount; a++)
		if (m_Attributes[Attr].ContextRecords[a].ContextSet & (1ull<<Context))
			return m_Attributes[Attr].ContextRecords[a].IconID;

	ASSERT(FALSE);
	return m_Attributes[Attr].AttrProperties.DefaultIconID;
}

BOOL LFApplication::IsAttributeSortDescending(UINT Context, UINT Attr) const
{
	ASSERT(Attr<LFAttributeCount);
	ASSERT(Context<LFContextCount);

	for (UINT a=0; a<LFAttributeContextRecordCount; a++)
		if (m_Attributes[Attr].ContextRecords[a].ContextSet & (1ull<<Context))
			return m_Attributes[Attr].ContextRecords[a].SortDescending;

	ASSERT(FALSE);
	return (m_Attributes[Attr].TypeProperties.DataFlags & LFDataSortDescending);
}


// Explorer context menu

void LFApplication::ExecuteExplorerContextMenu(CHAR cVolume, LPCSTR Verb)
{
	// Sicherheitsprüfung
	if (strcmp(Verb, "format")==0)
		if (LFStoresOnVolume(cVolume))
		{
			CString Caption;
			Caption.Format(IDS_FORMAT_CAPTION, cVolume);

			LFMessageBox(m_pMainWnd, CString((LPCSTR)IDS_FORMAT_MSG), Caption, MB_ICONWARNING);

			return;
		}

	// Ausführen
	WCHAR Path[4] = L" :\\";
	Path[0] = cVolume;

	LPITEMIDLIST pidlFQ = SHSimpleIDListFromPath(Path);
	LPCITEMIDLIST pidlRel;

	IShellFolder* pParentFolder;
	if (SUCCEEDED(SHBindToParent(pidlFQ, IID_IShellFolder, (void**)&pParentFolder, &pidlRel)))
	{
		IContextMenu* pContextMenu;
		if (SUCCEEDED(pParentFolder->GetUIObjectOf(m_pMainWnd->GetSafeHwnd(), 1, &pidlRel, IID_IContextMenu, NULL, (void**)&pContextMenu)))
		{
			HMENU hPopup = CreatePopupMenu();
			if (hPopup)
			{
				UINT uFlags = CMF_NORMAL | CMF_EXPLORE;
				if (SUCCEEDED(pContextMenu->QueryContextMenu(hPopup, 0, 1, 0x6FFF, uFlags)))
				{
					CWaitCursor WaitCursor;

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

					pContextMenu->InvokeCommand(&cmi);
				}
			}
		}
	}

	GetShellManager()->FreeItem(pidlFQ);
}

void LFApplication::OpenFolderAndSelectItem(LPCWSTR Path)
{
	ASSERT(Path);

	if (Path[0]!=L'\0')
	{
		LPITEMIDLIST pidlFQ;
		if (SUCCEEDED(SHParseDisplayName(Path, NULL, &pidlFQ, 0, NULL)))
		{
			SHOpenFolderAndSelectItems(pidlFQ, 0, NULL, 0);

			GetShellManager()->FreeItem(pidlFQ);
		}
	}
}


// Sounds

void LFApplication::PlayRegSound(const CString& Identifier)
{
	CString strKey;
	strKey.Format(_T("AppEvents\\Schemes\\%s\\.Current"), Identifier);

	CSettingsStoreSP regSP;
	CSettingsStore& reg = regSP.Create(FALSE, TRUE);

	if (reg.Open(strKey))
	{
		CString strFile;

		if (reg.Read(_T(""), strFile) && !strFile.IsEmpty())
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


// Updates

void LFApplication::GetUpdateSettings(BOOL& EnableAutoUpdate, INT& Interval) const
{
	EnableAutoUpdate = GetGlobalInt(_T("EnableAutoUpdate"), 1);
	Interval = GetGlobalInt(_T("UpdateCheckInterval"), 0);
}

void LFApplication::WriteUpdateSettings(BOOL EnableAutoUpdate, INT Interval) const
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
		FILETIME FileTime;
		GetSystemTimeAsFileTime(&FileTime);

		ULARGE_INTEGER Now;
		Now.HighPart = FileTime.dwHighDateTime;
		Now.LowPart = FileTime.dwLowDateTime;

		ULARGE_INTEGER LastUpdateCheck;
		LastUpdateCheck.HighPart = GetGlobalInt(_T("LastUpdateCheckHigh"), 0);
		LastUpdateCheck.LowPart = GetGlobalInt(_T("LastUpdateCheckLow"), 0);

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
			return TRUE;
	}

	return FALSE;
}

void LFApplication::WriteUpdateCheckTime() const
{
	FILETIME FileTime;
	GetSystemTimeAsFileTime(&FileTime);

	WriteGlobalInt(_T("LastUpdateCheckHigh"), FileTime.dwHighDateTime);
	WriteGlobalInt(_T("LastUpdateCheckLow"), FileTime.dwLowDateTime);
}

CString LFApplication::GetLatestVersion(CString CurrentVersion)
{
	CString VersionIni;

	// License
	CurrentVersion += LFIsLicensed() ? _T(" (licensed)") : LFIsSharewareExpired() ? _T(" (expired)") : _T(" (evaluation)");

	// Get available version
	HINTERNET hSession = WinHttpOpen(_T("liquidFOLDERS/")+CurrentVersion, WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
	if (hSession)
	{
		HINTERNET hConnect = WinHttpConnect(hSession, L"update.liquidfolders.net", INTERNET_DEFAULT_HTTP_PORT, 0);
		if (hConnect)
		{
			HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", L"/version.ini", NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
			if (hRequest)
			{
				if (WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, NULL, 0, 0, 0))
					if (WinHttpReceiveResponse(hRequest, NULL))
					{
						DWORD dwSize;

						do
						{
							dwSize = 0;
							if (WinHttpQueryDataAvailable(hRequest, &dwSize))
							{
								CHAR* pBuffer = new CHAR[dwSize+1];
								DWORD dwDownloaded;

								if (WinHttpReadData(hRequest, pBuffer, dwSize, &dwDownloaded))
								{
									pBuffer[dwDownloaded] = '\0';

									VersionIni += CString(pBuffer);
								}

								delete pBuffer;
							}
						}
						while (dwSize>0);
					}

				WinHttpCloseHandle(hRequest);
			}

			WinHttpCloseHandle(hConnect);
		}

		WinHttpCloseHandle(hSession);
	}

	return VersionIni;
}

CString LFApplication::GetIniValue(CString Ini, const CString& Name)
{
	while (!Ini.IsEmpty())
	{
		INT Pos = Ini.Find(L"\n");
		if (Pos==-1)
			Pos = Ini.GetLength()+1;

		CString Line = Ini.Mid(0, Pos-1);
		Ini.Delete(0, Pos+1);

		Pos = Line.Find(L"=");
		if (Pos!=-1)
			if (Line.Mid(0, Pos)==Name)
				return Line.Mid(Pos+1, Line.GetLength()-Pos);
	}

	return _T("");
}

void LFApplication::ParseVersion(const CString& tmpStr, LFVersion* pVersion)
{
	ASSERT(pVersion);

	swscanf_s(tmpStr, L"%u.%u.%u", &pVersion->Major, &pVersion->Minor, &pVersion->Build);
}

BOOL LFApplication::IsVersionLater(const LFVersion& LatestVersion, const LFVersion& CurrentVersion)
{
	return ((LatestVersion.Major>CurrentVersion.Major) ||
		((LatestVersion.Major==CurrentVersion.Major) && (LatestVersion.Minor>CurrentVersion.Minor)) ||
		((LatestVersion.Major==CurrentVersion.Major) && (LatestVersion.Minor==CurrentVersion.Minor) && (LatestVersion.Build>CurrentVersion.Build)));
}

BOOL LFApplication::IsUpdateFeatureLater(const CString& VersionIni, const CString& Name, LFVersion& CurrentVersion)
{
	LFVersion FeatureVersion = { 0 };
	ParseVersion(GetIniValue(VersionIni, Name), &FeatureVersion);

	return IsVersionLater(FeatureVersion, CurrentVersion);
}

DWORD LFApplication::GetUpdateFeatures(const CString& VersionIni, LFVersion& CurrentVersion)
{
	DWORD UpdateFeatures = 0;

	if (IsUpdateFeatureLater(VersionIni, _T("SecurityPatch"), CurrentVersion))
		UpdateFeatures |= UPDATE_SECUTIRYPATCH;

	if (IsUpdateFeatureLater(VersionIni, _T("ImportantBugfix"), CurrentVersion))
		UpdateFeatures |= UPDATE_IMPORTANTBUGFIX;

	if (IsUpdateFeatureLater(VersionIni, _T("NetworkAPI"), CurrentVersion))
		UpdateFeatures |= UPDATE_NETWORKAPI;

	if (IsUpdateFeatureLater(VersionIni, _T("NewFeature"), CurrentVersion))
		UpdateFeatures |= UPDATE_NEWFEATURE;

	if (IsUpdateFeatureLater(VersionIni, _T("NewVisualization"), CurrentVersion))
		UpdateFeatures |= UPDATE_NEWVISUALIZATION;

	if (IsUpdateFeatureLater(VersionIni, _T("UI"), CurrentVersion))
		UpdateFeatures |= UPDATE_UI;

	if (IsUpdateFeatureLater(VersionIni, _T("SmallBugfix"), CurrentVersion))
		UpdateFeatures |= UPDATE_SMALLBUGFIX;

	if (IsUpdateFeatureLater(VersionIni, _T("IATA"), CurrentVersion))
		UpdateFeatures |= UPDATE_IATA;

	if (IsUpdateFeatureLater(VersionIni, _T("Performance"), CurrentVersion))
		UpdateFeatures |= UPDATE_PERFORMANCE;

	return UpdateFeatures;
}

void LFApplication::CheckForUpdate(BOOL Force, CWnd* pParentWnd)
{
	// Obtain current version from instance version resource
	CString CurrentVersion;
	GetFileVersion(AfxGetResourceHandle(), CurrentVersion);

	LFVersion CV = { 0 };
	ParseVersion(CurrentVersion, &CV);

	// Check due?
	const BOOL Check = Force | IsUpdateCheckDue();

	// Perform check
	CString LatestVersion = GetGlobalString(_T("LatestUpdateVersion"));
	CString LatestMSN = GetGlobalString(_T("LatestUpdateMSN"));
	DWORD LatestUpdateFeatures = GetGlobalInt(_T("LatestUpdateFeatures"));

	if (Check)
	{
		CWaitCursor WaitCursor;
		CString VersionIni = GetLatestVersion(CurrentVersion);

		if (!VersionIni.IsEmpty())
		{
			LatestVersion = GetIniValue(VersionIni, _T("Version"));
			LatestMSN = GetIniValue(VersionIni, _T("MSN"));
			LatestUpdateFeatures = GetUpdateFeatures(VersionIni, CV);

			WriteGlobalString(_T("LatestUpdateVersion"), LatestVersion);
			WriteGlobalString(_T("LatestUpdateMSN"), LatestMSN);
			WriteGlobalInt(_T("LatestUpdateFeatures"), LatestUpdateFeatures);
			WriteUpdateCheckTime();
		}
	}

	// Update available?
	BOOL UpdateAvailable = FALSE;
	if (!LatestVersion.IsEmpty())
	{
		LFVersion LV = { 0 };
		ParseVersion(LatestVersion, &LV);

		CString IgnoreMSN = GetGlobalString(_T("IgnoreUpdateMSN"));

		UpdateAvailable = ((IgnoreMSN!=LatestMSN) || Force) && IsVersionLater(LV, CV);
	}

	// Result
	if (UpdateAvailable)
	{
		if (pParentWnd)
		{
			if (m_pUpdateNotification)
				m_pUpdateNotification->DestroyWindow();

			LFUpdateDlg(LatestVersion, LatestMSN, LatestUpdateFeatures, pParentWnd).DoModal();
		}
		else
			if (m_pUpdateNotification)
			{
				m_pUpdateNotification->SendMessage(WM_COMMAND, IDM_UPDATE_RESTORE);
			}
			else
			{
				m_pUpdateNotification = new LFUpdateDlg(LatestVersion, LatestMSN, LatestUpdateFeatures);
				m_pUpdateNotification->Create();
				m_pUpdateNotification->ShowWindow(SW_SHOW);
			}
	}
	else
	{
		if (Force)
			LFMessageBox(pParentWnd, CString((LPCSTR)IDS_UPDATENOTAVAILABLE), CString((LPCSTR)IDS_UPDATE), MB_ICONINFORMATION | MB_OK);
	}
}


BEGIN_MESSAGE_MAP(LFApplication, CWinAppEx)
	ON_COMMAND(IDM_BACKSTAGE_PURCHASE, OnBackstagePurchase)
	ON_COMMAND(IDM_BACKSTAGE_ENTERLICENSEKEY, OnBackstageEnterLicenseKey)
	ON_COMMAND(IDM_BACKSTAGE_SUPPORT, OnBackstageSupport)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_BACKSTAGE_PURCHASE, IDM_BACKSTAGE_ABOUT, OnUpdateBackstageCommands)
END_MESSAGE_MAP()

void LFApplication::OnBackstagePurchase()
{
	ShellExecute(m_pActiveWnd->GetSafeHwnd(), _T("open"), CString((LPCSTR)IDS_PURCHASEURL), NULL, NULL, SW_SHOWNORMAL);
}

void LFApplication::OnBackstageEnterLicenseKey()
{
	LFLicenseDlg(m_pActiveWnd).DoModal();
}

void LFApplication::OnBackstageSupport()
{
	SendMail();
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
