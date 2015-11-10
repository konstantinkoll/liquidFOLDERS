
// liquidFOLDERS.cpp: Definiert das Klassenverhalten f�r die Anwendung.
//

#include "stdafx.h"
#include "CFileDropWnd.h"
#include "liquidFOLDERS.h"


GUID theAppID =	// {5EB05AE5-C6FE-4E53-A034-3623921D18ED}
	{ 0x5EB05AE5, 0xC6FE, 0x4E53, { 0xA0, 0x34, 0x36, 0x23, 0x92, 0x1D, 0x18, 0xED } };

BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam)
{
	if (GetWindow(hWnd, GW_OWNER))
		return TRUE;

	DWORD_PTR Result;
	if (SendMessageTimeout(hWnd, theApp.m_WakeupMsg, NULL, NULL, SMTO_NORMAL, 500, &Result))
		if (Result==24878)
		{
			CDS_Wakeup cdsw;
			ZeroMemory(&cdsw, sizeof(cdsw));
			cdsw.AppID = theAppID;
			if (lParam)
				wcscpy_s(cdsw.Command, MAX_PATH, (WCHAR*)lParam);

			COPYDATASTRUCT cds;
			cds.cbData = sizeof(cdsw);
			cds.lpData = &cdsw;
			if (SendMessage(hWnd, WM_COPYDATA, NULL, (LPARAM)&cds))
				return FALSE;
		}

	return TRUE;
}


// CLiquidFoldersApp

// CLiquidFoldersApp-Erstellung

CLiquidFoldersApp::CLiquidFoldersApp()
	: LFApplication(theAppID)
{
	m_AppInitialized = FALSE;
}


// Das einzige CLiquidFoldersApp-Objekt

CLiquidFoldersApp theApp;


// CLiquidFoldersApp-Initialisierung

BOOL CLiquidFoldersApp::InitInstance()
{
	WCHAR CmdLine[256] = L"";
	for (INT a=1; a<__argc; a++)
	{
		wcscat_s(CmdLine, 256, __wargv[a]);

		if (a<__argc-1)
			wcscat_s(CmdLine, 256, L" ");
	}

	// Parameter
	if (!EnumWindows((WNDENUMPROC)EnumWindowsProc, (LPARAM)(__argc>1 ? CmdLine : NULL)))
		return FALSE;

	if (!LFApplication::InitInstance())
		return FALSE;

	// AppID
	if (m_ShellLibLoaded)
		zSetCurrentProcessExplicitAppUserModelID(L"liquidFOLDERS");

	// RestartManager
	if (m_KernelLibLoaded)
		zRegisterApplicationRestart(L"", 8);	// RESTART_NO_REBOOT

	for (UINT a=0; a<LFContextCount; a++)
	{
		m_AllowedViews[a] = 0;

		UINT cnt = (a==LFContextStores) ? LFViewStrips : ((a<=LFLastGroupContext) || (a==LFContextSearch)) ? LFViewCount-1 : LFViewPreview;
		for (UINT b=0; b<=cnt; b++)
			m_AllowedViews[a] |= 1<<b;
	}

	// Pfad zu Google Earth
	HKEY hKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\Google\\Google Earth Plus"), 0, KEY_READ | KEY_WOW64_64KEY, &hKey)==ERROR_SUCCESS)
	{
		DWORD dwType = REG_SZ;
		WCHAR lszValue[256];
		DWORD dwSize = sizeof(lszValue);

		if (RegQueryValueEx(hKey, _T("InstallLocation"), NULL, &dwType, (LPBYTE)&lszValue, &dwSize)==ERROR_SUCCESS)
			m_PathGoogleEarth = lszValue;

		RegCloseKey(hKey);
	}

	// Registry auslesen
	SetRegistryBase(_T("Settings"));
	m_ShowInspectorPane = GetInt(_T("ShowInspectorPane"), TRUE);

	m_InspectorWidth = GetInt(_T("InspectorWidth"), 200);
	if (m_InspectorWidth<32)
		m_InspectorWidth = 32;

	m_nTextureSize = GetInt(_T("TextureSize"), LFTextureAuto);
	m_nMaxTextureSize = GetInt(_T("MaxTextureSize"), LFTexture4096);
	if (m_nTextureSize<0)
		m_nTextureSize = 0;
	if (m_nTextureSize>m_nMaxTextureSize)
		m_nTextureSize = m_nMaxTextureSize;

	m_CalendarShowCaptions = GetInt(_T("CalendarShowCaptions"), TRUE);
	m_GlobeLighting = GetInt(_T("GlobeLighting"), TRUE);
	m_GlobeAtmosphere = GetInt(_T("GlobeAtmosphere"), TRUE);
	m_GlobeShowViewport = GetInt(_T("GlobeShowViewport"), FALSE);
	m_GlobeShowCrosshairs = GetInt(_T("GlobeShowCrosshairs"), FALSE);
	m_FileDropAlwaysOnTop = GetInt(_T("FileDropAlwaysOnTop"), TRUE);

	for (UINT a=0; a<LFContextCount; a++)
		LoadViewOptions(a);

	m_SourceIcons.Create(IDB_SOURCEICONS);

	CWnd* pFrame = OpenCommandLine(__argc>1 ? CmdLine : NULL);
	if (pFrame)
	{
		LFCheckForUpdate();

		if (LFGetStoreCount()==0)
		{
			LFAddStoreDlg dlg(pFrame);
			dlg.DoModal();
		}
	}

	m_AppInitialized = TRUE;

	return TRUE;
}

CWnd* CLiquidFoldersApp::OpenCommandLine(WCHAR* CmdLine)
{
	// Parse parameter and create window
	if (CmdLine)
	{
		// Prepare arguments
		WCHAR* pChar = CmdLine;
		while (*pChar)
			*(pChar++) = (WCHAR)toupper(*pChar);

		CHAR StoreID[LFKeySize] = "";

		WCHAR* pSpace = wcschr(CmdLine, L' ');
		if (pSpace)
		{
			WideCharToMultiByte(CP_ACP, 0, pSpace+1, -1, StoreID, LFKeySize, NULL, NULL);
			*pSpace = L'\0';
		}

		// Update
		if (wcscmp(CmdLine, L"/CHECKUPDATE")==0)
		{
			LFCheckForUpdate();

			return NULL;
		}

		// FileDrop
		if (wcscmp(CmdLine, L"/FILEDROP")==0)
		{
			if (StoreID[0]=='\0')
			{
				UINT Result = LFGetDefaultStore(StoreID);
				if (Result!=LFOk)
				{
					LFErrorBox(CWnd::GetForegroundWindow(), Result);

					return NULL;
				}
			}

			return GetFileDrop(StoreID);
		}

		// Key or IATA code
		if ((wcschr(CmdLine, L'.')==NULL) && (wcschr(CmdLine, L':')==NULL) && (wcschr(CmdLine, L'\\')==NULL) && (wcschr(CmdLine, L'/')==NULL))
		{
			// Key
			if (wcslen(CmdLine)==LFKeySize-1)
			{
				WideCharToMultiByte(CP_ACP, 0, CmdLine, -1, StoreID, LFKeySize, NULL, NULL);

				CMainWnd* pFrame = new CMainWnd();
				pFrame->CreateStore(StoreID);
				pFrame->ShowWindow(SW_SHOW);
	
				return pFrame;
			}

			// IATA airport code
			if (wcslen(CmdLine)==3)
			{
				CHAR Code[4];
				WideCharToMultiByte(CP_ACP, 0, CmdLine, -1, Code, 4, NULL, NULL);

				LFFilter* f = LFAllocFilter();
				f->Mode = LFFilterModeSearch;
				f->ConditionList = LFAllocFilterConditionEx(LFFilterCompareIsEqual, LFAttrLocationIATA);
				strcpy_s(f->ConditionList->AttrData.AnsiString, 256, Code);

				LFAirport* pAirport = NULL;
				if (LFIATAGetAirportByCode(Code, &pAirport))
					MultiByteToWideChar(CP_ACP, 0, pAirport->Name, -1, f->OriginalName, 256);

				CMainWnd* pFrame = new CMainWnd();
				pFrame->CreateFilter(f);
				pFrame->ShowWindow(SW_SHOW);
	
				return pFrame;
			}
		}

		// Filter
		CMainWnd* pFrame = new CMainWnd();
		pFrame->CreateFilter(CmdLine);
		pFrame->ShowWindow(SW_SHOW);

		return pFrame;
	}

	// Root
	CMainWnd* pFrame = new CMainWnd();
	pFrame->CreateRoot();
	pFrame->ShowWindow(SW_SHOW);

	return pFrame;
}

INT CLiquidFoldersApp::ExitInstance()
{
	if (m_AppInitialized)
	{
		for (UINT a=0; a<LFContextCount; a++)
			SaveViewOptions(a);

		SetRegistryBase(_T("Settings"));
		WriteInt(_T("ShowInspectorPane"), m_ShowInspectorPane);
		WriteInt(_T("InspectorWidth"), m_InspectorWidth);
		WriteInt(_T("TextureSize"), m_nTextureSize);
		WriteInt(_T("MaxTextureSize"), m_nMaxTextureSize);
		WriteInt(_T("CalendarShowCaptions"), m_CalendarShowCaptions);
		WriteInt(_T("GlobeLighting"), m_GlobeLighting);
		WriteInt(_T("GlobeAtmosphere"), m_GlobeAtmosphere);
		WriteInt(_T("GlobeShowViewport"), m_GlobeShowViewport);
		WriteInt(_T("GlobeShowCrosshairs"), m_GlobeShowCrosshairs);
		WriteInt(_T("FileDropAlwaysOnTop"), m_FileDropAlwaysOnTop);
	}

	return LFApplication::ExitInstance();
}


CMainWnd* CLiquidFoldersApp::GetClipboard()
{
	if (!p_Clipboard)
	{
		p_Clipboard = new CMainWnd();
		p_Clipboard->CreateClipboard();
		p_Clipboard->ShowWindow(SW_SHOW);
	}

	return p_Clipboard;
}

CWnd* CLiquidFoldersApp::GetFileDrop(CHAR* StoreID)
{
	for (POSITION p=m_pMainFrames.GetHeadPosition(); p; )
	{
		CWnd* pFrame = m_pMainFrames.GetNext(p);
		if (pFrame->SendMessage(WM_OPENFILEDROP, (WPARAM)StoreID)==24878)
			return pFrame;
	}

	CFileDropWnd* pFrame = new CFileDropWnd();
	pFrame->Create(StoreID);
	pFrame->ShowWindow(SW_SHOW);

	return pFrame;
}


__forceinline BOOL CLiquidFoldersApp::IsViewAllowed(INT Context, INT View) const
{
	ASSERT(View>=0);
	ASSERT(View<=31);

	return m_AllowedViews[Context] & (1<<View);
}

BOOL CLiquidFoldersApp::SanitizeSortBy(LFViewParameters* pViewParameters, INT Context) const
{
	BOOL Modified = FALSE;

	// Enforce valid view mode
	if ((pViewParameters->Mode>=LFViewCount) || (!IsViewAllowed(Context, pViewParameters->Mode)))
	{
		pViewParameters->Mode = LFViewTiles;
		Modified = TRUE;
	}

	// Choose other view mode if neccessary
	if (!AttributeSortableInView(pViewParameters->SortBy, pViewParameters->Mode))
		for (UINT a=0; a<LFViewCount; a++)
			if (IsViewAllowed(Context, a) && (AttributeSortableInView(pViewParameters->SortBy, a)))
			{
				pViewParameters->Mode = (a<=LFViewTiles) ? LFViewTiles : a;
				Modified = TRUE;
				break;
			}

	return Modified;
}

BOOL CLiquidFoldersApp::SanitizeViewMode(LFViewParameters* pViewParameters, INT Context) const
{
	BOOL Modified = FALSE;

	// Enforce valid view mode
	if ((pViewParameters->Mode>=LFViewCount) || (!IsViewAllowed(Context, pViewParameters->Mode)))
	{
		pViewParameters->Mode = LFViewTiles;
		Modified = TRUE;
	}

	// Choose other sorting if neccessary
	if (!AttributeSortableInView(pViewParameters->SortBy, pViewParameters->Mode))
		for (UINT a=0; a<LFAttributeCount; a++)
			if (AttributeSortableInView(a, pViewParameters->Mode))
			{
				pViewParameters->SortBy = a;
				Modified = TRUE;
				break;
			}

	return Modified;
}

void CLiquidFoldersApp::Broadcast(INT Context, INT View, UINT cmdMsg)
{
	for (POSITION p=m_pMainFrames.GetHeadPosition(); p; )
		m_pMainFrames.GetNext(p)->PostMessage(WM_CONTEXTVIEWCOMMAND, cmdMsg, MAKELPARAM(Context, View));
}

void CLiquidFoldersApp::UpdateSortOptions(INT Context)
{
	SanitizeSortBy(&m_Views[Context], Context);
	Broadcast(Context, -1, WM_UPDATESORTOPTIONS);
}

void CLiquidFoldersApp::UpdateViewOptions(INT Context, INT View)
{
	BOOL Modified = (Context!=-1) ? SanitizeViewMode(&m_Views[Context], Context) : FALSE;
	Broadcast(Context, Modified ? -1 : View, Modified ? WM_UPDATESORTOPTIONS : WM_UPDATEVIEWOPTIONS);
}

void CLiquidFoldersApp::Reload(INT Context)
{
	Broadcast(Context, -1, WM_RELOAD);
}


// Registry and view settings

void CLiquidFoldersApp::LoadViewOptions(UINT Context)
{
	CString base;
	base.Format(_T("Settings\\Context%u"), Context);
	SetRegistryBase(base);

	UINT DefaultMode = LFViewTiles;
	UINT DefaultSortBy = LFAttrFileName;
	BOOL DefaultDescending = FALSE;

	switch (Context)
	{
	case LFContextFavorites:
		DefaultSortBy = LFAttrRating;
		DefaultDescending = TRUE;
		break;

	case LFContextAllFiles:
	case LFContextPictures:
	case LFContextVideos:
		DefaultMode = LFViewTimeline;
		DefaultSortBy = LFAttrCreationTime;
		break;

	case LFContextAudio:
		DefaultMode = LFViewLargeIcons;
		DefaultSortBy = LFAttrArtist;
		break;

	case LFContextNew:
		DefaultSortBy = LFAttrAddTime;
		break;

	case LFContextArchive:
		DefaultSortBy = LFAttrArchiveTime;
		DefaultMode = LFViewDetails;
		break;

	case LFContextSubfolderDay:
		DefaultMode = LFViewPreview;
		break;

	case LFContextTrash:
		DefaultSortBy = LFAttrDeleteTime;
		DefaultDescending = TRUE;
		break;

	case LFContextSearch:
	case LFContextClipboard:
		DefaultMode = LFViewContent;
		break;

	case LFContextStores:
		DefaultMode = LFViewLargeIcons;
		break;
	}

	if (GetInt(_T("Version"), 0)==ViewParametersVersion)
	{
		m_Views[Context].Mode = GetInt(_T("Mode"), DefaultMode);
		m_Views[Context].SortBy = GetInt(_T("SortBy"), DefaultSortBy);
		m_Views[Context].Descending = GetInt(_T("Descending"), DefaultDescending);
		m_Views[Context].AutoDirs = GetInt(_T("AutoDirs"), TRUE);
	}
	else
	{
		m_Views[Context].Mode = DefaultMode;
		m_Views[Context].SortBy = DefaultSortBy;
		m_Views[Context].Descending = DefaultDescending;
		m_Views[Context].AutoDirs = TRUE;
	}

	m_Views[Context].GlobeLatitude = GetInt(_T("GlobeLatitude"), 1);
	m_Views[Context].GlobeLongitude = GetInt(_T("GlobeLongitude"), 1);
	m_Views[Context].GlobeZoom = GetInt(_T("GlobeZoom"), 600);
	m_Views[Context].GlobeShowSpots = GetInt(_T("GlobeShowSpots"), TRUE);
	m_Views[Context].GlobeShowAirportNames = GetInt(_T("GlobeShowAirportNames"), TRUE);
	m_Views[Context].GlobeShowGPS = GetInt(_T("GlobeShowGPS"), TRUE);
	m_Views[Context].GlobeShowDescription = GetInt(_T("GlobeShowDescription"), TRUE);
	m_Views[Context].TagcloudCanonical = GetInt(_T("TagcloudSortCanonical"), TRUE);
	m_Views[Context].TagcloudShowRare = GetInt(_T("TagcloudShowRare"), TRUE);
	m_Views[Context].TagcloudUseSize = GetInt(_T("TagcloudUseSize"), TRUE);
	m_Views[Context].TagcloudUseColors = GetInt(_T("TagcloudUseColors"), TRUE);
	m_Views[Context].TagcloudUseOpacity = GetInt(_T("TagcloudUseOpacity"), FALSE);

	if ((m_Views[Context].Mode>=LFViewCount) || (!IsViewAllowed(Context, m_Views[Context].Mode)))
		m_Views[Context].Mode = DefaultMode;

	if (!IsAttributeAllowed(Context, m_Views[Context].SortBy))
		m_Views[Context].SortBy = DefaultSortBy;

	for (UINT a=0; a<LFAttributeCount; a++)
	{
		m_Views[Context].ColumnOrder[a] = a;

		if (IsAttributeAllowed(Context, a) && (a!=LFAttrStoreID) && (a!=LFAttrFileID) && (a!=LFAttrFileFormat))
		{
			m_Views[Context].ColumnWidth[a] = m_Attributes[a].RecommendedWidth;
		}
		else
		{
			m_Views[Context].ColumnWidth[a] = 0;
		}
	}

	GetBinary(_T("ColumnOrder"), &m_Views[Context].ColumnOrder, sizeof(m_Views[Context].ColumnOrder));
	GetBinary(_T("ColumnWidth"), &m_Views[Context].ColumnWidth, sizeof(m_Views[Context].ColumnWidth));

	for (UINT a=0; a<LFAttributeCount; a++)
		if (!LFIsAttributeAllowed(m_Contexts[Context], a))
			m_Views[Context].ColumnWidth[a] = 0;

	m_Views[Context].AutoDirs &= (m_Contexts[Context].AllowGroups) || (Context>=LFContextSubfolderDefault);

	SetRegistryBase(_T("Settings"));
}

void CLiquidFoldersApp::SaveViewOptions(UINT Context)
{
	CString base;
	base.Format(_T("Settings\\Context%u"), Context);
	SetRegistryBase(base);

	WriteInt(_T("Version"), ViewParametersVersion);
	WriteInt(_T("Mode"), m_Views[Context].Mode);
	WriteInt(_T("SortBy"), m_Views[Context].SortBy);
	WriteInt(_T("Descending"), m_Views[Context].Descending);
	WriteInt(_T("AutoDirs"), m_Views[Context].AutoDirs);
	WriteInt(_T("GlobeLatitude"), m_Views[Context].GlobeLatitude);
	WriteInt(_T("GlobeLongitude"), m_Views[Context].GlobeLongitude);
	WriteInt(_T("GlobeZoom"), m_Views[Context].GlobeZoom);
	WriteInt(_T("GlobeShowSpots"), m_Views[Context].GlobeShowSpots);
	WriteInt(_T("GlobeShowAirportNames"), m_Views[Context].GlobeShowAirportNames);
	WriteInt(_T("GlobeShowGPS"), m_Views[Context].GlobeShowGPS);
	WriteInt(_T("GlobeShowDescription"), m_Views[Context].GlobeShowDescription);
	WriteInt(_T("TagcloudSortCanonical"), m_Views[Context].TagcloudCanonical);
	WriteInt(_T("TagcloudShowRare"), m_Views[Context].TagcloudShowRare);
	WriteInt(_T("TagcloudUseSize"), m_Views[Context].TagcloudUseSize);
	WriteInt(_T("TagcloudUseColors"), m_Views[Context].TagcloudUseColors);
	WriteInt(_T("TagcloudUseOpacity"), m_Views[Context].TagcloudUseOpacity);

	WriteBinary(_T("ColumnOrder"), (LPBYTE)m_Views[Context].ColumnOrder, sizeof(m_Views[Context].ColumnOrder));
	WriteBinary(_T("ColumnWidth"), (LPBYTE)m_Views[Context].ColumnWidth, sizeof(m_Views[Context].ColumnWidth));

	SetRegistryBase(_T("Settings"));
}
