
// liquidFOLDERS.h: Hauptheaderdatei f�r die liquidFOLDERS-Anwendung
//

#pragma once
#include "LFCommDlg.h"
#include "CFormatCache.h"
#include "CThumbnailCache.h"
#include "CMainWnd.h"
#include "resource.h"


// CLiquidFoldersApp:
// Siehe StoreManager.cpp f�r die Implementierung dieser Klasse
//

#define ViewParametersVersion     2

class CLiquidFoldersApp : public LFApplication
{
public:
	CLiquidFoldersApp();

	virtual BOOL InitInstance();
	virtual CWnd* OpenCommandLine(WCHAR* CmdLine=NULL);
	virtual INT ExitInstance();

	CMainWnd* GetClipboard();
	CWnd* GetFileDrop(CHAR* StoreID);

	BOOL IsViewAllowed(INT Context, INT View) const;
	void Broadcast(INT Context, INT View, UINT cmdMsg);
	void UpdateSortOptions(INT Context);
	void UpdateViewOptions(INT Context=-1, INT View=-1);
	void Reload(INT Context);

	CMainWnd* p_Clipboard;
	CString m_PathGoogleEarth;
	CFormatCache m_FileFormats;
	CThumbnailCache m_ThumbnailCache;
	CIcons m_SourceIcons;
	LFViewParameters m_Views[LFContextCount];
	UINT m_AllowedViews[LFContextCount];

	UINT m_nTextureSize;
	UINT m_nMaxTextureSize;

	BOOL m_ShowInspectorPane;
	UINT m_InspectorWidth;

	BOOL m_CalendarShowCaptions;
	BOOL m_GlobeLighting;
	BOOL m_GlobeAtmosphere;
	BOOL m_GlobeShowViewport;
	BOOL m_GlobeShowCrosshairs;

	BOOL m_FileDropAlwaysOnTop;

protected:
	BOOL SanitizeSortBy(LFViewParameters* pViewParameters, INT Context) const;
	BOOL SanitizeViewMode(LFViewParameters* pViewParameters, INT Context) const;
	void LoadViewOptions(UINT Context);
	void SaveViewOptions(UINT Context);

	BOOL m_AppInitialized;
};

extern CLiquidFoldersApp theApp;
