
// StoreManager.cpp: Definiert das Klassenverhalten für die Anwendung.
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
	if (RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\Google\\Google Earth Plus"), 0, KEY_ALL_ACCESS, &hKey)==ERROR_SUCCESS)
	{
		DWORD dwType = REG_SZ;
		CHAR lszValue[255];
		DWORD dwSize = 255;

		if (RegQueryValueEx(hKey, _T("InstallLocation"), NULL, &dwType, (LPBYTE)&lszValue, &dwSize)==ERROR_SUCCESS)
			path_GoogleEarth = lszValue;

		RegCloseKey(hKey);
	}

	// Nag screen
	m_NagCounter = 20;
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

	for (UINT a=0; a<LFContextCount; a++)
	{
		m_AllowedViews[a] = new LFBitArray(LFViewCount);

		UINT cnt = ((a>LFContextClipboard) && (a<LFContextSubfolderDefault)) ? LFViewCount-1 : (a>LFContextStoreHome) ? LFViewPreview : LFViewSearchResult;
		for (UINT b=0; b<=cnt; b++)
			if (b!=LFViewCalendarDay)
				(*m_AllowedViews[a]) += b;

		if (a==LFContextSubfolderDay)
			(*m_AllowedViews[a]) += LFViewCalendarDay;

		// TODO
		(*m_AllowedViews[a]) -= LFViewCalendarYear;
		(*m_AllowedViews[a]) -= LFViewTimeline;
	}

	// Registry auslesen
	CString oldBase = GetRegistryBase();
	SetRegistryBase(_T("Settings"));
	m_HideEmptyDrives = GetInt(_T("HideEmptyDrives"), FALSE);
	m_HideEmptyDomains = GetInt(_T("HideEmptyDomains"), FALSE);
	m_GlobeHQModel = GetInt(_T("GlobeHQModel"), TRUE);
	m_GlobeLighting = GetInt(_T("GlobeLighting"), TRUE);
	m_nTextureSize = GetInt(_T("TextureSize"), 0);
	m_nMaxTextureSize = GetInt(_T("MaxTextureSize"), LFTexture8192);
	if (m_nTextureSize<0)
		m_nTextureSize = 0;
	if (m_nTextureSize>m_nMaxTextureSize)
		m_nTextureSize = m_nMaxTextureSize;
	m_nAppLook = GetGlobalInt(_T("ApplicationLook"), ID_VIEW_APPLOOK_OFF_2007_BLUE);

	SetRegistryBase(oldBase);
	SetApplicationLook(m_nAppLook);

	for (INT a=0; a<LFContextCount; a++)
		LoadViewOptions(a);

	char StoreID[LFKeySize];
	char* RootStore = NULL;

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

void CStoreManagerApp::SetApplicationLook(UINT nID)
{
	if (nID!=m_nAppLook)
	{
		m_nAppLook = nID;
		WriteGlobalInt(_T("ApplicationLook"), nID);
	}

	switch (m_nAppLook)
	{
	case ID_VIEW_APPLOOK_OFF_2007_BLACK:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2007));
		CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_ObsidianBlack);
		break;
	case ID_VIEW_APPLOOK_OFF_2007_SILVER:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2007));
		CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Silver);
		break;
	case ID_VIEW_APPLOOK_OFF_2007_AQUA:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2007));
		CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Aqua);
		break;
#if (_MFC_VER>=0x1000)
	case ID_VIEW_APPLOOK_WINDOWS_7:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows7));
		break;
#endif
	default:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2007));
		CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_LunaBlue);
		break;
	}

	CDockingManager::SetDockingMode(DT_IMMEDIATE);

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
	CMainFrame* pFrame = new CMainFrame(NULL, TRUE);
	pFrame->LoadFrame(IDR_MAINFRAME, WS_OVERLAPPEDWINDOW);
	pFrame->ShowWindow(SW_SHOW);
}

void CStoreManagerApp::OnAppExit()
{
	CloseAllFrames();
	LFApplication::OnAppExit();
}

void CStoreManagerApp::OpenChildViews(INT context, BOOL UpdateViewOptions)
{
	std::list<CMainFrame*>::iterator ppFrame = m_listMainFrames.begin();
	while (ppFrame!=m_listMainFrames.end())
	{
		if ((*ppFrame)->ActiveContextID==context)
		{
			(*ppFrame)->OpenChildView(0, FALSE, TRUE);
		}
		else
		{
			if (UpdateViewOptions)
				(*ppFrame)->UpdateViewOptions();
		}
		ppFrame++;
	}
}

void CStoreManagerApp::UpdateViewOptions(INT context)
{
	std::list<CMainFrame*>::iterator ppFrame = m_listMainFrames.begin();
	while (ppFrame!=m_listMainFrames.end())
	{
		if (((*ppFrame)->ActiveContextID==context) || (context==-1))
			(*ppFrame)->UpdateViewOptions();
		ppFrame++;
	}
}

void CStoreManagerApp::UpdateSortOptions(INT context)
{
	std::list<CMainFrame*>::iterator ppFrame = m_listMainFrames.begin();
	while (ppFrame!=m_listMainFrames.end())
	{
		if ((*ppFrame)->ActiveContextID==context)
			(*ppFrame)->UpdateSortOptions();
		ppFrame++;
	}
}

void CStoreManagerApp::Reload(INT context)
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
	case LFContextClipboard:
		DefaultView = LFViewTiles;
		break;
	case LFContextSubfolderDay:
		DefaultView = LFViewCalendarDay;
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

	SetRegistryBase(oldBase);
}

void CStoreManagerApp::ToggleAttribute(LFViewParameters* vp, UINT attr, INT ColumnCount)
{
	INT colId = 0;

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
		for (INT a=0; a<ColumnCount; a++)
			if (vp->ColumnOrder[a]>=colId)
				vp->ColumnOrder[a]++;
		vp->ColumnOrder[ColumnCount] = colId;
	}
	else
	{
		INT col = 0;
		for (INT a=0; a<ColumnCount; a++)
			if (vp->ColumnOrder[a]==colId)
			{
				col = a;
				break;
			}

		for (INT a=col; a<ColumnCount-1; a++)
			vp->ColumnOrder[a] = vp->ColumnOrder[a+1];
		for (INT a=0; a<ColumnCount-1; a++)
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

void CStoreManagerApp::GetRibbonColors(COLORREF* back, COLORREF* text, COLORREF* highlight)
{
#if (_MFC_VER>=0x1000)
	if (m_nAppLook==ID_VIEW_APPLOOK_WINDOWS_7)
	{
		if (back)
			*back = (COLORREF)0xCAD4E3;
		if (text)
			*text = (COLORREF)0x000000;
		if (highlight)
			*highlight = (COLORREF)0x003399;
	}
	else
#endif
		switch (CMFCVisualManagerOffice2007::GetStyle())
		{
		case CMFCVisualManagerOffice2007::Office2007_Silver:
			if (back)
				*back = (COLORREF)0xDDD4D0;
			if (text)
				*text = (COLORREF)0x000000;
			if (highlight)
				*highlight = (COLORREF)0x5C534C;
			break;
		case CMFCVisualManagerOffice2007::Office2007_ObsidianBlack:
			if (back)
				*back = (COLORREF)0x535353;
			if (text)
				*text = (COLORREF)0xFFFFFF;
			if (highlight)
				*highlight = (COLORREF)0xFFFFFF;
			break;
		case CMFCVisualManagerOffice2007::Office2007_Aqua:
			if (back)
				*back = (COLORREF)0xD9CAC4;
			if (text)
				*text = (COLORREF)0x000000;
			if (highlight)
				*highlight = (COLORREF)0x6E1500;
			break;
		default:
			if (back)
				*back = (COLORREF)0xFFDBBF;
			if (text)
				*text = (COLORREF)0x000000;
			if (highlight)
				*highlight = (COLORREF)0x8B4215;
		}
}

CMFCRibbonButton* CStoreManagerApp::CommandButton(UINT nID, INT nSmallImageIndex, INT nLargeImageIndex, BOOL bAlwaysShowDescription, BOOL bInsertSpace)
{
	return new CMFCRibbonButton(nID, GetCommandName(nID, bInsertSpace), nSmallImageIndex, nLargeImageIndex, bAlwaysShowDescription);
}

CMFCRibbonCheckBox* CStoreManagerApp::CommandCheckBox(UINT nID)
{
	return new CMFCRibbonCheckBox(nID, GetCommandName(nID));
}
