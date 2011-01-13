
// StoreManager.cpp: Definiert das Klassenverhalten für die Anwendung.
//

#include "stdafx.h"
#include "afxwinappex.h"
#include "StoreManager.h"
#include "MainFrm.h"
#include "..\\LFCore\\resource.h"
#include "LFCore.h"
#include "LFCommDlg.h"
#include "MenuIcons.h"


// CStoreManagerApp

BEGIN_MESSAGE_MAP(CStoreManagerApp, LFApplication)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_APP_NEWVIEW, OnAppNewView)
	ON_COMMAND(ID_APP_EXIT, OnAppExit)
END_MESSAGE_MAP()


// CStoreManagerApp-Erstellung

CStoreManagerApp::CStoreManagerApp()
	: LFApplication(HasGUI_Ribbon)
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

		UINT cnt = ((a>LFContextClipboard) && (a<LFContextSubfolderDefault)) ? LFViewCount-1 : (a>LFContextStoreHome) ? LFViewPreview : LFViewSearchResult;
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
	m_HideEmptyDrives = GetInt(_T("HideEmptyDrives"), FALSE);
	m_HideEmptyDomains = GetInt(_T("HideEmptyDomains"), FALSE);
	m_GlobeHQModel = GetInt(_T("GlobeHQModel"), TRUE);
	m_GlobeLighting = GetInt(_T("GlobeLighting"), TRUE);
	m_GlobeAtmosphere = GetInt(_T("GlobeAtmosphere"), TRUE);
	m_GlobeShadows = GetInt(_T("GlobeShadows"), TRUE);
	m_nTextureSize = GetInt(_T("TextureSize"), 0);
	m_nMaxTextureSize = GetInt(_T("MaxTextureSize"), LFTexture4096);
	if (m_nTextureSize<0)
		m_nTextureSize = 0;
	if (m_nTextureSize>m_nMaxTextureSize)
		m_nTextureSize = m_nMaxTextureSize;

	SetRegistryBase(oldBase);

	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2007));
	CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_LunaBlue);
	CDockingManager::SetDockingMode(DT_IMMEDIATE);

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

	CMainFrame* pFrame = new CMainFrame(RootStore);
	pFrame->LoadFrame(IDR_MAINFRAME, WS_OVERLAPPEDWINDOW);
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
	WriteInt(_T("HideEmptyDrives"), m_HideEmptyDrives);
	WriteInt(_T("HideEmptyDomains"), m_HideEmptyDomains);
	WriteInt(_T("GlobeHQModel"), m_GlobeHQModel);
	WriteInt(_T("GlobeLighting"), m_GlobeLighting);
	WriteInt(_T("GlobeAtmosphere"), m_GlobeAtmosphere);
	WriteInt(_T("GlobeShadows"), m_GlobeShadows);
	WriteInt(_T("TextureSize"), m_nTextureSize);
	WriteInt(_T("MaxTextureSize"), m_nMaxTextureSize);
	SetRegistryBase(oldBase);

	return LFApplication::ExitInstance();
}


void CStoreManagerApp::AddFrame(CMainFrame* pFrame)
{
	if (!m_pMainWnd)
		m_pMainWnd = pFrame;
	m_listMainFrames.push_back(pFrame);

	if (pFrame->IsClipboard)
		p_Clipboard = pFrame;
}

void CStoreManagerApp::KillFrame(CMainFrame* pFrame)
{
	m_listMainFrames.remove(pFrame);

	if (pFrame->IsClipboard)
		p_Clipboard = NULL;
}

void CStoreManagerApp::ReplaceMainFrame(CMainFrame* pFrame)
{
	if (pFrame!=m_pMainWnd)
		return;

	std::list<CMainFrame*>::iterator ppFrame = m_listMainFrames.begin();
	while (ppFrame!=m_listMainFrames.end())
	{
		if (*ppFrame!=pFrame)
		{
			m_pMainWnd = *ppFrame;
			break;
		}
		ppFrame++;
	}
}

CMainFrame* CStoreManagerApp::GetClipboard()
{
	if (!p_Clipboard)
	{
		p_Clipboard = new CMainFrame(NULL, TRUE);
		p_Clipboard->LoadFrame(IDR_MAINFRAME, WS_OVERLAPPEDWINDOW);
		p_Clipboard->ShowWindow(SW_SHOW);
	}

	return p_Clipboard;
}

void CStoreManagerApp::CloseAllFrames(BOOL leaveOne)
{
	MSG msg;

	// Nachrichten löschen
	while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		DispatchMessage(&msg);

	std::list<CMainFrame*>::iterator ppFrame = m_listMainFrames.begin();
	while (ppFrame!=m_listMainFrames.end())
	{
		if ((*ppFrame)!=m_pMainWnd)
			(*ppFrame)->PostMessage(WM_CLOSE);
		ppFrame++;

		while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			DispatchMessage(&msg);
	}

	if ((m_pMainWnd) && (!leaveOne))
		m_pMainWnd->PostMessage(WM_CLOSE);
}

void CStoreManagerApp::OnClosingMainFrame(CFrameImpl* pFrame)
{
	if (!((CMainFrame*)(pFrame->GetFrameList().GetHead()))->IsClipboard)
		SaveState(NULL, pFrame);
}

void CStoreManagerApp::OnAppAbout()
{
	LFAbout(_T("StoreManager"), _T(__TIMESTAMP__), IDB_ABOUTICON, m_pActiveWnd);
}

void CStoreManagerApp::OnAppNewView()
{
	CMainFrame* pFrame = new CMainFrame();
	pFrame->LoadFrame(IDR_MAINFRAME, WS_OVERLAPPEDWINDOW);
	pFrame->ShowWindow(SW_SHOW);
}

void CStoreManagerApp::OnAppExit()
{
	CloseAllFrames();
	LFApplication::OnAppExit();
}


void CStoreManagerApp::SetContextMenuIcon(CMenu* pMenu, UINT CmdID, UINT ResID)
{
	INT cx = GetSystemMetrics((OSVersion==OS_XP) ? SM_CXMENUCHECK : SM_CXSMICON);
	INT cy = GetSystemMetrics((OSVersion==OS_XP) ? SM_CYMENUCHECK : SM_CYSMICON);

	for (UINT a=0; a<pMenu->GetMenuItemCount(); a++)
		if (pMenu->GetMenuItemID(a)==CmdID)
		{
			HMODULE hModCore = LoadLibrary(_T("LFCORE.DLL"));
			if (hModCore)
			{
				HICON hIcon = (HICON)LoadImage(hModCore, MAKEINTRESOURCE(ResID), IMAGE_ICON, cx, cy, LR_DEFAULTCOLOR);
				FreeLibrary(hModCore);

				SetMenuItemBitmaps(*pMenu, a, MF_BYPOSITION, IconToBitmap(hIcon, cx, cy), NULL);
				DestroyIcon(hIcon);
			}
		}
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
		for (UINT a=0; a<=LFViewTimeline; a++)
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

void CStoreManagerApp::Broadcast(INT context, UINT cmdMsg)
{
	std::list<CMainFrame*>::iterator ppFrame = m_listMainFrames.begin();
	while (ppFrame!=m_listMainFrames.end())
	{
		if (((*ppFrame)->ActiveContextID==context) || (context==-1))
			(*ppFrame)->PostMessage(cmdMsg);

		ppFrame++;
	}
}

void CStoreManagerApp::UpdateSortOptions(INT context)
{
	SanitizeSortBy(&theApp.m_Views[context], context);
	Broadcast(context, WM_UPDATESORTOPTIONS);
}

void CStoreManagerApp::UpdateViewOptions(INT context)
{
	BOOL Modified = (context!=-1) ? SanitizeViewMode(&theApp.m_Views[context], context) : FALSE;
	Broadcast(context, Modified ? WM_UPDATESORTOPTIONS : WM_UPDATEVIEWOPTIONS);
}

void CStoreManagerApp::Reload(INT context)
{
	Broadcast(context, WM_RELOAD);
}

void CStoreManagerApp::PrepareFormatData(CHAR* FileFormat)
{
	if (m_FileFormats.count(FileFormat))
		return;

	CString Ext = _T("*.");
	Ext += FileFormat;

	SHFILEINFO sfi;
	if (SUCCEEDED(SHGetFileInfo(Ext, 0, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_TYPENAME | SHGFI_USEFILEATTRIBUTES)))
	{
		FormatData fd;
		fd.SysIconIndex = sfi.iIcon;
		wcscpy_s(fd.FormatName, 80, sfi.szTypeName);

		m_FileFormats[FileFormat] = fd;
	}
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
		DefaultView = LFViewSearchResult;
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
	m_Views[context].GlobeZoom = GetInt(_T("GlobeZoom"), 70);
	m_Views[context].GlobeShowSpots = GetInt(_T("GlobeShowSpots"), TRUE);
	m_Views[context].GlobeShowAirportNames = GetInt(_T("GlobeShowAirportNames"), TRUE);
	m_Views[context].GlobeShowGPS = GetInt(_T("GlobeShowGPS"), TRUE);
	m_Views[context].GlobeShowDescription = GetInt(_T("GlobeShowDescription"), TRUE);
	m_Views[context].GlobeShowViewport = GetInt(_T("GlobeShowViewport"), FALSE);
	m_Views[context].GlobeShowCrosshair = GetInt(_T("GlobeShowCrosshair"), TRUE);
	m_Views[context].TagcloudCanonical = GetInt(_T("TagcloudSortCanonical"), TRUE);
	m_Views[context].TagcloudHideRare = GetInt(_T("TagcloudHideRare"), FALSE);
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
	WriteInt(_T("GlobeShowViewport"), m_Views[context].GlobeShowViewport);
	WriteInt(_T("GlobeShowCrosshair"), m_Views[context].GlobeShowCrosshair);
	WriteInt(_T("TagcloudSortCanonical"), m_Views[context].TagcloudCanonical);
	WriteInt(_T("TagcloudHideRare"), m_Views[context].TagcloudHideRare);
	WriteInt(_T("TagcloudUseSize"), m_Views[context].TagcloudUseSize);
	WriteInt(_T("TagcloudUseColors"), m_Views[context].TagcloudUseColors);
	WriteInt(_T("TagcloudUseOpacity"), m_Views[context].TagcloudUseOpacity);

	WriteBinary(_T("ColumnOrder"), (LPBYTE)m_Views[context].ColumnOrder, sizeof(m_Views[context].ColumnOrder));
	WriteBinary(_T("ColumnWidth"), (LPBYTE)m_Views[context].ColumnWidth, sizeof(m_Views[context].ColumnWidth));

	SetRegistryBase(oldBase);
}


// Ribbon slack

CString CStoreManagerApp::GetCommandName(UINT nID, BOOL bInsertSpace)
{
	CString tmpStr = _T("?");
	tmpStr.LoadString(nID);

	INT pos = tmpStr.Find(L'\n');
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

CMFCRibbonButton* CStoreManagerApp::CommandButton(UINT nID, INT nSmallImageIndex, INT nLargeImageIndex, BOOL bAlwaysShowDescription, BOOL bInsertSpace)
{
	return new CMFCRibbonButton(nID, GetCommandName(nID, bInsertSpace), nSmallImageIndex, nLargeImageIndex, bAlwaysShowDescription);
}
