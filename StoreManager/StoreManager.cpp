
// StoreManager.cpp: Definiert das Klassenverhalten f�r die Anwendung.
//

#include "stdafx.h"
#include "afxwinappex.h"
#include "StoreManager.h"
#include "MainFrm.h"
#include "..\\LFCore\\resource.h"
#include "LFCore.h"
#include "LFCommDlg.h"


// CStoreManagerApp

BEGIN_MESSAGE_MAP(CStoreManagerApp, LFApplication)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_APP_NEWVIEW, OnAppNewView)
	ON_COMMAND(ID_APP_NEWCLIPBOARD, OnAppNewClipboard)
	ON_COMMAND(ID_APP_EXIT, OnAppExit)
END_MESSAGE_MAP()


// CStoreManagerApp-Erstellung

CStoreManagerApp::CStoreManagerApp()
	: LFApplication(HasGUI_Ribbon)
{
	ZeroMemory(&m_GLTextureCache, sizeof(m_GLTextureCache));
	ZeroMemory(&m_GLTextureBinds, sizeof(m_GLTextureBinds));

	// Pfad zu Google Earth
	path_GoogleEarth = "";

	HKEY hKey;
	if (RegOpenKeyExA(HKEY_CURRENT_USER, "SOFTWARE\\GOOGLE\\GOOGLE EARTH PLUS", 0, KEY_ALL_ACCESS, &hKey)==ERROR_SUCCESS)
	{
		DWORD dwType = REG_SZ;
		char lszValue[255];
		DWORD dwSize = 255;

		if (RegQueryValueExA(hKey, "InstallLocation", NULL, &dwType, (LPBYTE)&lszValue, &dwSize)==ERROR_SUCCESS)
			path_GoogleEarth = lszValue;

		RegCloseKey(hKey);
	}

	// Load icons
	HINSTANCE hModIcons = LoadLibrary(_T("LFCORE.DLL"));
	if (hModIcons)
	{
		ExtractCoreIcons(hModIcons, 128, &m_Icons128);
		ExtractCoreIcons(hModIcons, 64, &m_Icons64);
		ExtractCoreIcons(hModIcons, 48, &m_Icons48);
		ExtractCoreIcons(hModIcons, 24, &m_Icons24);
		ExtractCoreIcons(hModIcons, 16, &m_Icons16);

		FreeLibrary(hModIcons);
	}
}

CStoreManagerApp::~CStoreManagerApp()
{
}


// Das einzige CStoreManagerApp-Objekt

CStoreManagerApp theApp;


// CStoreManagerApp-Initialisierung

BOOL CStoreManagerApp::InitInstance()
{
	LFApplication::InitInstance();

	// Registry auslesen
	CString oldBase = GetRegistryBase();
	SetRegistryBase(_T("Settings"));
	m_HideEmptyDrives = GetInt(_T("HideEmptyDrives"), FALSE);
	m_HideEmptyDomains = GetInt(_T("HideEmptyDomains"), FALSE);
	m_GlobeHQModel = GetInt(_T("GlobeHQModel"), TRUE);
	m_GlobeLighting = GetInt(_T("GlobeLighting"), TRUE);
	m_ShowQueryDuration = GetInt(_T("ShowQueryDuration"), 0);
	m_nTextureSize = GetInt(_T("TextureSize"), 0);
	m_nMaxTextureSize = GetInt(_T("MaxTextureSize"), LFTexture8192);
	if (m_nTextureSize<0)
		m_nTextureSize = 0;
	if (m_nTextureSize>m_nMaxTextureSize)
		m_nTextureSize = m_nMaxTextureSize;

	for (UINT a=0; a<LFViewCount; a++)
		switch (a)
		{
		case LFViewGlobe:
			m_Background[a] = ChildBackground_Ribbon;
			break;
		default:
			m_Background[a] = (osInfo.dwMajorVersion>=6) ? ChildBackground_White : ChildBackground_System;
		}
	GetBinary(_T("Background"), m_Background, sizeof(m_Background));

	SetRegistryBase(oldBase);

	for (int a=0; a<LFContextCount; a++)
		LoadViewOptions(a);

	OnAppNewView();
	return TRUE;
}

int CStoreManagerApp::ExitInstance()
{
	CString oldBase = GetRegistryBase();
	SetRegistryBase(_T("Settings"));
	WriteInt(_T("HideEmptyDrives"), m_HideEmptyDrives);
	WriteInt(_T("HideEmptyDomains"), m_HideEmptyDomains);
	WriteInt(_T("GlobeHQModel"), m_GlobeHQModel);
	WriteInt(_T("GlobeLighting"), m_GlobeLighting);
	WriteInt(_T("ShowQueryDuration"), m_ShowQueryDuration);
	WriteInt(_T("TextureSize"), m_nTextureSize);
	WriteInt(_T("MaxTextureSize"), m_nMaxTextureSize);
	WriteBinary(_T("Background"), (LPBYTE)&m_Background, sizeof(m_Background));
	SetRegistryBase(oldBase);

	return LFApplication::ExitInstance();
}

void CStoreManagerApp::AddFrame(CMainFrame* pFrame)
{
	if (!m_pMainWnd)
		m_pMainWnd = pFrame;
	m_listMainFrames.push_back(pFrame);
	if (pFrame->IsClipboard)
		m_listClipboardFrames.push_back(pFrame);
}

void CStoreManagerApp::KillFrame(CMainFrame* pFrame)
{
	m_listMainFrames.remove(pFrame);
	if (pFrame->IsClipboard)
		m_listClipboardFrames.remove(pFrame);
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

CMainFrame* CStoreManagerApp::GetClipboard(BOOL ForceNew)
{
	if ((ForceNew) || (m_listClipboardFrames.empty()))
		OnAppNewClipboard();

	return m_listClipboardFrames.back();
}

void CStoreManagerApp::CloseAllFrames(BOOL leaveOne)
{
	MSG msg;

	// Nachrichten l�schen
	while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		DispatchMessage(&msg);

	std::list<CMainFrame*>::iterator ppFrame = m_listMainFrames.begin();
	while (ppFrame!=m_listMainFrames.end())
	{
		if ((*ppFrame)!=m_pMainWnd)
			(*ppFrame)->PostMessage(WM_CLOSE);
		ppFrame++ ;

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

void CStoreManagerApp::SetApplicationLook(UINT nID)
{
	LFApplication::SetApplicationLook(nID);

	// Alle Fenster neu zeichnen
	std::list<CMainFrame*>::iterator ppFrame = m_listMainFrames.begin();
	while (ppFrame!=m_listMainFrames.end())
	{
		(*ppFrame)->RedrawWindow(NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME | RDW_ERASE);
		(*ppFrame)->UpdateViewOptions();
		ppFrame++;
	}
}

void CStoreManagerApp::OnAppAbout()
{
	LFAboutDlgParameters p;
	p.appname = "StoreManager";
	p.build = __TIMESTAMP__;
	p.icon = new CGdiPlusBitmapResource();
	p.icon->Load(IDB_ABOUTICON, _T("PNG"), AfxGetResourceHandle());
	p.TextureSize = m_nTextureSize;
	p.MaxTextureSize = m_nMaxTextureSize;
	p.RibbonColor = m_nAppLook;
	p.HideEmptyDrives = m_HideEmptyDrives;
	p.HideEmptyDomains = m_HideEmptyDomains;

	LFAboutDlg dlg(&p, m_pActiveWnd);
	if (dlg.DoModal()==IDOK)
	{
		// Textur
		if (m_nTextureSize!=(UINT)p.TextureSize)
		{
			m_nTextureSize = p.TextureSize;
			UpdateViewOptions();
		}

		// Laufwerke
		if (p.HideEmptyDrives!=m_HideEmptyDrives)
		{
			m_HideEmptyDrives = p.HideEmptyDrives;
			Reload(LFContextStores);
		}

		// Domains
		if (p.HideEmptyDomains!=m_HideEmptyDomains)
		{
			m_HideEmptyDomains = p.HideEmptyDomains;
			Reload(LFContextStoreHome);
		}
	}

	delete p.icon;
}

void CStoreManagerApp::OnAppNewView()
{
	CMainFrame* pFrame = new CMainFrame();
	pFrame->LoadFrame(IDR_MAINFRAME, WS_OVERLAPPEDWINDOW);
	pFrame->ShowWindow(SW_SHOW);
}

void CStoreManagerApp::OnAppNewClipboard()
{
	CMainFrame* pFrame = new CMainFrame(TRUE);
	pFrame->LoadFrame(IDR_MAINFRAME, WS_OVERLAPPEDWINDOW);
	pFrame->ShowWindow(SW_SHOW);
}

void CStoreManagerApp::OnAppExit()
{
	CloseAllFrames();
	LFApplication::OnAppExit();
}

void CStoreManagerApp::OpenChildViews(int context, BOOL UpdateViewOptions)
{
	std::list<CMainFrame*>::iterator ppFrame = m_listMainFrames.begin();
	while (ppFrame!=m_listMainFrames.end())
	{
		if ((*ppFrame)->ActiveContextID==context)
		{
			(*ppFrame)->OpenChildView((*ppFrame)->GetFocusItem(), FALSE, TRUE);
		}
		else
		{
			if (UpdateViewOptions)
				(*ppFrame)->UpdateViewOptions();
		}
		ppFrame++;
	}
}

void CStoreManagerApp::UpdateViewOptions(int context)
{
	std::list<CMainFrame*>::iterator ppFrame = m_listMainFrames.begin();
	while (ppFrame!=m_listMainFrames.end())
	{
		if (((*ppFrame)->ActiveContextID==context) || (context==-1))
			(*ppFrame)->UpdateViewOptions();
		ppFrame++;
	}
}

void CStoreManagerApp::UpdateSortOptions(int context)
{
	std::list<CMainFrame*>::iterator ppFrame = m_listMainFrames.begin();
	while (ppFrame!=m_listMainFrames.end())
	{
		if ((*ppFrame)->ActiveContextID==context)
			(*ppFrame)->UpdateSortOptions();
		ppFrame++;
	}
}

void CStoreManagerApp::Reload(int context)
{
	std::list<CMainFrame*>::iterator ppFrame = m_listMainFrames.begin();
	while (ppFrame!=m_listMainFrames.end())
	{
		if ((*ppFrame)->ActiveContextID==context)
			(*ppFrame)->PostMessage(WM_COMMAND, ID_NAV_RELOAD);
		ppFrame++;
	}
}

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

void CStoreManagerApp::LoadViewOptions(int context)
{
	CString oldBase = GetRegistryBase();
	CString base;
	base.Format(_T("Settings\\Context%d"), context);
	SetRegistryBase(base);

	m_Views[context].Mode = GetInt(_T("Viewmode"), LFViewAutomatic);
	m_Views[context].GrannyMode = GetInt(_T("GrannyMode"), FALSE);
	m_Views[context].ShowCategories = GetInt(_T("ShowCategories"), TRUE);
	m_Views[context].FullRowSelect = GetInt(_T("FullRowSelect"), FALSE);
	m_Views[context].AlwaysSave = GetInt(_T("AlwaysSave"), TRUE);
	m_Views[context].SortBy = GetInt(_T("SortBy"), LFAttrFileName);
	m_Views[context].Descending = GetInt(_T("Descending"), FALSE);
	m_Views[context].AutoDirs = GetInt(_T("AutoDirs"), TRUE);
	m_Views[context].GlobeLatitude = GetInt(_T("GlobeLatitude"), 1);
	m_Views[context].GlobeLongitude = GetInt(_T("GlobeLongitude"), 1);
	m_Views[context].GlobeZoom = GetInt(_T("GlobeZoom"), 70);
	m_Views[context].GlobeShowBubbles = GetInt(_T("GlobeShowBubbles"), TRUE);
	m_Views[context].GlobeShowAirportNames = GetInt(_T("GlobeShowAirportNames"), TRUE);
	m_Views[context].GlobeShowGPS = GetInt(_T("GlobeShowGPS"), TRUE);
	m_Views[context].GlobeShowHints = GetInt(_T("GlobeShowHints"), TRUE);
	m_Views[context].GlobeShowSpots = GetInt(_T("GlobeShowSpots"), TRUE);
	m_Views[context].GlobeShowViewpoint = GetInt(_T("GlobeShowViewpoint"), FALSE);
	m_Views[context].TagcloudCanonical = GetInt(_T("TagcloudSortCanonical"), TRUE);
	m_Views[context].TagcloudOmitRare = GetInt(_T("TagcloudOmitRare"), FALSE);
	m_Views[context].TagcloudUseSize = GetInt(_T("TagcloudUseSize"), TRUE);
	m_Views[context].TagcloudUseColors = GetInt(_T("TagcloudUseColors"), TRUE);
	m_Views[context].TagcloudUseOpacity = GetInt(_T("TagcloudUseOpacity"), FALSE);
	m_Views[context].Changed = FALSE;

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

	if (!theApp.m_Contexts[context]->AllowedViews->IsSet(m_Views[context].Mode))
		m_Views[context].Mode = LFViewDetails;

	SetRegistryBase(oldBase);
}

void CStoreManagerApp::SaveViewOptions(int context, UINT SaveMode)
{
	CString oldBase = GetRegistryBase();
	CString base;
	base.Format(_T("Settings\\Context%d"), context);
	SetRegistryBase(base);

	WriteInt(_T("AlwaysSave"), m_Views[context].AlwaysSave);
	if ((m_Views[context].AlwaysSave) || (SaveMode==SaveMode_Force))
	{
		WriteInt(_T("Viewmode"), m_Views[context].Mode);
		WriteInt(_T("GrannyMode"), m_Views[context].GrannyMode);
		WriteInt(_T("ShowCategories"), m_Views[context].ShowCategories);
		WriteInt(_T("FullRowSelect"), m_Views[context].FullRowSelect);
		WriteInt(_T("SortBy"), m_Views[context].SortBy);
		WriteInt(_T("Descending"), m_Views[context].Descending);
		WriteInt(_T("AutoDirs"), m_Views[context].AutoDirs);
		WriteInt(_T("GlobeLatitude"), m_Views[context].GlobeLatitude);
		WriteInt(_T("GlobeLongitude"), m_Views[context].GlobeLongitude);
		WriteInt(_T("GlobeZoom"), m_Views[context].GlobeZoom);
		WriteInt(_T("GlobeShowBubbles"), m_Views[context].GlobeShowBubbles);
		WriteInt(_T("GlobeShowAirportNames"), m_Views[context].GlobeShowAirportNames);
		WriteInt(_T("GlobeShowGPS"), m_Views[context].GlobeShowGPS);
		WriteInt(_T("GlobeShowHints"), m_Views[context].GlobeShowHints);
		WriteInt(_T("GlobeShowSpots"), m_Views[context].GlobeShowSpots);
		WriteInt(_T("GlobeShowViewpoint"), m_Views[context].GlobeShowViewpoint);
		WriteInt(_T("TagcloudSortCanonical"), m_Views[context].TagcloudCanonical);
		WriteInt(_T("TagcloudOmitRare"), m_Views[context].TagcloudOmitRare);
		WriteInt(_T("TagcloudUseSize"), m_Views[context].TagcloudUseSize);
		WriteInt(_T("TagcloudUseColors"), m_Views[context].TagcloudUseColors);
		WriteInt(_T("TagcloudUseOpacity"), m_Views[context].TagcloudUseOpacity);

		WriteBinary(_T("ColumnOrder"), (LPBYTE)m_Views[context].ColumnOrder, sizeof(m_Views[context].ColumnOrder));
		WriteBinary(_T("ColumnWidth"), (LPBYTE)m_Views[context].ColumnWidth, sizeof(m_Views[context].ColumnWidth));

		m_Views[context].Changed = FALSE;
	}
	else
	{
		if (SaveMode!=SaveMode_FlagOnly)
			m_Views[context].Changed = TRUE;
	}

	SetRegistryBase(oldBase);
}

void CStoreManagerApp::ToggleAttribute(LFViewParameters* vp, UINT attr, int ColumnCount)
{
	int colId = 0;

	if (ColumnCount==-1)
	{
		ColumnCount++;
		for (UINT a=0; a<LFAttributeCount; a++)
		{
			if (a==attr)
				colId = ColumnCount;
			if (vp->ColumnWidth[a])
				ColumnCount++;
		}
	}

	vp->ColumnWidth[attr] = (vp->ColumnWidth[attr] ? 0 : theApp.m_Attributes[attr]->RecommendedWidth);
	if (vp->ColumnWidth[attr])
	{
		for (int a=0; a<ColumnCount; a++)
			if (vp->ColumnOrder[a]>=colId)
				vp->ColumnOrder[a]++;
		vp->ColumnOrder[ColumnCount] = colId;
	}
	else
	{
		int col = 0;
		for (int a=0; a<ColumnCount; a++)
			if (vp->ColumnOrder[a]==colId)
			{
				col = a;
				break;
			}

		for (int a=col; a<ColumnCount-1; a++)
			vp->ColumnOrder[a] = vp->ColumnOrder[a+1];
		for (int a=0; a<ColumnCount-1; a++)
			if (vp->ColumnOrder[a]>colId)
				vp->ColumnOrder[a]--;
	}
}

HBITMAP CStoreManagerApp::GetGLTexture(UINT nID)
{
	nID--;

	// Release all other textures
	for (UINT a=0; a<4; a++)
		if ((m_GLTextureCache[a]) && (!m_GLTextureBinds[a]) && (a!=nID))
		{
			DeleteObject(m_GLTextureCache[a]);
			m_GLTextureCache[a] = NULL;
		}

	if (!m_GLTextureCache[nID])
	{
		CGdiPlusBitmapResource* texture = new CGdiPlusBitmapResource();
		texture->Load(nID+IDB_BLUEMARBLE_1024, _T("PNG"));
		texture->m_pBitmap->GetHBITMAP(NULL, &m_GLTextureCache[nID]);
		delete texture;
	}

	m_GLTextureBinds[nID]++;
	return m_GLTextureCache[nID];
}

void CStoreManagerApp::FreeGLTexture(UINT nID)
{
	m_GLTextureBinds[nID-1]--;
}
