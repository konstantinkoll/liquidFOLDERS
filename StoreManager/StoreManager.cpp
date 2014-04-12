
// StoreManager.cpp: Definiert das Klassenverhalten für die Anwendung.
//

#include "stdafx.h"
#include "FileDropWnd.h"
#include "..\\LFCore\\resource.h"
#include "LFCore.h"
#include "LFCommDlg.h"
#include "MainWnd.h"
#include "StoreManager.h"


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


// CStoreManagerApp

BEGIN_MESSAGE_MAP(CStoreManagerApp, LFApplication)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
END_MESSAGE_MAP()


// CStoreManagerApp-Erstellung

CStoreManagerApp::CStoreManagerApp()
	: LFApplication(theAppID)
{
	m_WakeupMsg = RegisterWindowMessage(_T("liquidFOLDERS.StoreManager.NewWindow"));
	m_NagCounter = 3;
	m_AppInitialized = FALSE;
}


// Das einzige CStoreManagerApp-Objekt

CStoreManagerApp theApp;


// CStoreManagerApp-Initialisierung

BOOL CStoreManagerApp::InitInstance()
{
	WCHAR CmdLine[256] = L"";
	for (INT a=1; a<__argc; a++)
		wcscat_s(CmdLine, 256, __wargv[a]);

	// Parameter
	if (!EnumWindows((WNDENUMPROC)EnumWindowsProc, (LPARAM)(__argc>1 ? CmdLine : NULL)))
		return FALSE;

	if (!LFApplication::InitInstance())
		return FALSE;

	// AppID
	if (m_ShellLibLoaded)
		zSetCurrentProcessExplicitAppUserModelID(L"liquidFOLDERS.StoreManager");

	for (UINT a=0; a<LFContextCount; a++)
	{
		m_AllowedViews[a] = new LFBitArray(LFViewCount);

		UINT cnt = (a==LFContextStores) ? LFViewStrips : ((a<=LFLastGroupContext) || (a==LFContextSearch)) ? LFViewCount-1 : LFViewPreview;
		for (UINT b=0; b<=cnt; b++)
			(*m_AllowedViews[a]) += b;
	}

	// Pfad zu Google Earth
	HKEY hKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\Google\\Google Earth Plus"), 0, KEY_ALL_ACCESS, &hKey)==ERROR_SUCCESS)
	{
		DWORD dwType = REG_SZ;
		CHAR lszValue[255];
		DWORD dwSize = 255;

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
	m_GlobeHQModel = GetInt(_T("GlobeHQModel"), TRUE);
	m_GlobeLighting = GetInt(_T("GlobeLighting"), TRUE);
	m_GlobeAtmosphere = GetInt(_T("GlobeAtmosphere"), TRUE);
	m_GlobeShowViewport = GetInt(_T("GlobeShowViewport"), FALSE);
	m_GlobeShowCrosshairs = GetInt(_T("GlobeShowCrosshairs"), FALSE);
	m_FileDropAlwaysOnTop = GetInt(_T("FileDropAlwaysOnTop"), TRUE);

	for (UINT a=0; a<LFContextCount; a++)
		LoadViewOptions(a);

	m_SourceIcons.Create(IDB_SOURCEICONS);

	m_ThumbnailCache.LoadFrames();

	CWnd* pFrame = OpenCommandLine(__argc>1 ? CmdLine : NULL);
	if (pFrame)
	{
		if (!LFIsLicensed())
			ShowNagScreen(NAG_NOTLICENSED | NAG_FORCE, pFrame);

		LFCheckForUpdate();
	}

	m_AppInitialized = TRUE;

	return TRUE;
}

CWnd* CStoreManagerApp::OpenCommandLine(WCHAR* CmdLine)
{
	CHAR StoreID[LFKeySize] = "";

	// Parse parameter and create window
	if (CmdLine)
	{
		WCHAR* pChar = CmdLine;
		while (*pChar)
			*(pChar++) = (WCHAR)toupper(*pChar);

		// Update
		if (wcscmp(CmdLine, L"/CHECKUPDATE")==0)
		{
			LFCheckForUpdate();
			return NULL;
		}

		// FileDrop
		if (wcsncmp(CmdLine, L"/FILEDROP", 9)==0)
		{
			if (CmdLine[9]==L' ')
			{
				WideCharToMultiByte(CP_ACP, 0, CmdLine+10, -1, StoreID, LFKeySize, NULL, NULL);
			}
			else
			{
				CHAR* pDefaultStore = LFGetDefaultStore();
				strcpy_s(StoreID, LFKeySize, pDefaultStore);
				free(pDefaultStore);
			}

			return GetFileDrop(StoreID);
		}

		// Key
		if ((wcslen(CmdLine)==LFKeySize-1) && (wcschr(CmdLine, L'.')==NULL) && (wcschr(CmdLine, L':')==NULL) && (wcschr(CmdLine, L'\\')==NULL))
		{
			WideCharToMultiByte(CP_ACP, 0, CmdLine, -1, StoreID, LFKeySize, NULL, NULL);

			CMainWnd* pFrame = new CMainWnd();
			pFrame->CreateStore(StoreID);
			pFrame->ShowWindow(SW_SHOW);

			return pFrame;
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

INT CStoreManagerApp::ExitInstance()
{
	if (m_AppInitialized)
	{
		for (UINT a=0; a<LFContextCount; a++)
		{
			SaveViewOptions(a);
			delete m_AllowedViews[a];
		}

		SetRegistryBase(_T("Settings"));
		WriteInt(_T("ShowInspectorPane"), m_ShowInspectorPane);
		WriteInt(_T("InspectorWidth"), m_InspectorWidth);
		WriteInt(_T("TextureSize"), m_nTextureSize);
		WriteInt(_T("MaxTextureSize"), m_nMaxTextureSize);
		WriteInt(_T("CalendarShowCaptions"), m_CalendarShowCaptions);
		WriteInt(_T("GlobeHQModel"), m_GlobeHQModel);
		WriteInt(_T("GlobeLighting"), m_GlobeLighting);
		WriteInt(_T("GlobeAtmosphere"), m_GlobeAtmosphere);
		WriteInt(_T("GlobeShowViewport"), m_GlobeShowViewport);
		WriteInt(_T("GlobeShowCrosshairs"), m_GlobeShowCrosshairs);
		WriteInt(_T("FileDropAlwaysOnTop"), m_FileDropAlwaysOnTop);
	}

	m_ThumbnailCache.DeleteFrames();

	return LFApplication::ExitInstance();
}


CMainWnd* CStoreManagerApp::GetClipboard()
{
	if (!p_Clipboard)
	{
		p_Clipboard = new CMainWnd();
		p_Clipboard->CreateClipboard();
		p_Clipboard->ShowWindow(SW_SHOW);
	}

	return p_Clipboard;
}

CGlassWindow* CStoreManagerApp::GetFileDrop(CHAR* StoreID)
{
	for (POSITION p=m_MainFrames.GetHeadPosition(); p; )
	{
		CGlassWindow* pFrame = m_MainFrames.GetNext(p);
		if (pFrame->SendMessage(WM_OPENFILEDROP, (WPARAM)StoreID)==24878)
			return pFrame;
	}

	CFileDropWnd* pFrame = new CFileDropWnd();
	pFrame->Create(StoreID);
	pFrame->ShowWindow(SW_SHOW);

	return pFrame;
}

void CStoreManagerApp::OnAppAbout()
{
	CString AppName;
	ENSURE(AppName.LoadString(IDR_APPLICATION));
	TIMESTAMP;
	LFAbout(AppName, Timestamp, IDB_ABOUTICON, m_pActiveWnd);
}


BOOL CStoreManagerApp::SanitizeSortBy(LFViewParameters* vp, INT context)
{
	BOOL Modified = FALSE;

	// Enforce valid view mode
	if ((vp->Mode>=LFViewCount) || (!theApp.m_AllowedViews[context]->IsSet(vp->Mode)))
	{
		vp->Mode = LFViewTiles;
		Modified = TRUE;
	}

	// Choose other view mode if neccessary
	if (!AttributeSortableInView(vp->SortBy, vp->Mode))
		for (UINT a=0; a<LFViewCount; a++)
			if ((theApp.m_AllowedViews[context]->IsSet(a)) && (AttributeSortableInView(vp->SortBy, a)))
			{
				vp->Mode = (a<=LFViewTiles) ? LFViewTiles : a;
				Modified = TRUE;
				break;
			}

	return Modified;
}

BOOL CStoreManagerApp::SanitizeViewMode(LFViewParameters* vp, INT context)
{
	BOOL Modified = FALSE;

	// Enforce valid view mode
	if ((vp->Mode>=LFViewCount) || (!theApp.m_AllowedViews[context]->IsSet(vp->Mode)))
	{
		vp->Mode = LFViewTiles;
		Modified = TRUE;
	}

	// Choose other sorting if neccessary
	if (!AttributeSortableInView(vp->SortBy, vp->Mode))
		for (UINT a=0; a<LFAttributeCount; a++)
			if (AttributeSortableInView(a, vp->Mode))
			{
				vp->SortBy = a;
				Modified = TRUE;
				break;
			}

	return Modified;
}

void CStoreManagerApp::Broadcast(INT Context, INT View, UINT cmdMsg)
{
	for (POSITION p=m_MainFrames.GetHeadPosition(); p; )
		m_MainFrames.GetNext(p)->PostMessage(WM_CONTEXTVIEWCOMMAND, cmdMsg, MAKELPARAM(Context, View));
}

void CStoreManagerApp::UpdateSortOptions(INT Context)
{
	SanitizeSortBy(&theApp.m_Views[Context], Context);
	Broadcast(Context, -1, WM_UPDATESORTOPTIONS);
}

void CStoreManagerApp::UpdateViewOptions(INT Context, INT View)
{
	BOOL Modified = (Context!=-1) ? SanitizeViewMode(&theApp.m_Views[Context], Context) : FALSE;
	Broadcast(Context, Modified ? -1 : View, Modified ? WM_UPDATESORTOPTIONS : WM_UPDATEVIEWOPTIONS);
}

void CStoreManagerApp::Reload(INT Context)
{
	Broadcast(Context, -1, WM_RELOAD);
}


// Registry and view settings

void CStoreManagerApp::GetBinary(LPCTSTR lpszEntry, void* pData, UINT size)
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

void CStoreManagerApp::LoadViewOptions(UINT context)
{
	CString base;
	base.Format(_T("Settings\\Context%u"), context);
	SetRegistryBase(base);

	UINT DefaultMode = LFViewTiles;
	UINT DefaultSortBy = LFAttrFileName;
	BOOL DefaultDescending = FALSE;
	switch (context)
	{
	case LFContextFavorites:
		DefaultSortBy = LFAttrRating;
		DefaultDescending = TRUE;
		break;
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
	case LFContextSubfolderDay:
		DefaultMode = LFViewDetails;
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
		m_Views[context].Mode = GetInt(_T("Mode"), DefaultMode);
		m_Views[context].SortBy = GetInt(_T("SortBy"), DefaultSortBy);
		m_Views[context].Descending = GetInt(_T("Descending"), DefaultDescending);
		m_Views[context].AutoDirs = GetInt(_T("AutoDirs"), TRUE);
	}
	else
	{
		m_Views[context].Mode = DefaultMode;
		m_Views[context].SortBy = DefaultSortBy;
		m_Views[context].Descending = DefaultDescending;
		m_Views[context].AutoDirs = TRUE;
	}

	m_Views[context].GlobeLatitude = GetInt(_T("GlobeLatitude"), 1);
	m_Views[context].GlobeLongitude = GetInt(_T("GlobeLongitude"), 1);
	m_Views[context].GlobeZoom = GetInt(_T("GlobeZoom"), 600);
	m_Views[context].GlobeShowSpots = GetInt(_T("GlobeShowSpots"), TRUE);
	m_Views[context].GlobeShowAirportNames = GetInt(_T("GlobeShowAirportNames"), TRUE);
	m_Views[context].GlobeShowGPS = GetInt(_T("GlobeShowGPS"), TRUE);
	m_Views[context].GlobeShowDescription = GetInt(_T("GlobeShowDescription"), TRUE);
	m_Views[context].TagcloudCanonical = GetInt(_T("TagcloudSortCanonical"), TRUE);
	m_Views[context].TagcloudShowRare = GetInt(_T("TagcloudShowRare"), TRUE);
	m_Views[context].TagcloudUseSize = GetInt(_T("TagcloudUseSize"), TRUE);
	m_Views[context].TagcloudUseColors = GetInt(_T("TagcloudUseColors"), TRUE);
	m_Views[context].TagcloudUseOpacity = GetInt(_T("TagcloudUseOpacity"), FALSE);

	if ((m_Views[context].Mode>=LFViewCount) || (!m_AllowedViews[context]->IsSet(m_Views[context].Mode)))
		m_Views[context].Mode = DefaultMode;
	if (!m_Contexts[context]->AllowedAttributes->IsSet(m_Views[context].SortBy))
		m_Views[context].SortBy = DefaultSortBy;

	for (UINT a=0; a<LFAttributeCount; a++)
	{
		m_Views[context].ColumnOrder[a] = a;
		if (m_Contexts[context]->AllowedAttributes->IsSet(a) && (a!=LFAttrStoreID) && (a!=LFAttrFileID) && (a!=LFAttrFileFormat))
		{
			m_Views[context].ColumnWidth[a] = m_Attributes[a]->RecommendedWidth;
		}
		else
		{
			m_Views[context].ColumnWidth[a] = 0;
		}
	}
	GetBinary(_T("ColumnOrder"), &m_Views[context].ColumnOrder, sizeof(m_Views[context].ColumnOrder));
	GetBinary(_T("ColumnWidth"), &m_Views[context].ColumnWidth, sizeof(m_Views[context].ColumnWidth));

	for (UINT a=0; a<LFAttributeCount; a++)
		if (!m_Contexts[context]->AllowedAttributes->IsSet(a))
			m_Views[context].ColumnWidth[a] = 0;

	m_Views[context].AutoDirs &= (m_Contexts[context]->AllowGroups==true) || (context>=LFContextSubfolderDefault);

	SetRegistryBase(_T("Settings"));
}

void CStoreManagerApp::SaveViewOptions(UINT context)
{
	CString base;
	base.Format(_T("Settings\\Context%u"), context);
	SetRegistryBase(base);

	WriteInt(_T("Version"), ViewParametersVersion);
	WriteInt(_T("Mode"), m_Views[context].Mode);
	WriteInt(_T("SortBy"), m_Views[context].SortBy);
	WriteInt(_T("Descending"), m_Views[context].Descending);
	WriteInt(_T("AutoDirs"), m_Views[context].AutoDirs);
	WriteInt(_T("GlobeLatitude"), m_Views[context].GlobeLatitude);
	WriteInt(_T("GlobeLongitude"), m_Views[context].GlobeLongitude);
	WriteInt(_T("GlobeZoom"), m_Views[context].GlobeZoom);
	WriteInt(_T("GlobeShowSpots"), m_Views[context].GlobeShowSpots);
	WriteInt(_T("GlobeShowAirportNames"), m_Views[context].GlobeShowAirportNames);
	WriteInt(_T("GlobeShowGPS"), m_Views[context].GlobeShowGPS);
	WriteInt(_T("GlobeShowDescription"), m_Views[context].GlobeShowDescription);
	WriteInt(_T("TagcloudSortCanonical"), m_Views[context].TagcloudCanonical);
	WriteInt(_T("TagcloudShowRare"), m_Views[context].TagcloudShowRare);
	WriteInt(_T("TagcloudUseSize"), m_Views[context].TagcloudUseSize);
	WriteInt(_T("TagcloudUseColors"), m_Views[context].TagcloudUseColors);
	WriteInt(_T("TagcloudUseOpacity"), m_Views[context].TagcloudUseOpacity);

	WriteBinary(_T("ColumnOrder"), (LPBYTE)m_Views[context].ColumnOrder, sizeof(m_Views[context].ColumnOrder));
	WriteBinary(_T("ColumnWidth"), (LPBYTE)m_Views[context].ColumnWidth, sizeof(m_Views[context].ColumnWidth));

	SetRegistryBase(_T("Settings"));
}
