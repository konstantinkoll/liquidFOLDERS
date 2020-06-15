
// liquidFOLDERS.cpp: Definiert das Klassenverhalten für die Anwendung.
//

#include "stdafx.h"
#include "AboutDlg.h"
#include "CFileDropWnd.h"
#include "liquidFOLDERS.h"


const GUID theAppID = { 0x5EB05AE5, 0xC6FE, 0x4E53, { 0xA0, 0x34, 0x36, 0x23, 0x92, 0x1D, 0x18, 0xED } };

BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam)
{
	if (GetWindow(hWnd, GW_OWNER))
		return TRUE;

	DWORD_PTR Result;
	if (SendMessageTimeout(hWnd, theApp.m_WakeupMsg, NULL, NULL, SMTO_NORMAL, 500, &Result))
		if (Result==24878)
		{
			CDSWAKEUP CDSW;
			ZeroMemory(&CDSW, sizeof(CDSW));

			CDSW.AppID = theAppID;

			if (lParam)
				wcscpy_s(CDSW.Command, MAX_PATH, (LPCWSTR)lParam);

			COPYDATASTRUCT CDS;
			CDS.cbData = sizeof(CDSW);
			CDS.lpData = &CDSW;

			if (SendMessage(hWnd, WM_COPYDATA, NULL, (LPARAM)&CDS))
				return FALSE;
		}

	return TRUE;
}


// CLiquidFoldersApp object

CLiquidFoldersApp theApp;

CLiquidFoldersApp::CLiquidFoldersApp()
	: LFApplication(theAppID)
{
	p_ClipboardWnd = NULL;
	m_AppInitialized = FALSE;
}


// CLiquidFoldersApp initialization

BOOL CLiquidFoldersApp::InitInstance()
{
	WCHAR CmdLine[256];
	CmdLine[0] = L'\0';

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

	// User Model ID
	if (m_ShellLibLoaded)
		zSetCurrentProcessExplicitAppUserModelID(L"app.liquidFOLDERS.liquidFOLDERS");

	// RestartManager
	if (m_KernelLibLoaded)
		zRegisterApplicationRestart(L"", 8);	// RESTART_NO_REBOOT

	// Path to Google Earth
	DWORD dwSize = sizeof(m_PathGoogleEarth)/sizeof(WCHAR);
	if (FAILED(AssocQueryString(ASSOCF_REMAPRUNDLL | ASSOCF_INIT_IGNOREUNKNOWN, ASSOCSTR_EXECUTABLE, L".kml", NULL, m_PathGoogleEarth, &dwSize)))
		m_PathGoogleEarth[0] = L'\0';

	// Registry
	SetRegistryBase(_T("Settings"));

	const BOOL ResetViewSettings = LoadGlobalViewSettings();

	for (ITEMCONTEXT Context=0; Context<LFContextCount; Context++)
		LoadContextViewSettings(Context, ResetViewSettings);

	m_ModelQuality = (GLModelQuality)GetInt(_T("ModelQuality"), MODELULTRA);
	m_TextureQuality = (GLTextureQuality)GetInt(_T("TextureQuality"), TEXTUREMEDIUM);
	m_TextureCompress = GetInt(_T("TextureCompress"), FALSE);

	m_StartWith = GetInt(_T("StartWith"), 0);

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

BOOL CLiquidFoldersApp::OpenCommandLine(LPWSTR pCmdLine)
{
	// Parse parameter and create window
	if (pCmdLine)
	{
		// Uppcase command line
		WCHAR* pChar = pCmdLine;
		while (*pChar)
			*(pChar++) = (WCHAR)toupper(*pChar);

		// Check for updates
		if (wcscmp(pCmdLine, L"/CHECKUPDATE")==0)
			return FALSE;

		// FileDrop
		if (wcscmp(pCmdLine, L"/FILEDROP")==0)
		{
			UINT Result;
			ABSOLUTESTOREID StoreID;
			if ((Result=LFGetDefaultStore(StoreID))==LFOk)
			{
				OpenFileDrop(StoreID);

				return TRUE;
			}
			else
			{
				LFErrorBox(CWnd::GetForegroundWindow(), Result);

				return FALSE;
			}
		}

		// Store ID or IATA airport code
		if (wcspbrk(pCmdLine, L".:\\/")==NULL)
		{
			// Store ID
			if (wcslen(pCmdLine)==LFKeySize-1)
			{
				ABSOLUTESTOREID StoreID;
				WideCharToMultiByte(CP_ACP, 0, pCmdLine, -1, StoreID, LFKeySize, NULL, NULL);

				(new CMainWnd())->Create(StoreID);

				return TRUE;
			}

			// IATA airport code
			if (wcslen(pCmdLine)==3)
			{
				CHAR IATACode[4];
				WideCharToMultiByte(CP_ACP, 0, pCmdLine, -1, IATACode, 4, NULL, NULL);

				(new CMainWnd())->Create(IATACode);

				return TRUE;
			}
		}
	}

	// Default
	switch (m_StartWith)
	{
	case STARTWITH_ALLFILES:
		(new CMainWnd())->Create((BYTE)LFContextAllFiles);
		break;

	case STARTWITH_FAVORITES:
		(new CMainWnd())->Create(LFContextFavorites);
		break;

	case STARTWITH_TASKS:
		(new CMainWnd())->Create(LFContextTasks);
		break;

	case STARTWITH_NEW:
		(new CMainWnd())->Create(LFContextNew);
		break;

	default:
		(new CMainWnd())->CreateRoot();
	}

	return TRUE;
}

INT CLiquidFoldersApp::ExitInstance()
{
	if (m_AppInitialized)
	{
		for (ITEMCONTEXT Context=0; Context<LFContextCount; Context++)
			SaveContextViewSettings(Context);

		SetRegistryBase(_T("Settings"));

		SaveGlobalViewSettings();

		WriteInt(_T("ModelQuality"), m_ModelQuality);
		WriteInt(_T("TextureQuality"), m_TextureQuality);
		WriteInt(_T("TextureCompress"), m_TextureCompress);

		WriteInt(_T("StartWith"), m_StartWith);

		WriteInt(_T("ShowInspectorPane"), m_ShowInspectorPane);
		WriteInt(_T("InspectorPaneWidth"), m_InspectorPaneWidth);

		WriteInt(_T("FileDropAlwaysOnTop"), m_FileDropAlwaysOnTop);
	}

	return LFApplication::ExitInstance();
}


CMainWnd* CLiquidFoldersApp::GetClipboard()
{
	if (!p_ClipboardWnd)
		(p_ClipboardWnd=new CMainWnd())->CreateClipboard();

	return p_ClipboardWnd;
}

void CLiquidFoldersApp::OpenFileDrop(const ABSOLUTESTOREID& StoreID)
{
	for (POSITION p=m_pMainFrames.GetHeadPosition(); p; )
	{
		CWnd* pFrameWnd = m_pMainFrames.GetNext(p);
		if (pFrameWnd->SendMessage(WM_OPENFILEDROP, (WPARAM)(LPCSTR)StoreID)==24878)
			return;
	}

	(new CFileDropWnd())->Create(StoreID);
}


void CLiquidFoldersApp::SanitizeContextViewSettings(ITEMCONTEXT Context)
{
	ASSERT(Context<LFContextCount);

	// Find view for this attribute that is allowed in the context
	const ATTRIBUTE Attr = m_ContextViewSettings[Context].SortBy;

	ASSERT(IsAttributeSortable(Attr, Context));
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
	ASSERT(View>=-1);
	ASSERT(View<LFViewCount);

	for (POSITION p=m_pMainFrames.GetHeadPosition(); p; )
		m_pMainFrames.GetNext(p)->PostMessage(WM_CONTEXTVIEWCOMMAND, cmdMsg, MAKELPARAM(Context, View));
}

void CLiquidFoldersApp::SetContextSort(ITEMCONTEXT Context, ATTRIBUTE Attr, BOOL SortDescending, BOOL SetLastView)
{
	ASSERT(Context<LFContextCount);
	ASSERT(Attr<LFAttributeCount);
	ASSERT(IsAttributeSortable(Attr, Context));
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

void CLiquidFoldersApp::SetContextView(ITEMCONTEXT Context, UINT View)
{
	ASSERT(Context<LFContextCount);
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

void CLiquidFoldersApp::LoadContextViewSettings(ITEMCONTEXT Context, BOOL Reset)
{
	CString Base;
	Base.Format(_T("Settings\\Context%u"), Context);
	SetRegistryBase(Base);

	const UINT DefaultAttribute = m_Contexts[Context].CtxProperties.DefaultAttribute;
	const BOOL SortDescending = theApp.IsAttributeSortDescending(DefaultAttribute, Context);
	const UINT DefaultView = m_Contexts[Context].CtxProperties.DefaultView;

	// Default columns
	memcpy(&m_ContextViewSettings[Context].ColumnOrder, m_SortedAttributeList, sizeof(LFAttributeList));

	for (UINT a=0; a<LFAttributeCount; a++)
		m_ContextViewSettings[Context].ColumnWidth[a] = (IsAttributeAlwaysVisible(a) || IsAttributeAdvertised(a, Context)) ? m_Attributes[a].TypeProperties.DefaultColumnWidth : 0;

	// Double the width of file name and title
	m_ContextViewSettings[Context].ColumnWidth[LFAttrFileName] <<= 1;
	m_ContextViewSettings[Context].ColumnWidth[LFAttrTitle] <<= 1;

	// Read settings
	if (!Reset)
	{
		m_ContextViewSettings[Context].SortBy = GetInt(_T("SortBy"), DefaultAttribute);
		m_ContextViewSettings[Context].SortDescending = GetInt(_T("SortDescending"), SortDescending);
		m_ContextViewSettings[Context].View = GetInt(_T("View"), DefaultView);

		Reset |= !IsAttributeAvailable(m_ContextViewSettings[Context].SortBy, Context) ||
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
			if (!IsAttributeAvailable(a, Context))
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

void CLiquidFoldersApp::SaveContextViewSettings(ITEMCONTEXT Context)
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
	m_GlobalViewSettings.GlobeShowLocations = GetInt(_T("GlobeShowLocations"), TRUE);
	m_GlobalViewSettings.GlobeShowAirportNames = GetInt(_T("GlobeShowAirportNames"), TRUE);
	m_GlobalViewSettings.GlobeShowCoordinates = GetInt(_T("GlobeShowCoordinates"), FALSE);
	m_GlobalViewSettings.GlobeShowDescriptions = GetInt(_T("GlobeShowDescriptions"), TRUE);

	m_GlobalViewSettings.IconsShowCapacity = GetInt(_T("IconsShowCapacity"), TRUE);

	m_GlobalViewSettings.TagcloudSort = GetInt(_T("TagcloudSort"), 0);
	m_GlobalViewSettings.TagcloudShowRare = GetInt(_T("TagcloudShowRare"), TRUE);
	m_GlobalViewSettings.TagcloudUseSize = GetInt(_T("TagcloudUseSize"), TRUE);
	m_GlobalViewSettings.TagcloudUseColor = GetInt(_T("TagcloudUseColor"), TRUE);
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
	WriteInt(_T("GlobeShowLocations"), m_GlobalViewSettings.GlobeShowLocations);
	WriteInt(_T("GlobeShowAirportNames"), m_GlobalViewSettings.GlobeShowAirportNames);
	WriteInt(_T("GlobeShowCoordinates"), m_GlobalViewSettings.GlobeShowCoordinates);
	WriteInt(_T("GlobeShowDescriptions"), m_GlobalViewSettings.GlobeShowDescriptions);

	WriteInt(_T("IconsShowCapacity"), m_GlobalViewSettings.IconsShowCapacity);

	WriteInt(_T("TagcloudSort"), m_GlobalViewSettings.TagcloudSort);
	WriteInt(_T("TagcloudShowRare"), m_GlobalViewSettings.TagcloudShowRare);
	WriteInt(_T("TagcloudUseSize"), m_GlobalViewSettings.TagcloudUseSize);
	WriteInt(_T("TagcloudUseColor"), m_GlobalViewSettings.TagcloudUseColor);
	WriteInt(_T("TagcloudUseOpacity"), m_GlobalViewSettings.TagcloudUseOpacity);

	WriteInt(_T("ViewSettingsVersion"), VIEWSETTINGSVERSION);
#endif
}


BEGIN_MESSAGE_MAP(CLiquidFoldersApp, LFApplication)
	ON_COMMAND(IDM_BACKSTAGE_ABOUT, OnBackstageAbout)
END_MESSAGE_MAP()

void CLiquidFoldersApp::OnBackstageAbout()
{
	CWaitCursor WaitCursor;

	AboutDlg dlg(m_pActiveWnd);
	if (dlg.DoModal()==IDOK)
		UpdateViewSettings();
}
