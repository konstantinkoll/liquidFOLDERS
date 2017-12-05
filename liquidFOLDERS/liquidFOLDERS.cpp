
// liquidFOLDERS.cpp: Definiert das Klassenverhalten für die Anwendung.
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
				wcscpy_s(cdsw.Command, MAX_PATH, (LPCWSTR)lParam);

			COPYDATASTRUCT cds;
			cds.cbData = sizeof(cdsw);
			cds.lpData = &cdsw;
			if (SendMessage(hWnd, WM_COPYDATA, NULL, (LPARAM)&cds))
				return FALSE;
		}

	return TRUE;
}


// CLiquidFoldersApp

CLiquidFoldersApp theApp;

CLiquidFoldersApp::CLiquidFoldersApp()
	: LFApplication(theAppID)
{
	p_ClipboardWnd = NULL;
	m_AppInitialized = FALSE;
}


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

	// RestartManager
	if (m_KernelLibLoaded)
		zRegisterApplicationRestart(L"", 8);	// RESTART_NO_REBOOT

	// Pfad zu Google Earth
	DWORD dwSize = sizeof(m_PathGoogleEarth)/sizeof(WCHAR);
	if (FAILED(AssocQueryString(ASSOCF_REMAPRUNDLL | ASSOCF_INIT_IGNOREUNKNOWN, ASSOCSTR_EXECUTABLE, L".kml", NULL, m_PathGoogleEarth, &dwSize)))
		m_PathGoogleEarth[0] = L'\0';

	// Registry auslesen
	SetRegistryBase(_T("Settings"));

	const BOOL ResetViewSettings = LoadGlobalViewSettings();

	for (UINT a=0; a<LFContextCount; a++)
		LoadContextViewSettings(a, ResetViewSettings);

	m_ModelQuality = (GLModelQuality)GetInt(_T("ModelQuality"), MODELULTRA);
	m_TextureQuality = (GLTextureQuality)GetInt(_T("TextureQuality"), TEXTUREMEDIUM);
	m_TextureCompress = GetInt(_T("TextureCompress"), FALSE);

	m_ShowInspectorPane = GetInt(_T("ShowInspectorPane"), TRUE);
	m_InspectorPaneWidth = GetInt(_T("InspectorPaneWidth"), 220);
	if (m_InspectorPaneWidth<140)
		m_InspectorPaneWidth = 140;

	m_FileDropAlwaysOnTop = GetInt(_T("FileDropAlwaysOnTop"), TRUE);

	// Icon factory
	m_IconFactory.m_ApplicationIcons.Load(IDB_APPLICATIONS_29, CSize(31, 32));

	// Execute
	CheckForUpdate();
	OpenCommandLine(__argc>1 ? CmdLine : NULL);

	m_AppInitialized = TRUE;

	return TRUE;
}

CWnd* CLiquidFoldersApp::OpenCommandLine(LPWSTR pCmdLine)
{
	// Parse parameter and create window
	if (pCmdLine)
	{
		// Prepare arguments
		WCHAR* pChar = pCmdLine;
		while (*pChar)
			*(pChar++) = (WCHAR)toupper(*pChar);

		CHAR StoreID[LFKeySize] = "";

		WCHAR* pSpace = wcschr(pCmdLine, L' ');
		if (pSpace)
		{
			WideCharToMultiByte(CP_ACP, 0, pSpace+1, -1, StoreID, LFKeySize, NULL, NULL);
			*pSpace = L'\0';
		}

		// Update
		if (wcscmp(pCmdLine, L"/CHECKUPDATE")==0)
			return NULL;

		// FileDrop
		if (wcscmp(pCmdLine, L"/FILEDROP")==0)
		{
			const UINT Result = (StoreID[0]=='\0') ? LFGetDefaultStore(StoreID) : LFOk;

			switch (Result)
			{
			case LFOk:
				return GetFileDrop(StoreID);

			case LFNoDefaultStore:
				goto OpenRootWindow;

			default:
				LFErrorBox(CWnd::GetForegroundWindow(), Result);

				return NULL;
			}
		}

		// Key or IATA code
		if ((wcschr(pCmdLine, L'.')==NULL) && (wcschr(pCmdLine, L':')==NULL) && (wcschr(pCmdLine, L'\\')==NULL) && (wcschr(pCmdLine, L'/')==NULL))
		{
			// Key
			if (wcslen(pCmdLine)==LFKeySize-1)
			{
				WideCharToMultiByte(CP_ACP, 0, pCmdLine, -1, StoreID, LFKeySize, NULL, NULL);

				CMainWnd* pFrameWnd = new CMainWnd();
				pFrameWnd->CreateStore(StoreID);
				pFrameWnd->ShowWindow(SW_SHOW);

				return pFrameWnd;
			}

			// IATA airport code
			if (wcslen(pCmdLine)==3)
			{
				CHAR Code[4];
				WideCharToMultiByte(CP_ACP, 0, pCmdLine, -1, Code, 4, NULL, NULL);

				LFVariantData VData;
				LFInitVariantData(VData, LFAttrLocationIATA);

				strcpy_s(VData.AnsiString, 256, Code);
				VData.IsNull = FALSE;

				LFFilter* pFilter = LFAllocFilter();
				pFilter->Mode = LFFilterModeSearch;
				pFilter->pConditionList = LFAllocFilterCondition(LFFilterCompareIsEqual, VData);

				LFAirport* pAirport;
				if (LFIATAGetAirportByCode(Code, pAirport))
					MultiByteToWideChar(CP_ACP, 0, pAirport->Name, -1, pFilter->OriginalName, 256);

				CMainWnd* pFrameWnd = new CMainWnd();
				pFrameWnd->CreateFilter(pFilter);
				pFrameWnd->ShowWindow(SW_SHOW);

				return pFrameWnd;
			}
		}

		// Filter
		CMainWnd* pFrameWnd = new CMainWnd();
		pFrameWnd->CreateFilter(LFLoadFilterEx(pCmdLine));
		pFrameWnd->ShowWindow(SW_SHOW);

		return pFrameWnd;
	}

	// Root
OpenRootWindow:
	CMainWnd* pFrameWnd = new CMainWnd();
	pFrameWnd->CreateRoot();
	pFrameWnd->ShowWindow(SW_SHOW);

	return pFrameWnd;
}

INT CLiquidFoldersApp::ExitInstance()
{
	if (m_AppInitialized)
	{
		for (UINT a=0; a<LFContextCount; a++)
			SaveContextViewSettings(a);

		SetRegistryBase(_T("Settings"));

		SaveGlobalViewSettings();

		WriteInt(_T("ModelQuality"), m_ModelQuality);
		WriteInt(_T("TextureQuality"), m_TextureQuality);
		WriteInt(_T("TextureCompress"), m_TextureCompress);

		WriteInt(_T("ShowInspectorPane"), m_ShowInspectorPane);
		WriteInt(_T("InspectorPaneWidth"), m_InspectorPaneWidth);

		WriteInt(_T("FileDropAlwaysOnTop"), m_FileDropAlwaysOnTop);
	}

	return LFApplication::ExitInstance();
}


CMainWnd* CLiquidFoldersApp::GetClipboard()
{
	if (!p_ClipboardWnd)
	{
		p_ClipboardWnd = new CMainWnd();
		p_ClipboardWnd->CreateClipboard();
		p_ClipboardWnd->ShowWindow(SW_SHOW);
	}

	return p_ClipboardWnd;
}

CWnd* CLiquidFoldersApp::GetFileDrop(LPCSTR StoreID)
{
	for (POSITION p=m_pMainFrames.GetHeadPosition(); p; )
	{
		CWnd* pFrameWnd = m_pMainFrames.GetNext(p);
		if (pFrameWnd->SendMessage(WM_OPENFILEDROP, (WPARAM)StoreID)==24878)
			return pFrameWnd;
	}

	CFileDropWnd* pFrameWnd = new CFileDropWnd();
	pFrameWnd->Create(StoreID);
	pFrameWnd->ShowWindow(SW_SHOW);

	return pFrameWnd;
}


void CLiquidFoldersApp::SanitizeContextViewSettings(INT Context)
{
	ASSERT(Context>=0);
	ASSERT(Context<LFContextCount);

	// Find view for this attribute that is allowed in the context
	const UINT Attr = m_ContextViewSettings[Context].SortBy;

	ASSERT(IsAttributeSortable(Context, Attr));
	ASSERT(m_Attributes[Attr].AttrProperties.DefaultView!=(UINT)-1);

	UINT Mask = m_Attributes[Attr].TypeProperties.AllowedViews & m_Contexts[Context].CtxProperties.AvailableViews;
	ASSERT(Mask!=0);

	UINT View = m_ContextViewSettings[Context].View;

	// If view is not available, try default view
	if ((Mask & (1<<m_ContextViewSettings[Context].View))==0)
		View = m_Attributes[Attr].AttrProperties.DefaultView;

	// If view is still not available, use first one that matches
	if ((Mask & (1<<View))==0)
		for (View=0; Mask && ((Mask & 1)==0); Mask>>=1, View++);

	// Set view
	m_ContextViewSettings[Context].View = View;
}

void CLiquidFoldersApp::Broadcast(INT Context, INT View, UINT cmdMsg)
{
	ASSERT(Context>=-1);
	ASSERT(Context<LFContextCount);

	for (POSITION p=m_pMainFrames.GetHeadPosition(); p; )
		m_pMainFrames.GetNext(p)->PostMessage(WM_CONTEXTVIEWCOMMAND, cmdMsg, MAKELPARAM(Context, View));
}

void CLiquidFoldersApp::SetContextSort(INT Context, UINT Attr, BOOL SortDescending, BOOL SetLastView)
{
	ASSERT(Context>=0);
	ASSERT(Context<LFContextCount);
	ASSERT(Attr>=0);
	ASSERT(Attr<LFAttributeCount);
	ASSERT(IsAttributeSortable(Context, Attr));
	ASSERT(m_Attributes[Attr].AttrProperties.DefaultView!=(UINT)-1);

	if (m_ContextViewSettings[Context].SortBy!=Attr)
	{
		m_ContextViewSettings[Context].SortBy = Attr;

		if (SetLastView)
			m_ContextViewSettings[Context].View = m_GlobalViewSettings.LastViewSelected[Attr];
	}

	m_ContextViewSettings[Context].SortDescending = SortDescending;

	SanitizeContextViewSettings(Context);

	Broadcast(Context, -1, WM_UPDATESORTSETTINGS);
}

void CLiquidFoldersApp::UpdateViewSettings(INT Context, INT View)
{
	ASSERT(Context>=-1);
	ASSERT(Context<LFContextCount);

	Broadcast(Context, View, WM_UPDATEVIEWSETTINGS);
}

void CLiquidFoldersApp::SetContextView(INT Context, INT View)
{
	ASSERT(Context>=0);
	ASSERT(Context<LFContextCount);
	ASSERT(View>=0);
	ASSERT(View<=LFViewCount);

	if (View!=(INT)m_ContextViewSettings[Context].View)
	{
		ASSERT(IsViewAllowed(Context, View));

		m_GlobalViewSettings.LastViewSelected[m_ContextViewSettings[Context].SortBy] =
			m_ContextViewSettings[Context].View = View;

		UpdateViewSettings(Context);
	}
}


// Registry and view settings

void CLiquidFoldersApp::LoadContextViewSettings(UINT Context, BOOL Reset)
{
	CString Base;
	Base.Format(_T("Settings\\Context%u"), Context);
	SetRegistryBase(Base);

	const UINT DefaultAttribute = m_Contexts[Context].CtxProperties.DefaultAttribute;
	const BOOL SortDescending = theApp.IsAttributeSortDescending(Context, DefaultAttribute);
	const UINT DefaultView = m_Contexts[Context].CtxProperties.DefaultView;

	// Default columns
	memcpy_s(&m_ContextViewSettings[Context].ColumnOrder, LFAttributeCount*sizeof(INT), m_SortedAttributeList, sizeof(LFAttributeList));

	for (UINT a=0; a<LFAttributeCount; a++)
		m_ContextViewSettings[Context].ColumnWidth[a] = (IsAttributeAlwaysVisible(a) || IsAttributeAdvertised(Context, a)) ? m_Attributes[a].TypeProperties.DefaultColumnWidth : 0;

	// Double the width of file name and title
	m_ContextViewSettings[Context].ColumnWidth[LFAttrFileName] <<= 1;
	m_ContextViewSettings[Context].ColumnWidth[LFAttrTitle] <<= 1;

	// Read settings
	if (!Reset)
	{
		m_ContextViewSettings[Context].SortBy = GetInt(_T("SortBy"), DefaultAttribute);
		m_ContextViewSettings[Context].SortDescending = GetInt(_T("SortDescending"), SortDescending);
		m_ContextViewSettings[Context].View = GetInt(_T("View"), DefaultView);

		Reset |= !IsAttributeAvailable(Context, m_ContextViewSettings[Context].SortBy) ||
			(m_ContextViewSettings[Context].View>=LFViewCount) || !IsViewAllowed(Context, m_ContextViewSettings[Context].View);
	}

	// Reset settings
	if (Reset)
	{
		m_ContextViewSettings[Context].SortBy = DefaultAttribute;
		m_ContextViewSettings[Context].SortDescending = SortDescending;
		m_ContextViewSettings[Context].View = DefaultView;
	}
	else
	{
		GetBinary(_T("ColumnOrder"), &m_ContextViewSettings[Context].ColumnOrder, sizeof(m_ContextViewSettings[Context].ColumnOrder));
		GetBinary(_T("ColumnWidth"), &m_ContextViewSettings[Context].ColumnWidth, sizeof(m_ContextViewSettings[Context].ColumnWidth));

		for (UINT a=0; a<LFAttributeCount; a++)
		{
			if (!IsAttributeAvailable(Context, a))
			{
				m_ContextViewSettings[Context].ColumnWidth[a] = 0;
			}
			else
				if (theApp.IsAttributeAlwaysVisible(a) && (m_ContextViewSettings[Context].ColumnWidth[a]==0))
				{
					m_ContextViewSettings[Context].ColumnWidth[a] = m_Attributes[a].TypeProperties.DefaultColumnWidth;
				}
		}
	}

	SetRegistryBase(_T("Settings"));
}

void CLiquidFoldersApp::SaveContextViewSettings(UINT Context)
{
	CString Base;
	Base.Format(_T("Settings\\Context%u"), Context);
	SetRegistryBase(Base);

#ifndef _DEBUG
	WriteInt(_T("SortBy"), m_ContextViewSettings[Context].SortBy);
	WriteInt(_T("SortDescending"), m_ContextViewSettings[Context].SortDescending);
	WriteInt(_T("View"), m_ContextViewSettings[Context].View);

	WriteBinary(_T("ColumnOrder"), (LPBYTE)m_ContextViewSettings[Context].ColumnOrder, sizeof(m_ContextViewSettings[Context].ColumnOrder));
	WriteBinary(_T("ColumnWidth"), (LPBYTE)m_ContextViewSettings[Context].ColumnWidth, sizeof(m_ContextViewSettings[Context].ColumnWidth));
#endif

	SetRegistryBase(_T("Settings"));
}

BOOL CLiquidFoldersApp::LoadGlobalViewSettings()
{
	BOOL Reset = GetInt(_T("ViewSettingsVersion"), 0)!=VIEWSETTINGSVERSION;

	for (UINT a=0; a<LFAttributeCount; a++)
		m_GlobalViewSettings.LastViewSelected[a] = m_Attributes[a].AttrProperties.DefaultView;

	if (!Reset)
		GetBinary(_T("LastViewSelected"), &m_GlobalViewSettings.LastViewSelected, sizeof(m_GlobalViewSettings.LastViewSelected));

	m_GlobalViewSettings.CalendarShowDays = GetInt(_T("CalendarShowDays"), TRUE);

	m_GlobalViewSettings.GlobeLatitude = GetInt(_T("GlobeLatitude"), 1);
	m_GlobalViewSettings.GlobeLongitude = GetInt(_T("GlobeLongitude"), 1);
	m_GlobalViewSettings.GlobeZoom = GetInt(_T("GlobeZoom"), 600);
	m_GlobalViewSettings.GlobeShowSpots = GetInt(_T("GlobeShowSpots"), TRUE);
	m_GlobalViewSettings.GlobeShowAirportNames = GetInt(_T("GlobeShowAirportNames"), TRUE);
	m_GlobalViewSettings.GlobeShowGPS = GetInt(_T("GlobeShowGPS"), FALSE);
	m_GlobalViewSettings.GlobeShowDescription = GetInt(_T("GlobeShowDescription"), TRUE);

	m_GlobalViewSettings.IconsShowCapacity = GetInt(_T("IconsShowCapacity"), TRUE);

	m_GlobalViewSettings.TagcloudCanonical = GetInt(_T("TagcloudSortCanonical"), TRUE);
	m_GlobalViewSettings.TagcloudShowRare = GetInt(_T("TagcloudShowRare"), TRUE);
	m_GlobalViewSettings.TagcloudUseSize = GetInt(_T("TagcloudUseSize"), TRUE);
	m_GlobalViewSettings.TagcloudUseColors = GetInt(_T("TagcloudUseColors"), TRUE);
	m_GlobalViewSettings.TagcloudUseOpacity = GetInt(_T("TagcloudUseOpacity"), FALSE);

	return Reset;
}

void CLiquidFoldersApp::SaveGlobalViewSettings()
{
#ifndef _DEBUG
	WriteBinary(_T("LastViewSelected"), (LPBYTE)m_GlobalViewSettings.LastViewSelected, sizeof(m_GlobalViewSettings.LastViewSelected));

	WriteInt(_T("CalendarShowDays"), m_GlobalViewSettings.CalendarShowDays);

	WriteInt(_T("GlobeLatitude"), m_GlobalViewSettings.GlobeLatitude);
	WriteInt(_T("GlobeLongitude"), m_GlobalViewSettings.GlobeLongitude);
	WriteInt(_T("GlobeZoom"), m_GlobalViewSettings.GlobeZoom);
	WriteInt(_T("GlobeShowSpots"), m_GlobalViewSettings.GlobeShowSpots);
	WriteInt(_T("GlobeShowAirportNames"), m_GlobalViewSettings.GlobeShowAirportNames);
	WriteInt(_T("GlobeShowGPS"), m_GlobalViewSettings.GlobeShowGPS);
	WriteInt(_T("GlobeShowDescription"), m_GlobalViewSettings.GlobeShowDescription);

	WriteInt(_T("IconsShowCapacity"), m_GlobalViewSettings.IconsShowCapacity);

	WriteInt(_T("TagcloudSortCanonical"), m_GlobalViewSettings.TagcloudCanonical);
	WriteInt(_T("TagcloudShowRare"), m_GlobalViewSettings.TagcloudShowRare);
	WriteInt(_T("TagcloudUseSize"), m_GlobalViewSettings.TagcloudUseSize);
	WriteInt(_T("TagcloudUseColors"), m_GlobalViewSettings.TagcloudUseColors);
	WriteInt(_T("TagcloudUseOpacity"), m_GlobalViewSettings.TagcloudUseOpacity);

	WriteInt(_T("ViewSettingsVersion"), VIEWSETTINGSVERSION);
#endif
}
