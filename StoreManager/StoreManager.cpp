
// StoreManager.cpp: Definiert das Klassenverhalten für die Anwendung.
//

#include "stdafx.h"
#include "StoreManager.h"
#include "MainWnd.h"
#include "..\\LFCore\\resource.h"
#include "LFCore.h"
#include "LFCommDlg.h"
#include "MenuIcons.h"


// CStoreManagerApp

BEGIN_MESSAGE_MAP(CStoreManagerApp, LFApplication)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
END_MESSAGE_MAP()


// CStoreManagerApp-Erstellung

CStoreManagerApp::CStoreManagerApp()
	: LFApplication(TRUE)
{
	m_NagCounter = 20;
}


// Das einzige CStoreManagerApp-Objekt

CStoreManagerApp theApp;


// CStoreManagerApp-Initialisierung

BOOL CStoreManagerApp::InitInstance()
{
	LFApplication::InitInstance();

	for (UINT a=0; a<LFContextCount; a++)
	{
		m_AllowedViews[a] = new LFBitArray(LFViewCount);

		UINT cnt = ((a>LFContextClipboard) && (a<LFContextSubfolderDefault)) ? LFViewCount-1 : (a>LFContextStoreHome) ? LFViewPreview : LFViewContent;
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
	CString oldBase = GetRegistryBase();
	SetRegistryBase(_T("Settings"));
	m_ShowEmptyDrives = GetInt(_T("ShowEmptyDrives"), TRUE);
	m_ShowEmptyDomains = GetInt(_T("ShowEmptyDomains"), TRUE);
	m_ShowStatistics = GetInt(_T("ShowStatistics"), TRUE);
	m_CalendarShowStatistics = GetInt(_T("CalendarShowStatistics"), TRUE);
	m_CalendarShowCaptions = GetInt(_T("CalendarShowCaptions"), TRUE);
	m_CalendarShowEmptyDays = GetInt(_T("CalendarShowEmptyDays"), TRUE);
	m_GlobeHQModel = GetInt(_T("GlobeHQModel"), TRUE);
	m_GlobeLighting = GetInt(_T("GlobeLighting"), TRUE);
	m_GlobeAtmosphere = GetInt(_T("GlobeAtmosphere"), TRUE);
	m_GlobeShadows = GetInt(_T("GlobeShadows"), TRUE);
	m_GlobeBlackBackground = GetInt(_T("GlobeBlackBackground"), FALSE);
	m_GlobeShowViewport = GetInt(_T("GlobeShowViewport"), FALSE);
	m_GlobeShowCrosshairs = GetInt(_T("GlobeShowCrosshairs"), FALSE);
	m_TagcloudShowLegend = GetInt(_T("TagcloudShowLegend"), TRUE);
	m_nTextureSize = GetInt(_T("TextureSize"), 0);
	m_nMaxTextureSize = GetInt(_T("MaxTextureSize"), LFTexture4096);
	if (m_nTextureSize<0)
		m_nTextureSize = 0;
	if (m_nTextureSize>m_nMaxTextureSize)
		m_nTextureSize = m_nMaxTextureSize;

	SetRegistryBase(oldBase);

	for (INT a=0; a<LFContextCount; a++)
		LoadViewOptions(a);

	CHAR StoreID[LFKeySize];
	CHAR* RootStore = NULL;

	if (__argc==2)
		if (wcslen(__wargv[1])==LFKeySize-1)
		{
			WideCharToMultiByte(CP_ACP, 0, __wargv[1], -1, StoreID, LFKeySize, NULL, NULL);
			RootStore = StoreID;
		}

	CMainWnd* pFrame = new CMainWnd();
	pFrame->Create(FALSE, RootStore);
	pFrame->ShowWindow(SW_SHOW);

	return TRUE;
}

INT CStoreManagerApp::ExitInstance()
{
	for (INT a=0; a<LFContextCount; a++)
	{
		SaveViewOptions(a);
		delete m_AllowedViews[a];
	}

	CString oldBase = GetRegistryBase();
	SetRegistryBase(_T("Settings"));
	WriteInt(_T("ShowEmptyDrives"), m_ShowEmptyDrives);
	WriteInt(_T("ShowEmptyDomains"), m_ShowEmptyDomains);
	WriteInt(_T("ShowStatistics"), m_ShowStatistics);
	WriteInt(_T("CalendarShowStatistics"), m_CalendarShowStatistics);
	WriteInt(_T("CalendarShowCaptions"), m_CalendarShowCaptions);
	WriteInt(_T("CalendarShowEmptyDays"), m_CalendarShowEmptyDays);
	WriteInt(_T("GlobeHQModel"), m_GlobeHQModel);
	WriteInt(_T("GlobeLighting"), m_GlobeLighting);
	WriteInt(_T("GlobeAtmosphere"), m_GlobeAtmosphere);
	WriteInt(_T("GlobeShadows"), m_GlobeShadows);
	WriteInt(_T("GlobeBlackBackground"), m_GlobeBlackBackground);
	WriteInt(_T("GlobeShowViewport"), m_GlobeShowViewport);
	WriteInt(_T("GlobeShowCrosshairs"), m_GlobeShowCrosshairs);
	WriteInt(_T("TagcloudShowLegend"), m_TagcloudShowLegend);
	WriteInt(_T("TextureSize"), m_nTextureSize);
	WriteInt(_T("MaxTextureSize"), m_nMaxTextureSize);
	SetRegistryBase(oldBase);

	return LFApplication::ExitInstance();
}


void CStoreManagerApp::AddFrame(CMainWnd* pFrame)
{
	if (!m_pMainWnd)
		m_pMainWnd = pFrame;

	m_MainFrames.AddTail(pFrame);

	if (pFrame->m_IsClipboard)
		p_Clipboard = pFrame;
}

void CStoreManagerApp::KillFrame(CMainWnd* pVictim)
{
	if (pVictim->m_IsClipboard)
		p_Clipboard = NULL;

	for (POSITION p=m_MainFrames.GetHeadPosition(); p; )
	{
		POSITION pl = p;
		CMainWnd* pFrame = m_MainFrames.GetNext(p);
		if (pFrame==pVictim)
		{
			m_MainFrames.RemoveAt(pl);
		}
		else
		{
			m_pMainWnd = pFrame;
		}
	}
}

CMainWnd* CStoreManagerApp::GetClipboard()
{
	if (!p_Clipboard)
	{
		p_Clipboard = new CMainWnd();
		p_Clipboard->Create(TRUE);
		p_Clipboard->ShowWindow(SW_SHOW);
	}

	return p_Clipboard;
}

void CStoreManagerApp::CloseAllFrames(BOOL LeaveOne)
{
	MSG msg;

	// Nachrichten löschen
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		DispatchMessage(&msg);

	for (POSITION p=m_MainFrames.GetHeadPosition(); p; )
	{
		CMainWnd* pFrame = m_MainFrames.GetNext(p);
		if (pFrame!=m_pMainWnd)
			pFrame->PostMessage(WM_CLOSE);

		// Nachrichten löschen
		while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			DispatchMessage(&msg);
	}

	if ((m_pMainWnd) && (!LeaveOne))
		m_pMainWnd->PostMessage(WM_CLOSE);
}

void CStoreManagerApp::OnAppAbout()
{
	CString AppName;
	ENSURE(AppName.LoadString(IDR_APPLICATION));
	LFAbout(AppName, _T(__TIMESTAMP__), IDB_ABOUTICON, m_pActiveWnd);
}


HBITMAP CStoreManagerApp::SetContextMenuIcon(CMenu* pMenu, UINT CmdID, UINT ResID)
{
	HBITMAP res = NULL;
	INT cx = GetSystemMetrics(SM_CXSMICON);
	INT cy = GetSystemMetrics(SM_CYSMICON);

	for (UINT a=0; a<pMenu->GetMenuItemCount(); a++)
		if (pMenu->GetMenuItemID(a)==CmdID)
		{
			HMODULE hModCore = LoadLibrary(_T("LFCORE.DLL"));
			if (hModCore)
			{
				HICON hIcon = (HICON)LoadImage(hModCore, MAKEINTRESOURCE(ResID), IMAGE_ICON, cx, cy, LR_DEFAULTCOLOR);
				FreeLibrary(hModCore);

				res = SetMenuItemIcon(*pMenu, a, hIcon, cx, cy);
				DestroyIcon(hIcon);
			}
		}

	return res;
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
	{
		for (UINT a=0; a<LFAttributeCount; a++)
			if (AttributeSortableInView(a, vp->Mode))
			{
				vp->SortBy = a;
				Modified = TRUE;
				break;
			}
	}

	return Modified;
}

void CStoreManagerApp::Broadcast(INT Context, INT View, UINT cmdMsg)
{
	for (POSITION p=m_MainFrames.GetHeadPosition(); p; )
	{
		CMainWnd* pFrame = m_MainFrames.GetNext(p);
		if ((pFrame->ActiveContextID==Context) || (Context==-1))
			if ((pFrame->ActiveViewID==View) || (View==-1))
				pFrame->PostMessage(cmdMsg);
	}
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

void CStoreManagerApp::UpdateFooter(INT Context, INT View)
{
	Broadcast(Context, View, WM_UPDATEFOOTER);
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

void CStoreManagerApp::LoadViewOptions(INT context)
{
	CString oldBase = GetRegistryBase();
	CString base;
	base.Format(_T("Settings\\Context%d"), context);
	SetRegistryBase(base);

	UINT DefaultView;
	switch (context)
	{
	case LFContextStores:
		DefaultView = LFViewLargeIcons;
		break;
	case LFContextStoreHome:
		DefaultView = LFViewList;
		break;
	case LFContextClipboard:
		DefaultView = LFViewContent;
		break;
	case LFContextHousekeeping:
	case LFContextTrash:
		DefaultView = LFViewTiles;
		break;
	default:
		DefaultView = LFViewDetails;
	}

	m_Views[context].Mode = GetInt(_T("Viewmode"), DefaultView);
	m_Views[context].SortBy = GetInt(_T("SortBy"), LFAttrFileName);
	m_Views[context].Descending = GetInt(_T("Descending"), FALSE);
	m_Views[context].AutoDirs = GetInt(_T("AutoDirs"), TRUE);
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

	if ((m_Views[context].Mode>=LFViewCount) || (!theApp.m_AllowedViews[context]->IsSet(m_Views[context].Mode)))
			m_Views[context].Mode = DefaultView;

	for (UINT a=0; a<LFAttributeCount; a++)
	{
		m_Views[context].ColumnOrder[a] = a;
		if (m_Contexts[context]->AllowedAttributes->IsSet(a) && (a!=LFAttrStoreID) && (a!=LFAttrFileID) && (a!=LFAttrFileFormat) && (a!=LFAttrFileCount))
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

	m_Views[context].AutoDirs &= (m_Contexts[context]->AllowGroups==true) || (context>=LFContextSubfolderDefault);

	SetRegistryBase(oldBase);
}

void CStoreManagerApp::SaveViewOptions(INT context)
{
	CString oldBase = GetRegistryBase();
	CString base;
	base.Format(_T("Settings\\Context%d"), context);
	SetRegistryBase(base);

	WriteInt(_T("Viewmode"), m_Views[context].Mode);
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

	SetRegistryBase(oldBase);
}
